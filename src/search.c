/* search.c - negamax search.
 * Copyright (C) 2002 Guido Flohr (guido@imperia.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include <libchi.h>

#include "brain.h"
#include "time_ctrl.h"

int next_time_control = 0x2000;

#define M1 ((bitv64) 0x5555555555555555)
#define M2 ((bitv64) 0x3333333333333333)

static unsigned int
popcount (bitv64 b)
{
	unsigned int n;

	bitv64 a = b - ((b >> 1) & M1);
	bitv64 c = (a & M2) + ((a >> 2) & M2);
	
	n = ((unsigned int) c) + ((unsigned int) (c >> 32));
	n = (n & 0x0f0f0f0f) + ((n >> 4) & 0x0f0f0f0f);
	n = (n & 0xffff) + (n >> 16);
	n = (n & 0xff) + (n >> 8);
	
	return n;	
}

int
search (TREE *tree, int depth, int ply, int alpha, int beta, int allow_null)
{ 
	chi_move moves[CHI_MAX_MOVES];
	chi_move move;
	chi_pos saved_pos;
	chi_pos* pos = &tree->pos;
	int num_moves = 0;
	int pv_seen = 0;
	int original_alpha = alpha;
	int best_score = -INF;
	chi_move best_move = 0;
	int selective = 1;
	int* tree_hist_hash;
	int* game_hist_hash;
	int* sub_tree_hist_hash;
	bitv64 this_signature = tree->signatures[ply];
	int hist_hash_key = this_signature % HASH_HIST_SIZE;
	int wtm;
	int sdepth = DEPTH2PLIES (depth);
	int primary_extensions = 0;
	int num_pieces = -1;

	chi_copy_pos (&saved_pos, pos);

	++tree->nodes;
	--next_time_control;

	tree->cv.moves[ply] = 0;
	tree->cv.length = ply;

	tree->pv[ply].moves[0] = 0;
	tree->pv[ply].length = 0;

	/* Check for time control and user input.  */
	if (next_time_control < 0) {
		if (event_pending) {
			int result = get_event ();
			if (result & EVENTMASK_ENGINE_STOP) {
			tree->status = result;
			}

			return beta;
		}

		if (rdifftime (rtime (), start_time) >= tree->time_for_move) {
			tree->status = EVENT_MOVE_NOW;
			return beta;
		}
		next_time_control = 0x2000;
	}

	if (ply >= MAX_PLY - 1)
		return beta;

	wtm = chi_on_move (pos) == chi_white;

	/* Check for repetitions.  */
	if (wtm) {
		game_hist_hash = tree->white_game_hist;
		tree_hist_hash = tree->white_tree_hist;
		sub_tree_hist_hash = tree->black_tree_hist;
	} else {
		game_hist_hash = tree->black_game_hist;
		tree_hist_hash = tree->black_tree_hist;
		sub_tree_hist_hash = tree->white_tree_hist;
	}

	if (ply > 1 && pos->half_move_clock >= 4 && 
	    tree_hist_hash[hist_hash_key]) {
		/* Maybe already visited.  */
		int i;
		
		for (i = 2; i <= pos->half_move_clock && i <= ply; i += 2) {
			if (tree->signatures[ply - i] == this_signature) {
	#if DEBUG_BRAIN
			indent_output (tree, ply);
			fprintf (stderr, "Draw: %d (in tree), alpha: %d, beta: %d\n",
				DRAW,
				chi_value2centipawns (alpha),
				chi_value2centipawns (beta));
	#endif
				return DRAW;
			}
		}	
	}

	/* Maybe a draw by 3-fold repetition.  */
	if (ply > 1 && pos->half_move_clock >= 8 && 
	    game_hist_hash[hist_hash_key] > 1) {
		int seen = 0;
		int i;
	
#if DEBUG_BRAIN
		fprintf (stderr, "searching for 3-fold repetition at ply %d (%016llx).\n", ply, this_signature);
#endif
		for (i = ply % 2; ((i <= game_hist_ply)
		     && (pos->half_move_clock - i - ply >= 0)); i += 2) {

#if DEBUG_BRAIN
			fprintf(stderr, "checking with i = %d (%016llx<%016llx>%016llx\n", 
			        i, 
			        game_hist[game_hist_ply - i - 1].signature,
			        game_hist[game_hist_ply - i].signature,
			        game_hist[game_hist_ply - i + 1].signature
			        );
#endif

			if (game_hist[game_hist_ply - i].signature == this_signature) {
				++seen;
#if DEBUG_BRAIN
				fprintf (stderr, "Hit #%d\n", seen);
#endif
				if (seen >= 2) {
#if DEBUG_BRAIN
					indent_output (tree, ply);
					fprintf(stderr, "Draw: %d (3-fold), alpha: %d, beta: %d\n",
				 	        DRAW,
					        chi_value2centipawns (alpha),
					        chi_value2centipawns (beta));
#endif

					return DRAW;
				}
				i += 2;
			}
		}
	}

	if (tree->in_check[ply]) primary_extensions += CHECK_EXT;

	/* Probe the transposition table.  */
	++tree->tt_probes;
	switch (probe_tt (pos, tree->signatures[ply], depth,
			  &alpha, &beta)) {	
	case HASH_EXACT:
		++tree->tt_exact_hits;
		++tree->tt_hits;		
		return alpha;
		
	case HASH_BETA:
		++tree->tt_beta_hits;
		++tree->tt_hits;
		return beta;
		
	case HASH_ALPHA:
		++tree->tt_alpha_hits;
		++tree->tt_hits;
		return alpha;
	}

	if (sdepth > 3 || tree->in_check[ply])
		selective = 0;
	else
		selective = 1;

