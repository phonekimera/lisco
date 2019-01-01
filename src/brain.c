/* brain.c - find the correct move... ;-)
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

#include <stdio.h>
#include <stdlib.h>

#include <error.h>

#include <libchi.h>

#include "brain.h"
#include "tate.h"
#include "time_ctrl.h"
#include "board.h"

bitv64 total_nodes = 0;
bitv64 total_centiseconds = 0;
bitv64 nps_peak = 0;

typedef struct {
    chi_move move;
    TREE* tree;
    int score;
    bitv64 nodes;
} ext_move;

static int compare_ext_moves(const void* a, const void* b);
static int root_search(TREE* tree, int sdepth, ext_move* root_moves,
				       int num_moves, int alpha, int beta);

#if 0
static void
dump_ext_moves (moves, num_moves)
     ext_move* moves;
     int num_moves;
{
    TREE* tree = moves[0].tree;
    int i;
    
    printf ("\n    bonny: %s-%s, clyde: %s-%s\n",
	    chi_shift2label (chi_move_from (tree->bonny[0])),
	    chi_shift2label (chi_move_to (tree->bonny[0])),
	    chi_shift2label (chi_move_from (tree->clyde[0])),
	    chi_shift2label (chi_move_to (tree->clyde[0])));

    for (i = 0; i < num_moves; ++i) {
	chi_move move = moves[i].move;
	int score = moves[i].score;

	printf (" %s-%s (",
		chi_shift2label (chi_move_from (move)),
		chi_shift2label (chi_move_to (move)));
	printf ("score: %d, ", score);
	printf ("history: %d)\n", history_lookup (tree, move));
    }
}
#endif

int 
compare_ext_moves (ext_a, ext_b)
     const void* ext_a;
     const void* ext_b;
{
    const ext_move* a = (const ext_move*) ext_a;
    const ext_move* b = (const ext_move*) ext_b;
    TREE* tree;
    chi_move best_move;
    tree = a->tree;

    if (a->move == b->move)
	return 0;

    if (a->move == tree->pv[0].moves[0])
	return -1;
    if (b->move == tree->pv[0].moves[0])
	return 1;

    if (a->score != b->score)
	return b->score - a->score;

    best_move = best_tt_move (&tree->pos, tree->signatures[0]);
    if (a->move == best_move)
	return -1;
    if (b->move == best_move)
	return 1;

//    if (a->nodes != b->nodes)
//	return b->nodes - a->nodes;

    if (chi_move_material (a->move)) {
	if (!(chi_move_material (b->move)))
	    return -1;
	return b->move - a->move;
    } else if (chi_move_material (b->move)) {
	return 1;
    }

    if (a->move == tree->bonny[0])
	return -1;
    if (b->move == tree->bonny[0])
	return 1;
    if (a->move == tree->clyde[0])
	return -1;
    if (b->move == tree->clyde[0])
	return 1;

    return history_lookup (tree, b->move) - 
	history_lookup (tree, a->move);
}

void
evaluate_move (move)
     chi_move move;
{
    TREE tree;
    int errnum;
    int score;

    memset (&tree, 0, sizeof tree);

    tree.pos = current;

    /* Initialize signature.  */
    tree.signatures[0] = game_hist[game_hist_ply].signature;

    /* Apply the move to the position.  */
    errnum = chi_apply_move (&tree.pos, move);
    if (errnum) {
	fprintf (stdout, "  illegal move: %s\n", chi_strerror (errnum));
	return;
    }

    tree.pv[0].moves[0] = move;
    tree.pv[0].length = 1;
    tree.in_check[0] = chi_check_check (&tree.pos);
    tree.signatures[1] = 
	chi_zk_update_signature (zk_handle, tree.signatures[0],
				 move, chi_on_move (&tree.pos));

    score = -evaluate (&tree, 1, 
		       100 * chi_material (&tree.pos), 
		       100 * chi_material (&tree.pos));

    fprintf (stdout, "  static score: %d\n", score);

    score = evaluate_dev_white (&tree, 1);
    fprintf (stdout, "  development white: %d\n", score);
    score = evaluate_dev_black (&tree, 1);
    fprintf (stdout, "  development black: %d\n", score);
    score = evaluate_mobility (&tree);
    fprintf (stdout, "  mobility: %d\n", score);
}

