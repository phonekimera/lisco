/* print_move.c - ASCII representation(s) of a move.
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

#include <libchi.h>

int
chi_print_move (pos, move, buf, bufsize, san)
     chi_pos* pos;
     chi_move move;
     char** buf;
     unsigned int* bufsize;
     int san;
{
    int from = chi_move_from (move);
    int to = chi_move_to (move);
    char* ptr;
    chi_piece_t piece;
    bitv64 from_mask, to_mask;
    bitv64 is_capture = 0;
    chi_pos target_pos;

    if (from > 63 || to > 63 || !buf || !bufsize || !pos)
	return CHI_ERR_YOUR_FAULT;
    
    target_pos = *pos;

    /* 20 is too much, but play safe.  */
    if (!*buf || *bufsize < 20) {
	char* new_buf = realloc (*buf, 20);
	
	if (!new_buf)
	    return CHI_ERR_ENOMEM;
	
	*buf = new_buf;
	*bufsize = 20;
    }
    
    ptr = *buf;

    /* Determine the piece that moves here.  Its color can be read from 
       POS.  */
    from_mask = ((bitv64) 1 << from);
    to_mask = ((bitv64) 1 << to);
    
    is_capture = to_mask & (target_pos.w_pieces | target_pos.b_pieces);

    if (chi_on_move (&target_pos) == chi_white) {
	if (from_mask & target_pos.w_pawns) {
	    piece = pawn;
	} else if (from_mask & target_pos.w_knights) {
	    piece = knight;
	} else if (from_mask & target_pos.w_bishops) {
	    if (from_mask & target_pos.w_rooks) {
		piece = queen;
	    } else {
		piece = bishop;
	    }
	} else if (from_mask & target_pos.w_rooks) {
	    piece = rook;
	} else if (from_mask & target_pos.w_kings) {
	    piece = king;
	} else {
	    return CHI_ERR_EMPTY_SQUARE;
	}
    } else {
	if (from_mask & target_pos.b_pawns) {
	    piece = pawn;
	} else if (from_mask & target_pos.b_knights) {
	    piece = knight;
	} else if (from_mask & target_pos.b_bishops) {
	    if (from_mask & target_pos.b_rooks) {
		piece = queen;
	    } else {
		piece = bishop;
	    }
	} else if (from_mask & target_pos.b_rooks) {
	    piece = rook;
	} else if (from_mask & target_pos.b_kings) {
	    piece = king;
	} else {
	    return CHI_ERR_EMPTY_SQUARE;
	}
    }

    if (piece != pawn)
	*ptr++ = chi_piece2char (piece);

    if (piece == king && to - from == 2) {
	--ptr;
	*ptr++ = 'O';
	*ptr++ = '-';
	*ptr++ = 'O';
	*ptr++ = '-';
	*ptr++ = 'O';
    } else if (piece == king && from - to == 2) {
	--ptr;
	*ptr++ = 'O';
	*ptr++ = '-';
	*ptr++ = 'O';
    } else if (!san) {
	const char* label = chi_shift2label (from);
	int is_ep = 0;

	*ptr++ = label[0];
	*ptr++ = label[1];

	if (is_capture) {
	    *ptr++ = 'x';
	} else {
	    /* If the pawn has moved an odd number of shifts, but no
	       capture has been detected, it must be an en passant 
	       capture.  */
	    if (piece == pawn && ((from - to) & 1)) {
		is_ep = 1;
		*ptr++ = 'x';
	    } else
		*ptr++ = '-';
	}

	label = chi_shift2label (to);

	*ptr++ = *label++;
	*ptr++ = *label++;

	if (chi_move_promote (move)) {
	    *ptr++ = '=';
	    *ptr++ = chi_piece2char (chi_move_promote (move));
	}
    }

    chi_apply_move (&target_pos, move);
    if (chi_check_check (&target_pos)) {
	chi_move moves[CHI_MAX_MOVES];
	chi_move* mv;

	mv = chi_legal_moves (&target_pos, moves);
	if (mv == moves)
	    *ptr++ = '#';
	else
	    *ptr++ = '+';
    }

    *ptr++ = '\000';

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