allow_null = 0;
	/* Try a null move if applicable.  */
	while (allow_null) {
		if (tree->in_check[ply]) {
			allow_null = 0;
			break;
		}

		if (sdepth < PLIES2DEPTH (7))
			break;

		if (wtm)
			num_pieces = popcount (pos->w_pieces);
		else
			num_pieces = popcount (pos->b_pieces);

		if (num_pieces <= 6) {
			allow_null = 0;
			break;
		}

		break;
	}

	if (allow_null) {
		int null_score;
		int saved_pv_length = tree->pv[ply].length;
		int null_dist = PLIES2DEPTH (3);

		++tree->null_moves;

		if (depth > PLIES2DEPTH (6)) {
			if (num_pieces < 0) {
				if (wtm)
					num_pieces = popcount (pos->w_pieces);
				else
					num_pieces = popcount (pos->b_pieces);
			}

			if (num_pieces > 8)
				null_dist = PLIES2DEPTH (4);
		}

#if DEBUG_BRAIN
		indent_output(tree, ply);
		fprintf(stderr, "NM: alpha: %d, beta: %d\n",
		        chi_value2centipawns (alpha),
				chi_value2centipawns (beta));
#endif

		chi_ep (pos) = 0;
		chi_on_move (pos) = !chi_on_move (pos);
		++pos->half_moves;
		tree->signatures[ply + 1] = 
			chi_zk_change_side (zk_handle, tree->signatures[ply]);
		tree->in_check[ply + 1] = 0;

		if (depth - null_dist >= ONEPLY)
			null_score = -search(tree, depth - null_dist, ply + 1,
			                    -beta, -beta + 1, 0);
		else
			null_score = -quiescence (tree, ply + 1, -beta, -beta + 1);

		chi_copy_pos (pos, &saved_pos);

		tree->pv[ply].length = saved_pv_length;

		if (null_score >= beta) {
#if DEBUG_BRAIN
			indent_output(tree, ply);
				fprintf(stderr, "NH: null move failed high with score %d\n",
						chi_value2centipawns (null_score));
#endif

			store_tt_entry (pos, tree->signatures[ply], 0, depth, 
			                null_score, HASH_BETA);

			++tree->null_fh;
			return null_score;
		} else if (null_score <= MATE) {
			/* This is a threat.  */
			primary_extensions += MATE_EXT;
		}

#if DEBUG_BRAIN
		indent_output(tree, ply);
		fprintf(stderr, "NM: null move returned score %d\n",
		        chi_value2centipawns (null_score));
#endif
	}

	tree->move_stack[ply] = moves;
	tree->move_states[ply] = move_state_init;

	tree->cv.moves[ply] = 0;
	tree->cv.length = ply + 1;

	while (0 != (move = next_move (tree, ply, depth))) {
		int score;
		bitv64 signature;

		if (num_moves && selective) {
			int pos_material = wtm ? 
			chi_material (pos) : -chi_material (pos);
			int move_material = chi_move_material (move);
			int pot_material = 100 * (pos_material + move_material);
			int fscore = INF;

			if (depth <= FRONTIER_DEPTH) {
				fscore = pot_material + FUTILITY_MARGIN;
			} else if (depth <= PRE_FRONTIER_DEPTH) {
				fscore = pot_material + EXT_FUTILITY_MARGIN;
			} else {
				fscore = pot_material + RAZOR_MARGIN;
			}
			
			if (fscore <= alpha) {
				chi_copy_pos (pos, &saved_pos);
				++tree->fprunes;
				continue;
			}
		}

		if (chi_illegal_move (pos, move,
		                      (tree->move_states[ply] <= 
		                      move_state_generate_non_captures)))
			continue;

		++num_moves;

		tree->in_check[ply + 1] = chi_check_check (pos);

		signature = tree->signatures[ply + 1] = 
			chi_zk_update_signature (zk_handle, tree->signatures[ply],
						 move, !wtm);

		tree->cv.moves[ply] = move;

#if DEBUG_BRAIN
		chi_on_move (pos) = !chi_on_move (pos);
		indent_output (tree, ply);
		fprintf (stderr, "ST: ");
		my_print_move (move);
		fprintf (stderr, " alpha: %d, beta: %d\n",
				 chi_value2centipawns (alpha),
				 chi_value2centipawns (beta));
		chi_on_move (pos) = !chi_on_move (pos);
#endif

		++sub_tree_hist_hash[signature % HASH_HIST_SIZE];
		if (pv_seen) {
			if (depth >= TWOPLY)
				score = -search (tree, depth - ONEPLY, ply + 1, 
				                 -alpha - 1, -alpha, !allow_null);
			else
				score = -quiescence (tree, ply + 1, -alpha - 1, -alpha);

			if ((score > alpha) && (score < beta)) {
#if DEBUG_BRAIN
				chi_on_move (pos) = !chi_on_move (pos);
				indent_output (tree, ply);
				fprintf (stderr, "ST: score %d out of bounds: ", score);
				my_print_move (move);
				fprintf (stderr, " alpha: %d, beta: %d\n",
				chi_value2centipawns (alpha),
				chi_value2centipawns (beta));
				chi_on_move (pos) = !chi_on_move (pos);
#endif

			if (depth >= TWOPLY)
				score = -search (tree, depth - ONEPLY, ply + 1, 
				                 -beta, -alpha, !allow_null);
			else
				score = -quiescence (tree, ply + 1, -beta, -alpha);
			}
		} else {
			if (depth >= TWOPLY)
				score = -search (tree, depth - ONEPLY, ply + 1, 
				                 -beta, -alpha, !allow_null);
			else
				score = -quiescence (tree, ply + 1, -beta, -alpha);
		}
		chi_copy_pos (pos, &saved_pos);
		--sub_tree_hist_hash[signature % HASH_HIST_SIZE];

		if (tree->status & EVENTMASK_ENGINE_STOP)
			return alpha;

		if (score > best_score) {
			best_score = score;
			best_move = move;
		}

		if (tree->status & EVENTMASK_ENGINE_STOP)
			return alpha;

		if (score > alpha) {
			if (score >= beta) {
#if DEBUG_BRAIN
				indent_output (tree, ply);
				fprintf (stderr, "FH: ");
				my_print_move (move);
				fprintf (stderr, " failed high with score %d\n",
				chi_value2centipawns (score));
#endif

				store_killer (tree, move, depth, ply);
				++tree->fh;

				if (num_moves == 1)
					++tree->ffh;
				else if ((move == tree->bonny[ply])
				         || (move == tree->clyde[ply]))
					++tree->killers;

				store_tt_entry (pos, tree->signatures[ply], move, depth, 
				                score, HASH_BETA);

				return score;
			} else {
				pv_seen = 1;
				alpha = score;
		
#if DEBUG_BRAIN
				indent_output (tree, ply);
				fprintf (stderr, "PV: ");
				my_print_move (move);
				fprintf (stderr, " new pv with score: %d\n",
				         chi_value2centipawns (score));
#endif

				tree->pv[ply].moves[0] = move;
				if (tree->pv[ply + 1].length) {
					memcpy (tree->pv[ply].moves + 1, tree->pv[ply + 1].moves,
					tree->pv[ply + 1].length * sizeof move);
					tree->pv[ply].length = tree->pv[ply + 1].length + 1;
				} else {
					tree->pv[ply].length = 1;
				}

#if DEBUG_BRAIN
				dump_pv (tree);
#endif
			}
		} else {
#if DEBUG_BRAIN
			indent_output (tree, ply);
			fprintf (stderr, "FL: ");
			my_print_move (move);
			fprintf (stderr, " failed low with score: %d\n",
					 chi_value2centipawns (score));
#endif
			++tree->fl;
		}
	}

	tree->cv.length = ply;

	if (!num_moves) {
		if (tree->in_check[ply])
			best_score = MATE - ply;
		else
			best_score = DRAW;

		tree->pv[ply].moves[0] = 0;
		tree->pv[ply].length = 0;
	}

	store_killer (tree, tree->pv[ply].moves[0], depth, ply);

	store_tt_entry(pos, tree->signatures[ply], best_move, 
			       depth, best_score, best_score != original_alpha ?
		           HASH_EXACT : HASH_ALPHA);

	return best_score;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