static int
root_search (tree, depth, moves, num_moves, alpha, beta)
     TREE* tree;
     int depth;
     ext_move* moves;
     int num_moves;
     int alpha;
     int beta;
{
    int i = 0;
    chi_pos tmp_pos;
    chi_pos* pos = &tree->pos;
    int wtm = chi_on_move (pos) == chi_white;
    int* hist_hash = wtm ? tree->black_tree_hist : tree->white_tree_hist;
    int pv_seen = 0;
    int original_alpha = alpha;
    bitv64 last_nodes = tree->nodes - tree->qnodes;
    int best_score = -INF;
    chi_move best_move = 0;

    /* Initialize principal variation.  */
    tree->cv.length = 1;
    tree->pv[0].length = 1;

    chi_copy_pos (&tmp_pos, pos);

    qsort (moves, num_moves, sizeof *moves, compare_ext_moves);
    for (i = 0; i < num_moves; ++i) {
	int score;
	bitv64 signature;
	chi_move move = moves[i].move;

	chi_apply_move (pos, move);

	tree->in_check[1] = chi_check_check (pos);

	signature = tree->signatures[1] = 
	    chi_zk_update_signature (zk_handle, tree->signatures[0],
				     move, !wtm);

	tree->cv.moves[0] = move;

	++hist_hash[signature % HASH_HIST_SIZE];
	if (pv_seen) {
	    if (!xboard)
		print_current_move (tree, &tmp_pos,
				    move, i + 1, num_moves, 
				    -alpha - 1, -alpha);

	    score = -search (tree, depth - ONEPLY, 
			     1, -alpha - 1, -alpha, 1);

	    if ((score > alpha) && (score < beta)) {
		if (tree->status & EVENTMASK_ENGINE_STOP)
		    return alpha;

		if (!xboard) {
		    print_current_move (tree, &tmp_pos,
					move, i + 1, num_moves, 
					-beta, -alpha);
		}
		score = -search (tree, 
				 depth - ONEPLY,
				 1, -beta, -alpha, 1);
	    }
	} else {
	    if (!xboard)
		print_current_move (tree, &tmp_pos,
				    move, i + 1, num_moves, 
				    -beta, -alpha);
	    score = -search (tree, depth - ONEPLY,
			     1, -beta, -alpha, 1);
	}
	--hist_hash[signature % HASH_HIST_SIZE];
	chi_copy_pos (pos, &tmp_pos);
	moves[i].score = score;
	moves[i].nodes = tree->nodes - tree->qnodes - last_nodes;
	last_nodes = tree->nodes - tree->qnodes;

	if (score > best_score) {
	    current_score = best_score = score;
	    best_move = move;

	    tree->pv[0].moves[0] = move;
	    if (tree->pv[1].length) {
		memcpy (tree->pv[0].moves + 1, tree->pv[1].moves,
			tree->pv[1].length * sizeof move);
		tree->pv[0].length = tree->pv[1].length + 1;
	    } else {
		tree->pv[0].length = 1;
	    }
	}

	if (tree->status & EVENTMASK_ENGINE_STOP)
	    return best_score;

	if (score > alpha) {
	    if (score >= beta) {
		store_killer (tree, move, depth, 0);
		++tree->fh;
		if (i == 0)
		    ++tree->ffh;
		else if ((move == tree->bonny[0]) || (move == tree->clyde[0]))
		    ++tree->killers;

		store_tt_entry (pos, tree->signatures[0], move, depth, 
				score, HASH_BETA);

		return score;
	    } else {
		pv_seen = 1;
		alpha = score;
		print_pv (tree, score, 0, 0);
	    }
	}
    }

    store_killer (tree, tree->pv[0].moves[0], depth, 0);

    store_tt_entry (pos, tree->signatures[0], tree->pv[0].moves[0],
		    depth, best_score, best_score != original_alpha ?
		    HASH_EXACT : HASH_ALPHA);

    return best_score;
}

