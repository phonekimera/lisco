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

#include <stdlib.h>

#include <libchi.h>

int
chi_print_move (chi_pos *pos, chi_move move, char **buf,
                unsigned int *bufsize, int san)
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
    } else {
	/* SAN.  */
	chi_move legalmoves[CHI_MAX_MOVES];
	chi_move* move_end = chi_legal_moves (&target_pos, legalmoves);
	chi_move* mv;
	int move_from_file = 7 - (chi_move_from (move) % 8);
	int move_from_rank = chi_move_from (move) / 8;
	int move_to = chi_move_to (move);
	chi_piece_t attacker = chi_move_attacker (move);
	int from_file_matches = 0;
	int from_rank_matches = 0;
	int is_ambiguous = 0;
	const char* label = chi_shift2label (chi_move_from (to));

	if (attacker != pawn) {
	    for (mv = legalmoves; mv < move_end; ++mv) {
		if (chi_move_attacker (*mv) != attacker)
		    continue;
		
		if (chi_move_to (*mv) != move_to)
		    continue;

		if (move_from_file == 7 - (chi_move_from (*mv) % 8))
		    ++from_file_matches;
		if (move_from_rank == chi_move_from (*mv) / 8)
		    ++from_rank_matches;
		++is_ambiguous;
	    }
	}

	if (attacker == pawn || ((is_ambiguous > 1) && from_file_matches <= 1))
	    *ptr++ = 'a' + move_from_file;
	if (attacker != pawn && (is_ambiguous > 1) && from_file_matches > 1)
	    *ptr++ = '1' + move_from_rank;
	if (chi_move_victim (move))
	    *ptr++ = 'x';
	if (attacker != pawn || chi_move_victim (move))
	    *ptr++ = label[0];
	*ptr++ = label[1];
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
