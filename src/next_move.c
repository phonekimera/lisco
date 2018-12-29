/* next_move.c - fetch next move from stack
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

/* Pick the next move from the stack and apply it to the position.  */
chi_move*
next_move (tree, ply, depth)
     TREE* tree;
     int ply;
     int depth;
{
    chi_move* mv;

    switch (tree->move_states[ply]) {
	case move_state_init:
	    tree->move_ptr[ply] = tree->move_stack[ply];
	    tree->move_states[ply] = move_state_pv;
	    tree->cached_moves[ply] = 0;

	    /* Try the pv move first if searching principal variation.  */
	    if (!depth && ply == 0 && tree->pv[ply].length) {
		++tree->cached_moves[ply];
		*(tree->move_ptr[ply]++) = tree->pv[ply].moves[0];
		return tree->move_stack[ply];
	    } else {
		chi_move best_move = best_tt_move (tree, 
						   tree->signatures[ply]);

		if (best_move) {
		    ++tree->tt_moves;
		    ++tree->cached_moves[ply];
		    *(tree->move_ptr[ply]++) = best_move;
		    return tree->move_stack[ply];
		}
	    }

	case move_state_pv:

	    mv = chi_generate_captures (&tree->pos, tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_captures;

	    /* FALLTHRU.  */
	    
	case move_state_captures:
	    while (tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }

	    if (!depth)
		break;

            /* FALLTHRU.  */

	case move_state_bonny:
	    ++tree->cached_moves[ply];
	    *(tree->move_ptr[ply]++) = tree->bonny[ply];
	    tree->move_states[ply] = move_state_clyde;
	    return tree->move_stack[ply];
	    
	case move_state_clyde:
	    ++tree->cached_moves[ply];
	    *(tree->move_ptr[ply]++) = tree->clyde[ply];
	    tree->move_states[ply] = move_state_generate_non_captures;
	    return tree->move_stack[ply];

	case move_state_generate_non_captures:

	    mv = chi_generate_pawn_double_steps (&tree->pos, 
						 tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_pawn_double_steps;

            /* FALLTHRU.  */

	case move_state_pawn_double_steps:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }
	    mv = chi_generate_pawn_single_steps (&tree->pos, 
						 tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_pawn_single_steps;
	    /* FALLTRHU.  */

	case move_state_pawn_single_steps:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }
	    mv = chi_generate_knight_moves (&tree->pos, tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_knight_moves;
	    /* FALLTRHU.  */

	case move_state_knight_moves:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }
	    mv = chi_generate_bishop_moves (&tree->pos, tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_bishop_moves;
	    /* FALLTRHU.  */

	case move_state_bishop_moves:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }
	    mv = chi_generate_rook_moves (&tree->pos, tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_rook_moves;
	    /* FALLTRHU.  */

	case move_state_rook_moves:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }
	    mv = chi_generate_king_moves (&tree->pos, tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_king_moves;
	    /* FALLTRHU.  */

	case move_state_king_moves:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }

	    mv = chi_generate_king_castling_moves (&tree->pos, 
						   tree->move_ptr[ply]);
	    tree->moves_left[ply] = mv - tree->move_ptr[ply];
	    tree->move_states[ply] = move_state_king_castlings;

	    /* FALLTRHU.  */

	case move_state_king_castlings:
	    while(tree->moves_left[ply]) {
		int i;

		mv = tree->move_ptr[ply]++;
		--tree->moves_left[ply];

		/* Discard duplicates.  */
		for (i = 0; i < tree->cached_moves[ply]; ++i) {
		    if (tree->move_stack[ply][i] == *mv)
			break;
		}

		if (i == tree->cached_moves[ply])
		    return mv;
	    }
	    /* FALLTRHU.  */

    }

    return NULL;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
