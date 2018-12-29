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
    chi_move move;
    chi_pos saved_pos;
    chi_pos* pos = &tree->pos;
    int pv_seen = 0;
    int standing_pat;
    int best_score;
    chi_move best_move = 0;
    int material;
    int num_moves;

    chi_copy_pos (&saved_pos, pos);

    ++tree->nodes;
    ++tree->qnodes;
    --next_time_control;

    tree->pv[ply].length = 0;
    tree->pv[ply].moves[0] = 0;

    tree->cv.moves[ply] = 0;
    tree->cv.length = ply;

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
	    fprintf (stdout, "  Time used up!\n");
	    return beta;
	}
	next_time_control = 0x2000;
    }

    if (ply >= MAX_PLY - 1)
	return beta;

    if (ply > tree->max_ply)
        tree->max_ply = ply;

    // FIXME: Maybe this has the opposite effect?!
    if (tree->in_check[ply])
        return search (tree, ONEPLY, ply, alpha, beta, 1);

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
		      chi_material (pos) : -chi_material (pos));

    num_moves = 0;

    tree->cv.length = ply + 1;

    while (0 != (move = next_move (tree, ply, 0))) {
	int score;
	
	/* Refute moves that will not bring the material balance close
	   to alpha.  */
	int min_material = alpha - material - MAX_POS_SCORE;
	int this_material = 100 * chi_move_material (move);

	if (!(chi_move_victim (move)) && !(chi_move_promote (move)))
	    continue;

#if DEBUG_BRAIN
	fprintf (stderr, "move: %s-%s, alpha: %d, material: %d, min_material: %d, move material: %d\n", 
		 chi_shift2label (chi_move_from (move)),
		 chi_shift2label (chi_move_to (move)),
		 alpha, material, min_material, 100 * chi_move_material (move));
#endif

	if (this_material < min_material) {
	    ++tree->refuted_quiescences;
	    continue;
	}

	if (chi_illegal_move (pos, move, 0))
	    continue;

	tree->signatures[ply + 1] = 
	    chi_zk_update_signature (zk_handle, tree->signatures[ply],
				     move, !chi_on_move (pos));
	update_castling_state (tree, move, ply);

	++num_moves;
	tree->cv.moves[ply] = move;
	tree->in_check[ply + 1] = chi_check_check (pos);

#if DEBUG_BRAIN
        chi_on_move (pos) = !chi_on_move (pos);
        indent_output (tree, ply);
        fprintf (stderr, "st: ");
        my_print_move (move);
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
	chi_copy_pos (pos, &saved_pos);

	if (tree->status & EVENTMASK_ENGINE_STOP)
	    return alpha;

	if (score > alpha) {
	    if (score >= beta) {
		// FIXME: A mate as a return value is not necessarily
		// correct!?

		/* Is this correct? */
		tree->pv[ply].moves[0] = move;
		memcpy (tree->pv[ply].moves + 1, tree->pv[ply + 1].moves,
			tree->pv[ply + 1].length * sizeof move);
		tree->pv[ply].length = tree->pv[ply + 1].length + 1;
		
#if DEBUG_BRAIN
		indent_output (tree, ply);
		fprintf (stderr, "fh: ");
		my_print_move (move);
		fprintf (stderr, " failed high with score %d\n",
			 chi_value2centipawns (score));
#endif	    
		++tree->qfh;
		if (num_moves == 1)
		    ++tree->qffh;
		return score;
	    } else {
		pv_seen = 1;
		alpha = score;
		if (score > best_score) {
		    best_score = score;
		    best_move = move;
		}
		tree->pv[ply].moves[0] = move;
		memcpy (tree->pv[ply].moves + 1, tree->pv[ply + 1].moves,
			tree->pv[ply + 1].length * sizeof move);
		tree->pv[ply].length = tree->pv[ply + 1].length + 1;
		
#if DEBUG_BRAIN
		indent_output (tree, ply);
		fprintf (stderr, "pv: ");
		my_print_move (move);
		fprintf (stderr, " new pv with score: %d\n",
			 chi_value2centipawns (score));
		dump_pv (tree);
#endif
	    }
	} else {
	    if (score > best_score) {
		best_score = score;
		best_move = move;
	    }
#if DEBUG_BRAIN
            indent_output (tree, ply);
            fprintf (stderr, "fl: ");
            my_print_move (move);
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