#define ASPIRATION 50

int
think (mv, epd)
     chi_move* mv;
     chi_epd_pos* epd;
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* move_ptr;
    ext_move ext_moves[CHI_MAX_MOVES];

    TREE tree;
    int alpha = -INF;
    int beta = +INF;
    int result = EVENT_CONTINUE;
    int sdepth;
    int last_nodes = 0;
    long int last_elapsed = 0;
    long int elapsed;
    int num_moves;
    int i;
    chi_color_t hash_color;
    int three_fold = 0;
    bitv64 this_signature = game_hist[game_hist_ply].signature;
    bitv64 nps = 0;  /* Shut up compiler.  */
    unsigned int aspiration = ASPIRATION;
    int direction = 0;

    memset (&tree, 0, sizeof tree);

    start_time = rtime ();

    move_ptr = chi_legal_moves (&current, moves);

    num_moves = move_ptr - moves;

#if 0
    current_score = chi_material (&current) * 100;
    if (chi_on_move (&current) != chi_white)
	current_score = - current_score;
#endif
    fprintf (stdout, "Current score is: %d\n", current_score);
    if (num_moves == 0) {
	if (chi_check_check (&current)) {
	    if (chi_on_move (&current) == chi_white) {
		fprintf (stdout, "0-1 {Black mates}\n");
	    } else {
		fprintf (stdout, "1-0 {White mates}\n");
	    }
	} else {
	    fprintf (stdout, "1/2-1/2 {Stalemate}\n");
	}
	return EVENT_GAME_OVER;
    }

    if (current.half_move_clock >= 100) {
	fprintf (stdout, "1/2-1/2 {Fifty-move rule}\n");
	return EVENT_GAME_OVER;
    }

    /* Pre-fill hash history tables.  */
    hash_color = chi_on_move (&current);
    for (i = 0; i <= current.half_move_clock && i <= game_hist_ply; ++i) {
	bitv64 signature = game_hist[game_hist_ply - i].signature;
	int hash_key = signature % HASH_HIST_SIZE;

	if (signature == this_signature)
	    ++three_fold;

	if (three_fold >= 3) {
	    fprintf (stdout, "1/2-1/2 {Three-fold repetition}\n");
	    return EVENT_GAME_OVER;
	}

	if (hash_color == chi_white) {
	    ++tree.white_game_hist[hash_key];
	} else {
	    ++tree.black_game_hist[hash_key];
	}

	hash_color = !hash_color;
    }
    
    /* Better than nothing ...  */
    *mv = moves[0];
    if (num_moves == 1) {
	time_cushion += inc * 100;
	return 0;
    }

    /* Initialize signature.  */
    tree.signatures[0] = game_hist[game_hist_ply].signature;
    tree.w_castled = chi_w_castled (&current);
    tree.b_castled = chi_b_castled (&current);

    tree.epd = epd;

    // FIXME: Check for book move.

    /* How many time will we afford ourselves? */
    if (pondering) {
	tree.time_for_move = 0x7fffffff;
    } else if (epd) {
	tree.time_for_move = epd->fixed_time;
    } else {
	if (fixed_time) {
	    tree.time_for_move = fixed_time;
	} else {
	    if (go_fast) {
		long int alloc = allocate_time ();
		if (alloc > 40)
		    tree.time_for_move = 40;
		else
		    tree.time_for_move = alloc;
	    } else {
		tree.time_for_move = allocate_time ();
	    }
	}
    }

    tree.pos = current;
    tree.in_check[0] = chi_check_check (&tree.pos);
    fprintf (stdout, "Time for move: %ld\n", tree.time_for_move);

#if DEBUG_BRAIN
    max_ply = DEBUG_BRAIN;
    tree.time_for_move = 999999;
