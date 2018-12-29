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

#include <system.h>

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
    tree.castling_states[0] = game_hist[game_hist_ply].castling_state;

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
    update_castling_state (&tree, move, 0);

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

int
think (mv, epd)
     chi_move* mv;
     chi_epd_pos* epd;
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* move_ptr;
    int the_score = current_score;
    TREE tree;
    int result = EVENT_CONTINUE;
    long int elapsed;
    int sdepth;
    int num_moves;
    int last_nodes = 0;
    long int last_elapsed = 0;
    int alpha = -INF;
    int beta = +INF;
    int i;
    chi_color_t hash_color;
    int three_fold = 0;
    bitv64 this_signature = game_hist[game_hist_ply].signature;
    bitv64 nps = 0;  /* Shut up compiler.  */

    memset (&tree, 0, sizeof tree);

    start_time = rtime ();

    move_ptr = chi_legal_moves (&current, moves);

    num_moves = move_ptr - moves;

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
    tree.best_move = *mv = moves[0];
    if (num_moves == 1) {
	time_cushion += inc * 100;
	return 0;
    }

    /* Initialize signature.  */
    tree.signatures[0] = game_hist[game_hist_ply].signature;
    tree.castling_states[0] = game_hist[game_hist_ply].castling_state;

    fprintf (stdout, "Initial castling state: 0x%x\n", tree.castling_states[0]);
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

#if ASPIRATION_WINDOW
    fprintf (stdout, "Current score is: %d\n", current_score);
    alpha = current_score - MIN_EVAL_DIFF;
    beta = current_score + MIN_EVAL_DIFF;
#endif

    for (sdepth = 1; sdepth <= max_ply; ++sdepth) {
	int score;

	tree.iteration_sdepth = sdepth;

#if DEBUG_BRAIN
	fprintf (stderr, "\
Searching best of %d moves in position (material %d) with depth %d [%d, %d]\n",
		 num_moves, chi_material (&tree.pos), sdepth, alpha, beta);
#endif
	
	score = search (&tree, PLIES2DEPTH (sdepth), 0, alpha, beta, 0);

	total_nodes += tree.nodes - last_nodes;
	last_nodes = tree.nodes;
	elapsed = rdifftime (rtime (), start_time);
	total_centiseconds += elapsed - last_elapsed;
	last_elapsed = elapsed;

	*mv = tree.best_move;

	if (tree.status & EVENTMASK_ENGINE_STOP)
	    break;
	
	current_score = score;

	if (score <= MATE) {
	    print_pv (&tree, score, 0, 0);

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
	    print_pv (&tree, score, 0, 0);

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

#define ASPIRATION_WINDOW 1

	if (score > MATE && score < -MATE) {
#if ASPIRATION_WINDOW
	    if (score <= alpha) {
		alpha = score - MIN_EVAL_DIFF;
#if DEBUG_BRAIN
		fprintf (stderr, "\
Re-searching (failed low) best of %d moves in position (material %d) with depth %d [%d, %d]\n",
			 num_moves, chi_material (&tree.pos), sdepth, alpha, beta);
#endif
		--sdepth;
		continue;
	    } else if (score >= beta) {
		beta = score + MIN_EVAL_DIFF;
#if DEBUG_BRAIN
		fprintf (stderr, "\
Re-searching (failed high) best of %d moves in position (material %d) with depth %d [%d, %d]\n",
			 num_moves, chi_material (&tree.pos), sdepth, alpha, beta);
#endif
		--sdepth;
		continue;
	    }
	    
	    alpha = score - MIN_EVAL_DIFF;
	    beta = score + MIN_EVAL_DIFF;
	}
#endif

	the_score = score;

	/* if (tree.iteration_sdepth > tree.pv_printed) */
	print_pv (&tree, score, 0, 0);

	/* Do not go any deeper, if we have already used up more than
	   2/3 of our time.  */
	if (!epd && sdepth > 2) {
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
	printf ("  Score: %#.3g\n", chi_value2pawns (the_score));
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
	printf ("  Null moves: %lu (%lu failed high)\n",
		tree.null_moves, tree.null_fh);
	printf ("  Futility-pruned moves: %lu\n",
		tree.fprunes);
	printf ("  Evaluations: %lu\n", tree.evals);
	printf ("  One-Pawn lazy evals: %lu\n", tree.lazy_one_pawn);
	printf ("  Two-pawn lazy evals: %lu\n", tree.lazy_two_pawns);
	printf ("  Total nodes searched: %llu\n", total_nodes);
	if (total_centiseconds)
	    printf ("  Total nodes per second: %llu (peak: %llu)\n", 
		    (100 * total_nodes) / total_centiseconds,
		    nps_peak);
    }

    time_left -= elapsed;

    return result;
}

void
update_castling_state (tree, move, ply)
     TREE* tree;
     chi_move move;
     int ply;
{
    int castling_state = tree->castling_states[ply];
    chi_pos* pos = &tree->pos;

#define wk_castle_move (((~king & 0x7) << 13) | (1 << 6) | 3)
#define wq_castle_move (((~king & 0x7) << 13) | (5 << 6) | 3)
#define bk_castle_move (((~king & 0x7) << 13) | (57 << 6) | 59)
#define bq_castle_move (((~king & 0x7) << 13) | (61 << 6) | 59)

    switch (move) {
	case wk_castle_move:
	case wq_castle_move:
	    castling_state &= 0x70;
	    castling_state |= 0x4;
	    break;
	case bk_castle_move:
	case bq_castle_move:
	    castling_state &= 0x7;
	    castling_state |= 0x40;
	    break;
    }

    if (!chi_wk_castle (pos))
	castling_state &= ~0x1;

    if (!chi_wq_castle (pos))
	castling_state &= ~0x2;
    
    if (!chi_bk_castle (pos))
	castling_state &= ~0x10;

    if (!chi_bq_castle (pos))
	castling_state &= ~0x20;

    tree->castling_states[ply + 1] = castling_state;
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
