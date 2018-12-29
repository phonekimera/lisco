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
quiescence (tree, ply, alpha, beta)
     TREE* tree;
     int ply;
     int alpha;
     int beta;
{ 
    chi_move moves[CHI_MAX_MOVES];
    chi_move* mv;
    chi_pos saved_pos = tree->pos;
    chi_pos* pos = &tree->pos;
    int pv_seen = 0;
    int standing_pat;
    int best_score;
    chi_move best_move = 0;
    int material;
    int num_moves;
    int original_alpha = alpha;

    ++tree->nodes;
    ++tree->qnodes;

    tree->pv[ply].length = 0;
    tree->pv[ply].moves[0] = 0;

    tree->cv.moves[ply] = 0;
    tree->cv.length = ply;

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

    if (ply > tree->max_ply)
        tree->max_ply = ply;

    tree->in_check[ply] = chi_check_check (pos);
#if 0
    // FIXME: Maybe this has the opposite effect?!
    if (tree->in_check[ply])
        return search (tree, 1, ply, alpha, beta, 1);
#endif

    best_score = standing_pat = evaluate (tree, ply, alpha, beta);
#if DEBUG_BRAIN
    indent_output (tree, ply);
    fprintf (stderr, "sp: standing pat score: %d (alpha: %d, beta: %d)\n",
	     best_score,
             chi_value2centipawns (alpha),
             chi_value2centipawns (beta));
#endif

    if (standing_pat >= beta) {
#if DEBUG_BRAIN
        indent_output (tree, ply);
        fprintf (stderr, "fh: standing pat failed high with score %d\n",
                 chi_value2centipawns (standing_pat));
#endif
        return standing_pat;
    } else if (standing_pat > alpha) {
        alpha = standing_pat;
#if DEBUG_BRAIN
        indent_output (tree, ply);
        fprintf (stderr, "pv: standing pat score: %d\n",
                 chi_value2centipawns (standing_pat));
#endif
    } else {
#if DEBUG_BRAIN
        indent_output (tree, ply);
        fprintf (stderr, "fl: standing pat failed low with score: %d\n",
                 chi_value2centipawns (standing_pat));
#endif
    }

    tree->move_stack[ply] = moves;
    tree->move_states[ply] = move_state_init;

    material = 100 * (chi_on_move (pos) == chi_white ? 
		      pos->material : -pos->material);

    num_moves = 0;

    tree->cv.length = ply + 1;

    while (NULL != (mv = next_move (tree, ply, 0))) {
	int score;
	
	/* Refute moves that will not bring the material balance close
	   to alpha.  */
	int min_material = alpha - material - 100;
	int this_material = 100 * chi_move_material (*mv);

	if (!(chi_move_victim (*mv)))
	    continue;

#if DEBUG_BRAIN
	fprintf (stderr, "move: %s-%s, alpha: %d, material: %d, min_material: %d, move material: %d\n", 
		 chi_shift2label (chi_move_from (*mv)),
		 chi_shift2label (chi_move_to (*mv)),
		 alpha, material, min_material, 100 * chi_move_material (*mv));
#endif

	if (this_material < min_material) {
	    ++tree->refuted_quiescences;
	    continue;
	}

	if (chi_illegal_move (pos, *mv, 0))
	    continue;

	tree->signatures[ply + 1] = 
	    chi_zk_update_signature (zk_handle, tree->signatures[ply],
				     *mv, !chi_on_move (pos));
	update_castling_state (tree, *mv, ply);

	++num_moves;
	tree->cv.moves[ply] = *mv;

#if DEBUG_BRAIN
        chi_on_move (pos) = !chi_on_move (pos);
        indent_output (tree, ply);
        fprintf (stderr, "st: ");
        my_print_move (*mv);
        fprintf (stderr, " alpha: %d, beta: %d\n",
                 chi_value2centipawns (alpha),
                 chi_value2centipawns (beta));
        chi_on_move (pos) = !chi_on_move (pos);
#endif

	if (pv_seen) {
	    score = -quiescence (tree, ply + 1, -alpha - 1, -alpha);
	    if ((score > alpha) && (score < beta)) {
		score = -quiescence (tree, ply + 1, -beta, -alpha);
	    }
	} else {
	    score = -quiescence (tree, ply + 1, -beta, -alpha);
	}
	*pos = saved_pos;

	if (tree->status & EVENTMASK_ENGINE_STOP)
	    return alpha;

	if (score >= beta) {
#if DEBUG_BRAIN
            indent_output (tree, ply);
            fprintf (stderr, "fh: ");
            my_print_move (*mv);
            fprintf (stderr, " failed high with score %d\n",
                     chi_value2centipawns (score));
#endif	    
	    ++tree->qfh;
	    if (num_moves == 1)
		++tree->qffh;
	    return score;
	} else if (score > alpha) {
	    pv_seen = 1;
	    alpha = score;
	    if (score > best_score) {
		best_score = score;
		best_move = *mv;
	    }
	    tree->pv[ply].moves[0] = *mv;
	    memcpy (tree->pv[ply].moves + 1, tree->pv[ply + 1].moves,
		    tree->pv[ply + 1].length * sizeof *mv);
	    tree->pv[ply].length = tree->pv[ply + 1].length + 1;

#if DEBUG_BRAIN
            indent_output (tree, ply);
            fprintf (stderr, "pv: ");
            my_print_move (*mv);
            fprintf (stderr, " new pv with score: %d\n",
                     chi_value2centipawns (score));
	    dump_pv (tree);
#endif
	} else {
	    if (score > best_score) {
		best_score = score;
		best_move = *mv;
	    }
#if DEBUG_BRAIN
            indent_output (tree, ply);
            fprintf (stderr, "fl: ");
            my_print_move (*mv);
            fprintf (stderr, " failed low with score: %d\n",
                     chi_value2centipawns (score));
#endif
	    ++tree->qfl;
	}

    }

#if DEBUG_BRAIN
    indent_output (tree, ply);
    fprintf (stderr, "returning best_score: %d (alpha: %d)\n", 
	     best_score, alpha);
#endif

    tree->cv.length = ply;

    if (best_move && !tree->pv[ply].length) {
	tree->pv[ply].length = 1;
	tree->pv[ply].moves[0] = best_move;
#if DEBUG_BRAIN
	fprintf (stderr, "best of fl moves ");
	my_print_move (best_move);
	fprintf (stderr, " becomes new pv\n");
	dump_pv (tree);
#endif
    }

    return best_score;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