#endif

    for (i = 0; i < num_moves; ++i) {
	ext_moves[i].move = moves[i];
	ext_moves[i].score = -INF;
	ext_moves[i].tree = &tree;
	ext_moves[i].nodes = 0ULL;
    }

    for (sdepth = 2; sdepth <= max_ply; ++sdepth) {
	int hash_type;

	if (HASH_UNKNOWN != (hash_type = probe_tt (&tree.pos, 
						   tree.signatures[0], 
						   PLIES2DEPTH (sdepth), 
						   &alpha, &beta))) {
	    chi_move best = best_tt_move (&tree.pos, tree.signatures[0]);

	    if (!best)
		break;
	    
	    switch (hash_type) {
		case HASH_EXACT:
		case HASH_ALPHA:
		    current_score = alpha;
		    break;
		case HASH_BETA:
		    current_score = beta;
		    break;
	    }
	    tree.pv[0].moves[0] = best;
	    tree.pv[0].length = 1;
	} else {
	    break;
	}
    }

    tree.iteration_sdepth = sdepth;
    if (sdepth != 2)
	print_pv (&tree, current_score, 0, 0);

    alpha = current_score - aspiration;
    beta = current_score + aspiration;

    qsort (ext_moves, num_moves, sizeof *ext_moves, compare_ext_moves);
    tree.pv[0].moves[0] = ext_moves[0].move;
    tree.pv[0].length = 1;

    for (; sdepth <= max_ply; ++sdepth) {
	long int elapsed;
	int score;

	tree.iteration_sdepth = sdepth;

#if DEBUG_BRAIN
	fprintf (stderr, "\
Searching best of %d moves in position (material %d) with depth %d [%d, %d]\n",
		 num_moves, chi_material (&tree.pos), sdepth, alpha, beta);
#endif
	
	score = root_search (&tree, PLIES2DEPTH (sdepth), ext_moves, 
			     num_moves, alpha, beta);

	clean_current_move (&tree);

	total_nodes += tree.nodes - last_nodes;
	last_nodes = tree.nodes;
	elapsed = rdifftime (rtime (), start_time);
	total_centiseconds += elapsed - last_elapsed;
	last_elapsed = elapsed;

	if (tree.status & EVENTMASK_ENGINE_STOP)
	    break;
	
	*mv = tree.pv[0].moves[0];

	current_score = score;
	
	if (score <= alpha) {
	    print_fail_low (&tree, score, 0);
	    if (direction > 0) {
		aspiration >>= 1;
		direction = - 1;
	    } else {
		aspiration <<= 1;
	    }
	    
	    if (aspiration < ASPIRATION)
		aspiration = ASPIRATION;
	    else if (aspiration > CHI_VALUE_QUEEN)
		aspiration = CHI_VALUE_QUEEN;
	    
	    alpha = score - aspiration;
	    beta = score + (aspiration >> 1);
	    
#if DEBUG_BRAIN
	    fprintf (stderr, "\
Re-searching (failed low) best of %d moves in position (material %d) with depth %d [%d, %d]\n",
		     num_moves, chi_material (&tree.pos), sdepth, alpha, beta);
#endif
	    --sdepth;
	    continue;
	} else if (score >= beta) {
	    print_fail_high (&tree, score, 0);
	    if (direction < 0) {
		aspiration >>= 1;
		direction = 1;
	    } else {
		aspiration <<= 1;
	    }
	    
	    if (aspiration < ASPIRATION)
		aspiration = ASPIRATION;
	    else if (aspiration > CHI_VALUE_QUEEN)
		aspiration = CHI_VALUE_QUEEN;
	    
	    beta = score + aspiration;
	    alpha = score - (aspiration >> 1);
	    
#if DEBUG_BRAIN
	    fprintf (stderr, "\
Re-searching (failed high) best of %d moves in position (material %d) with depth %d [%d, %d]\n",
		     num_moves, chi_material (&tree.pos), sdepth, alpha, beta);
#endif
	    --sdepth;
	    continue;
	}
	    
	aspiration = ASPIRATION;
	alpha = score - 1;
	beta = score + 1;
	
	if (score <= MATE) {
	    // print_pv (&tree, score, 0, 0);
	    
	    /* We will not resign but let our opponent the fun to
	       beat us.  */
	    if (elapsed & 0x10) {
		fprintf (stdout, "offer draw\n"); /* ;-) */
	    } else if (tree.iteration_sdepth > 1) {
		fprintf (stdout, "tellothers Mated in %d. :-(\n",
			 (MATE - score) >> 1);
	    }
	    break;
	} else if (score >= -MATE) {
	    // print_pv (&tree, score, 0, 0);
	    
	    if (!mate_announce) {
		mate_announce = (MATE + score) >> 1;
		if (mate_announce > 1) {
		    fprintf (stdout, "tellall Mate in %d! :->\n",
			     mate_announce);
		    fprintf (stdout, "tellothers");
		    print_pv (&tree, score, 1, 0);
		}
	    }
	    break;
	}
	
	aspiration = ASPIRATION;

	// if (tree.iteration_sdepth > tree.pv_printed)
	print_pv (&tree, score, 0, 0);

	/* Do not go any deeper, if we have already used up more than
	   2/3 of our time.  */
	if (!tree.epd && sdepth > 2) {
	    elapsed = rdifftime (rtime (), start_time);
	    if (elapsed > tree.time_for_move * 2.1 / 3.0)
		break;
	}
    }

    elapsed = rdifftime (rtime (), start_time);
    if (moves_to_tc && !pondering)
	time_cushion += tree.time_for_move - elapsed + inc;

    if (elapsed) {
	nps = (100 * (bitv64) tree.nodes) / (bitv64) elapsed;
	
	if (tree.nodes > 100000 && nps > nps_peak)
	    nps_peak = nps;
    }

    if (1) {
	printf ("  Nodes searched: %ld (quiescence: %ld [%f%%])\n", 
		tree.nodes, tree.qnodes, 100 * (float) tree.qnodes / ((float) tree.nodes + 1));
	printf ("  Deepest search: %d\n", tree.max_ply);
	printf ("  Refuted quiescences: %ld\n", tree.refuted_quiescences);
	printf ("  Elapsed: %ld.%02lds\n", elapsed / 100, elapsed % 100);
	if (elapsed)
	    printf ("  Nodes per second: %llu\n", nps);
	else
	    printf ("  Nodes per second: INF\n");
	printf ("  Failed high: %ld (move ordering: %f%%), failed low: %ld\n",
		tree.fh, (((float) tree.ffh * 100) / (float) (tree.fh + 1)), 
		tree.fl);
	printf ("  Killer rate: %f%%\n",
		(((float) tree.killers * 100) / (float) (tree.fh + 1)));
	printf ("  Q: failed high: %ld (move ordering: %f%%), failed low: %ld\n",
		tree.qfh, (((float) tree.qffh * 100) / (float) (tree.qfh + 1)), 
		tree.qfl);
	printf ("  TT probes: %lu\n", tree.tt_probes);
	printf ("  TT hits: %lu (exact: %lu, alpha: %lu, beta: %lu, moves: %lu)\n", 
		tree.tt_hits, tree.tt_exact_hits, 
		tree.tt_alpha_hits, tree.tt_beta_hits,
		tree.tt_moves);
	printf ("  QTT probes: %lu\n", tree.qtt_probes);
	printf ("  QTT hits: %lu (exact: %lu, alpha: %lu, beta: %lu)\n", 
		tree.qtt_hits, tree.qtt_exact_hits, 
		tree.qtt_alpha_hits, tree.qtt_beta_hits);
	printf ("  Evaluations: %lu (cache hits: %lu, lazy: %lu)\n",
		tree.evals, tree.ev_hits, tree.lazy_evals);
	printf ("  Null moves: %lu (%lu failed high)\n",
		tree.null_moves, tree.null_fh);
	printf ("  Futility-pruned moves: %lu\n",
		tree.fprunes);
	printf ("  Total nodes searched: %llu\n", total_nodes);
	if (total_centiseconds)
	    printf ("  Total nodes per second: %llu (peak: %llu)\n", 
		    (100 * total_nodes) / total_centiseconds,
		    nps_peak);
    }

    time_left -= elapsed;

    return result;
}

