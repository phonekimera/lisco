/* This file is part of the chess engine tate.
 *
 * Copyright (C) 2002-2019 cantanea EOOD.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#include "brain.h"

/* Pick the next move from the stack and apply it to the position.  */
chi_move
next_move (tree, ply, depth)
     TREE* tree;
     int ply;
     int depth;
{
    chi_move* mv_end;

    switch (tree->move_states[ply]) {
	case move_state_init:
	    tree->move_ptr[ply] = tree->move_stack[ply];
	    tree->move_states[ply] = move_state_pv;
	    tree->pv_move[ply] = 0;

	    /* Try the pv move first if searching principal variation.  */
	    if (!tree->pv_move[ply] && tree->pv[ply].length)
		tree->pv_move[ply] = tree->pv[ply].moves[0];
	    
	    if (!tree->pv_move[ply])
		tree->pv_move[ply] = best_tt_move (&tree->pos, 
						   tree->signatures[ply]);
	    
	    if (tree->pv_move[ply]) {
		++tree->tt_moves;
		*(tree->move_ptr[ply]++) = tree->pv_move[ply];
		return tree->pv_move[ply];
	    }

	    /* FALLTHRU */

	case move_state_pv:
	    mv_end = chi_generate_captures (&tree->pos, tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv_end - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_captures;

	    if (tree->moves_left[ply]) {
		chi_move* mark;

		for (mark = tree->move_ptr[ply]; mark < mv_end; ++mark) {
		    /* Mark new moves, except for those already played.  */
		    chi_move the_move = *mark;

		    *mark |= 0x80000000;

		    if (tree->pv_move[ply] == the_move)
			*mark &= 0x7fffffff;
		}
	    } else {
		tree->move_states[ply] = move_state_bonny; 
	    }

	    /* FALLTHRU.  */
	    
	case move_state_captures:
	    if (tree->moves_left[ply]) {
		int i;
		chi_move best_move = tree->move_ptr[ply][0];
		int index = 0;

		for (i = 1; i < tree->moves_left[ply]; ++i) {
		    if (tree->move_ptr[ply][i] > best_move) {
			index = i;
			best_move = tree->move_ptr[ply][i];
		    }
		}

		if (tree->move_ptr[ply][index] & 0x80000000) {
		    tree->move_ptr[ply][index] &= 0x7fffffff;
		    return tree->move_ptr[ply][index];
		}
	    }

	    /* No more captures.  */

	    if (DEPTH2PLIES (depth) < 1)
		break;

            /* FALLTHRU.  */

	case move_state_bonny:
	    tree->move_states[ply] = move_state_clyde;
	    if (tree->bonny[ply] && tree->bonny[ply] != tree->pv_move[ply])    
		return tree->bonny[ply];

            /* FALLTHRU.  */

	case move_state_clyde:
	    tree->move_states[ply] = move_state_generate_non_captures;
	    
	    if (tree->clyde[ply] &&
		tree->clyde[ply] != tree->pv_move[ply] &&
		tree->clyde[ply] != tree->bonny[ply])	    
		return tree->clyde[ply];
	    
	    /* FALLTHRU.  */

	case move_state_generate_non_captures:
	    mv_end = chi_generate_non_captures (&tree->pos, 
						tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv_end - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_non_captures;

	    if (tree->moves_left[ply]) {
		chi_move* mark;

		for (mark = tree->move_ptr[ply]; mark < mv_end; ++mark) {
		    /* Mark moves as new, except for those already played.  */
		    chi_move the_move = *mark;

		    *mark |= 0x80000000;

		    if (tree->pv_move[ply] == the_move ||
			tree->bonny[ply] == the_move ||
			tree->clyde[ply] == the_move)
			*mark &= 0x7fffffff;
		}
	    }

	    /* FALLTHRU.  */

	case move_state_non_captures:
	    if (tree->moves_left[ply]) {
		int i;
		unsigned int best_history = 0;
		int index = 0;

		for (i = 0; i < tree->moves_left[ply]; ++i) {
		    unsigned int this_history;
		    chi_move this_move = tree->move_ptr[ply][i];
		   
		    if (!(0x80000000 & this_move))
			continue;

		    this_history = 
			history_lookup (tree, tree->move_ptr[ply][i]);
		    if (this_history >= best_history) {
			index = i;
			best_history = this_history;
		    }
		}

		if (tree->move_ptr[ply][index] & 0x80000000) {
		    tree->move_ptr[ply][index] &= 0x7fffffff;
		    return tree->move_ptr[ply][index];
		}
	    }

            /* FALLTHRU.  */
    }

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
