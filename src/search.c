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

#include <system.h>

#include <libchi.h>

#include "brain.h"
#include "time_ctrl.h"

int
search (tree, depth, ply, alpha, beta, allow_null)
     TREE* tree;
     int depth;
     int ply;
     int alpha;
     int beta;
     int allow_null;
{ 
    chi_move moves[CHI_MAX_MOVES];
    chi_move* mv;
    chi_pos saved_pos = tree->pos;
    chi_pos* pos = &tree->pos;
    int num_moves = 0;
    int pv_seen = 0;
    int original_alpha = alpha;
    chi_move best_move = 0;

    ++tree->nodes;

    tree->pv[ply].length = 0;

    /* Check for time control and user input.  */
    if (tree->nodes > 0x2000) {
	if (event_pending) {
	    int result = get_event ();
	    if (result & EVENTMASK_ENGINE_STOP)
		tree->status = result;
	    return beta;
	}

	if (rdifftime (rtime (), start_time) >= tree->time_for_move) {
	    tree->status = EVENT_MOVE_NOW;
	    fprintf (stdout, "  Time used up!\n");
	    return beta;
	}
    }

    if (ply >= MAX_PLY - 1)
	return beta;

    ++tree->tt_probes;
    /* Probe the transposition table.  */
    switch (probe_tt (tree, tree->signatures[ply],
                      depth, &alpha, &beta)) {
        case HASH_EXACT:
            ++tree->tt_hits;
	    ++tree->tt_exact_hits;
	    if (ply == 0)
		print_pv (tree, alpha, 0, ply);
	    if (alpha < beta) {
		tree->pv[ply].moves[0] = best_tt_move (tree,
						       tree->signatures[ply]);
		tree->pv[ply].length = 1;
            }
#if DEBUG_BRAIN
	    indent_output (tree, ply);
	    fprintf (stderr, "HX: result hashed");
	    fprintf (stderr, " alpha: %d, beta: %d\n",
		     chi_value2centipawns (alpha),
		     chi_value2centipawns (beta));
#endif
	    return alpha;
        case HASH_ALPHA:
            ++tree->tt_hits;
            ++tree->tt_alpha_hits;
	    if (ply == 0)
		print_pv (tree, alpha, 0, ply);
#if DEBUG_BRAIN
	    indent_output (tree, ply);
	    fprintf (stderr, "HA: result hashed");
	    fprintf (stderr, " alpha: %d, beta: %d\n",
		     chi_value2centipawns (alpha),
		     chi_value2centipawns (beta));
#endif

            return alpha;
        case HASH_BETA:
            ++tree->tt_hits;
            ++tree->tt_beta_hits;
	    if (ply == 0)
		print_pv (tree, beta, 0, ply);
#if DEBUG_BRAIN
	    indent_output (tree, ply);
	    fprintf (stderr, "HB: result hashed");
	    fprintf (stderr, " alpha: %d, beta: %d\n",
		     chi_value2centipawns (alpha),
		     chi_value2centipawns (beta));
#endif
            return beta;
    }

    if (depth < 1) {
	int score;
#if NO_QUIESCENCE
	/* Leaf.  Do a quiescence search.  */
	score = evaluate (tree, ply, alpha, beta);
#else
	score = quiescence (tree, ply, alpha, beta);
#endif

#if DEBUG_BRAIN
        chi_on_move (pos) = !chi_on_move (pos);
	indent_output (tree, ply - 1);
	fprintf (stderr, 
		 "Storing xscore %d with no move at depth %d (HASH_EXACT)\n",
		 -score, depth);
        chi_on_move (pos) = !chi_on_move (pos);
#endif

	if (score >= beta) {
	    tree->tt_collisions += store_tt_entry (tree,
						   tree->signatures[ply], 0, 0,
						   score, HASH_BETA);
	} else if (score > alpha) {
	    tree->tt_collisions += store_tt_entry (tree,
						   tree->signatures[ply], 0, 0,
						   score, HASH_EXACT);
	} else {
	    tree->tt_collisions += store_tt_entry (tree,
						   tree->signatures[ply], 0, 0,
						   score, HASH_ALPHA);
	}

	return score;
    }

    tree->in_check[ply] = chi_check_check (pos);

#if 0
    /* Try a null move if applicable.  */
    if (depth <= NULL_R)
	allow_null = 0;
#endif

    if (allow_null && !tree->in_check[ply]) {
	int null_score;
	int saved_pv_length = tree->pv[ply].length;

	++tree->null_moves;

#if DEBUG_BRAIN
        chi_on_move (pos) = !chi_on_move (pos);
        indent_output (tree, ply);
        fprintf (stderr, "NM: ");
        fprintf (stderr, " alpha: %d, beta: %d\n",
                 chi_value2centipawns (alpha),
                 chi_value2centipawns (beta));
        chi_on_move (pos) = !chi_on_move (pos);
#endif

	chi_ep (pos) = 0;
	chi_on_move (pos) = !chi_on_move (pos);
	tree->signatures[ply + 1] = 
	    chi_zk_change_side (zk_handle, tree->signatures[ply]);
	tree->castling_states[ply + 1] = tree->castling_states[ply];
	null_score = -search (tree, depth - NULL_R, ply + 1,
			      -beta, -beta + 1, 0);

	*pos = saved_pos;
	tree->pv[ply].length = saved_pv_length;

	if (null_score >= beta) {
#if DEBUG_BRAIN
	    chi_on_move (pos) = !chi_on_move (pos);
	    indent_output (tree, ply);
	    fprintf (stderr, "NH: ");
            fprintf (stderr, " NH: null move failed high with score %d\n",
                     chi_value2centipawns (null_score));
	    chi_on_move (pos) = !chi_on_move (pos);
#endif
	    ++tree->null_fh;
	    return null_score;
	}
#if DEBUG_BRAIN
	chi_on_move (pos) = !chi_on_move (pos);
	indent_output (tree, ply);
	fprintf (stderr, "NX: ");
	fprintf (stderr, " NH: null move returned score %d\n",
		 chi_value2centipawns (null_score));
	chi_on_move (pos) = !chi_on_move (pos);
#endif
    } else {
	allow_null = 0;
    }

    tree->move_stack[ply] = moves;
    tree->move_states[ply] = move_state_init;

    while (NULL != (mv = next_move (tree, ply, depth))) {
	int score;

	if (chi_illegal_move (pos, *mv))
	    continue;

	tree->signatures[ply + 1] = 
	    chi_zk_update_signature (zk_handle, tree->signatures[ply],
				     *mv, !chi_on_move (pos));
	update_castling_state (tree, *mv, ply);

	++num_moves;

#if DEBUG_BRAIN
        chi_on_move (pos) = !chi_on_move (pos);
        indent_output (tree, ply);
        fprintf (stderr, "ST: ");
        my_print_move (*mv);
        fprintf (stderr, " alpha: %d, beta: %d\n",
                 chi_value2centipawns (alpha),
                 chi_value2centipawns (beta));
        chi_on_move (pos) = !chi_on_move (pos);
#endif

	if (pv_seen) {
	    score = -search (tree, depth - 1, ply + 1, 
			     -alpha - 1, -alpha, !allow_null);
	    if ((score > alpha) && (score < beta)) {
		score = -search (tree, depth - 1, ply + 1, 
				 -beta, -alpha, !allow_null);
	    }
	} else {
	    score = -search (tree, depth - 1, ply + 1, 
			     -beta, -alpha, !allow_null);
	}
	*pos = saved_pos;

	if (tree->status & EVENTMASK_ENGINE_STOP)
	    return alpha;

	if (score >= beta) {
#if DEBUG_BRAIN
	    chi_on_move (pos) = !chi_on_move (pos);
            indent_output (tree, ply);
            fprintf (stderr, "FH: ");
            my_print_move (*mv);
            fprintf (stderr, " failed high with score %d\n",
                     chi_value2centipawns (score));
	    indent_output (tree, ply);
	    fprintf (stderr, "Storing score %d with move ", score);
	    my_print_move (*mv);
	    fprintf (stderr, " at depth %d (HASH_BETA)\n", depth);
	    chi_on_move (pos) = !chi_on_move (pos);
#endif

	    store_killer (tree, *mv, ply);
	    ++tree->fh;
	    if (num_moves == 1)
		++tree->ffh;
	    else if (*mv == tree->bonny[ply] || *mv == tree->clyde[ply])
		++tree->killers;
	    tree->tt_collisions += store_tt_entry (tree,
						   tree->signatures[ply], 
						   *mv, depth, score, 
						   HASH_BETA);
	    return score;
	} else if (score > alpha) {
	    pv_seen = 1;
	    alpha = score;
	    tree->pv[ply].moves[0] = best_move = *mv;
	    memcpy (tree->pv[ply].moves + 1, tree->pv[ply + 1].moves,
		    tree->pv[ply + 1].length * sizeof *mv);
	    tree->pv[ply].length = tree->pv[ply + 1].length + 1;

	    if (ply == 0)
		print_pv (tree, score, 0, ply);

#if DEBUG_BRAIN
            indent_output (tree, ply);
            fprintf (stderr, "PV: ");
            my_print_move (*mv);
            fprintf (stderr, " new pv with score: %d\n",
                     chi_value2centipawns (score));
#endif
	} else {
#if DEBUG_BRAIN
            indent_output (tree, ply);
            fprintf (stderr, "FL: ");
            my_print_move (*mv);
            fprintf (stderr, " failed low with score: %d\n",
                     chi_value2centipawns (score));
#endif
	    ++tree->fl;
	}

    }

    if (!num_moves) {
	if (tree->in_check[ply])
	    alpha = MATE;
	else
	    alpha = DRAW;
    }

#if DEBUG_BRAIN
    fprintf (stderr, "Storing final score %d with move ", alpha);
    my_print_move (best_move);
    fprintf (stderr, " at depth %d (%s)\n", depth,
	     original_alpha == alpha ? "HASH_ALPHA" : "HASH_EXACT");
#endif

    store_killer (tree, best_move, ply);
    tree->tt_collisions += store_tt_entry (tree, tree->signatures[ply], 
					   best_move, depth, alpha,
					   original_alpha == alpha ? HASH_ALPHA : HASH_EXACT);

    return alpha;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