unsigned int history[8192];

// FIXME: Should be inlined.
void
store_killer (tree, move, depth, ply)
     TREE* tree;
     chi_move move;
     int depth;
     int ply;
{
    /* Moves that change the material balance will always be tried first.  */
    if (chi_move_material (move))
	return;

    history[(move & 0xfff) + (chi_on_move (&tree->pos) << 6)] +=
	depth * tree->pos.half_moves;

    if (tree->bonny[ply] != move) {
	tree->clyde[ply] = tree->bonny[ply];
	tree->bonny[ply] = move;
    }
}

void init_killers ()
{
    memset (history, 0, sizeof history);
}

#if DEBUG_BRAIN
void
indent_output (TREE* tree, int ply)
{
    int i;

//    int ply = tree->current_depth - depth;

//    for (i = depth; i < tree->current_depth; ++i)
//        fprintf (stderr, " ");

    for (i = 0; i < ply; ++i)
	fputc (' ', stderr);

    /* Assumed to be called *after* a move has been applied.  */
    if (chi_on_move (&tree->pos) != chi_white)
        fprintf (stderr, " [%s(%d)]: ", 
		 ply < tree->iteration_sdepth ? "BLACK" : "black", ply);
    else
        fprintf (stderr, " [%s(%d)]: ", 
		 ply < tree->iteration_sdepth ? "WHITE" : "white", ply);
}
#endif

void
my_print_move (chi_move mv)
{
    switch (chi_move_attacker (mv)) {
	case knight:
	    fputc ('N', stderr);
	    break;
	case bishop:
	    fputc ('B', stderr);
	    break;
	case rook:
	    fputc ('R', stderr);
	    break;
	case queen:
	    fputc ('Q', stderr);
	    break;
	case king:
	    fputc ('K', stderr);
	    break;
    }

    fprintf (stderr, "%s%c%s", 
	     chi_shift2label (chi_move_from (mv)),
	     chi_move_victim (mv) ? 'x' : '-',
	     chi_shift2label (chi_move_to (mv)));
    switch (chi_move_promote (mv)) {
	case knight:
	    fprintf (stderr, "=N");
	    break;
	case bishop:
	    fprintf (stderr, "=B");
	    break;
	case rook:
	    fprintf (stderr, "=R");
	    break;
	case queen:
	    fprintf (stderr, "=Q");
	    break;
    }

#if 0
    fprintf (stderr, "[%08x:", mv);
    fprintf (stderr, "%d:", chi_move_material (mv));
    fprintf (stderr, "%s:", chi_move_is_ep (mv) ? "ep" : "-");

    switch (chi_move_promote (mv)) {
	case knight:
	    fprintf (stderr, "=N:");
	    break;
	case bishop:
	    fprintf (stderr, "=B:");
	    break;
	case rook:
	    fprintf (stderr, "=R:");
	    break;
	case queen:
	    fprintf (stderr, "=Q:");
	    break;
	case empty:
	    fprintf (stderr, "-:");
	    break;
	default:
	    fprintf (stderr, "=?%d:", chi_move_promote (mv));
	    break;
    }
    
    switch (chi_move_victim (mv)) {
	case pawn:
	    fprintf (stderr, "xP:");
	    break;
	case knight:
	    fprintf (stderr, "xN:");
	    break;
	case bishop:
	    fprintf (stderr, "xB:");
	    break;
	case rook:
	    fprintf (stderr, "xR:");
	    break;
	case queen:
	    fprintf (stderr, "xQ:");
	    break;
	case empty:
	    fprintf (stderr, "-:");
	    break;
	default:
	    fprintf (stderr, "x?%d:", chi_move_victim (mv));
	    break;
    }

    switch (chi_move_attacker (mv)) {
	case pawn:
	    fprintf (stderr, "P");
	    break;
	case knight:
	    fprintf (stderr, "N");
	    break;
	case bishop:
	    fprintf (stderr, "B");
	    break;
	case rook:
	    fprintf (stderr, "R");
	    break;
	case queen:
	    fprintf (stderr, "Q");
	    break;
	case king:
	    fprintf (stderr, "K");
	    break;
    }

    fprintf (stderr, "]");
#endif
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
