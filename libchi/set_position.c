/* set_piece.c - Set a position according to a FEN position
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

static const int rotate90[64] = {
     7, 15, 23, 31, 39, 47, 55, 63,
     6, 14, 22, 30, 38, 46, 54, 62,
     5, 13, 21, 29, 37, 45, 53, 61,
     4, 12, 20, 28, 36, 44, 52, 60,
     3, 11, 19, 27, 35, 43, 51, 59,
     2, 10, 18, 26, 34, 42, 50, 58,
     1,  9, 17, 25, 33, 41, 49, 57,
     0,  8, 16, 24, 32, 40, 48, 56,
};

int
chi_set_position (argpos, fen)
     chi_pos* argpos;
     const char* fen;
{
    int file = 0;
    int rank = 0;
    int shift = 63;
    bitv64 mask = ((bitv64) 1) << shift;
    const char* ptr = fen;
    int material = 0;
    int ep_file = -1;
    unsigned long num;
    char* num_end;
    chi_pos tmp_pos;
    chi_pos* pos = &tmp_pos;

    if (!argpos || !fen)
	return CHI_ERR_YOUR_FAULT;

    chi_clear_position (pos);

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

    while (shift >= 0) {
	switch (*ptr) {
	    case 'P':
		if (rank == 0 || rank == 7)
		    return CHI_ERR_ILLEGAL_FEN;
		pos->w_pawns |= mask;
		material += 1;
		break;
	    case 'p':
		if (rank == 0 || rank == 7)
		    return CHI_ERR_ILLEGAL_FEN;
		pos->b_pawns |= mask;
		material -= 1;
		break;
	    case 'N':
		material += 3;
		pos->w_knights |= mask;
		break;
	    case 'n':
		material -= 3;
		pos->b_knights |= mask;
		break;
	    case 'B':
		material += 3;
		pos->w_bishops |= mask;
		break;
	    case 'b':
		material -= 3;
		pos->b_bishops |= mask;
		break;
	    case 'R':
		material += 5;
		pos->w_rooks |= mask;
		break;
	    case 'r':
		material -= 5;
		pos->b_rooks |= mask;
		break;
	    case 'Q':
		material += 9;
		pos->w_bishops |= mask;
		pos->w_rooks |= mask;
		break;
	    case 'q':
		material -= 9;
		pos->b_bishops |= mask;
		pos->b_rooks |= mask;
		break;
	    case 'K':
		if (pos->w_kings)
		    return CHI_ERR_TWO_KINGS;
		pos->w_kings |= mask;
		break;
	    case 'k':
		if (pos->b_kings)
		    return CHI_ERR_TWO_KINGS;
		pos->b_kings |= mask;
		break;
	    case '1':
		break;
	    case '2':
		file += 1;
		break;
	    case '3':
		file += 2;
		break;
	    case '4':
		file += 3;
		break;
	    case '5':
		file += 4;
		break;
	    case '6':
		file += 5;
		break;
	    case '7':
		file += 6;
		break;
	    case '8':
		file += 7;
		break;
	    case '/':
		++rank;
		file = -1;
		break;
	    case ' ':
	    case '\t':
		break;
	    default:
		return CHI_ERR_ILLEGAL_FEN;
	}
	++file;

	if (file > 8 || rank > 8)
	    return CHI_ERR_ILLEGAL_FEN;

	if (*ptr == ' ' || *ptr == '\t') {
	    if (rank != 7)
		return CHI_ERR_ILLEGAL_FEN;
	    break;
	}

	++ptr;
	shift = 63 - (rank * 8 + file);
	mask = ((bitv64) 1) << shift;
    }

    if (!pos->w_kings || !pos->b_kings)
	return CHI_ERR_NO_KING;

    pos->material = material;

    pos->w_pieces = pos->w_pawns | pos->w_knights | pos->w_bishops | 
	pos->w_rooks | pos->w_kings;
    pos->b_pieces = pos->b_pawns | pos->b_knights | pos->b_bishops | 
	pos->b_rooks | pos->b_kings;

    for (shift = 0; shift < 64; ++shift) {
	int shift90 = rotate90[shift];
	bitv64 mask90 = ((bitv64) 1) << shift90;
	
	mask = ((bitv64) 1) << shift;

	if (mask & pos->w_pieces)
	    pos->w_pieces90 |= mask90;
	else if (mask & pos->b_pieces)
	    pos->b_pieces90 |= mask90;
    }

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

    switch (*ptr) {
	case 'w':
	case 'W':
	    chi_on_move (pos) = chi_white;
	    break;
	case 'b':
	case 'B':
	    chi_on_move (pos) = chi_black;
	    break;
	default:
	    return CHI_ERR_ILLEGAL_FEN;
    }
    ++ptr;

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

    while (1) {
	switch (*ptr) {
	    case 'K':
		chi_wk_castle (pos) = 1;
		break;
	    case 'Q':
		chi_wq_castle (pos) = 1;
		break;
	    case 'k':
		chi_bk_castle (pos) = 1;
		break;
	    case 'q':
		chi_bq_castle (pos) = 1;
		break;
	    case '-':
		break;
	    default:
		return CHI_ERR_ILLEGAL_FEN;
	}

	if (*ptr == '-') {
	    ++ptr;
	    break;
	}
	++ptr;
	if (*ptr == ' ')
	    break;
    }

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

    switch (*ptr) {
	case 'a':
	    ep_file = 0;
	    break;
	case 'b':
	    ep_file = 1;
	    break;
	case 'c':
	    ep_file = 2;
	    break;
	case 'd':
	    ep_file = 3;
	    break;
	case 'e':
	    ep_file = 4;
	    break;
	case 'f':
	    ep_file = 5;
	    break;
	case 'g':
	    ep_file = 6;
	    break;
	case 'h':
	    ep_file = 7;
	    break;
	case '-':
	    break;
	default:
	    return CHI_ERR_ILLEGAL_FEN;
    }
    ++ptr;

    if (ep_file >= 0) {
	if (*ptr != '3' && *ptr != '6')
	    return CHI_ERR_ILLEGAL_FEN;
	++ptr;

	chi_ep (pos) = 1;
	chi_ep_file (pos) = ep_file;
    }

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

    num = strtoul (ptr, &num_end, 10);
    if (num_end == ptr)
	return CHI_ERR_ILLEGAL_FEN;
    pos->half_move_clock = num;
    ptr = num_end;

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

    num = strtoul (ptr, &num_end, 10);
    if (num_end == ptr)
	return CHI_ERR_ILLEGAL_FEN;
    pos->half_moves = num;
    ptr = num_end;

    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')
	++ptr;

    /* Trailing garbage? */
    if (*ptr)
	return CHI_ERR_ILLEGAL_FEN;

    /* Side not to move in check? */
    chi_on_move (pos) = !chi_on_move (pos);
    if (chi_check_check (pos))
	return CHI_ERR_IN_CHECK;
    chi_on_move (pos) = !chi_on_move (pos);

    if (chi_wk_castle (pos) || chi_wq_castle (pos)) {
	if (pos->w_kings != (CHI_E_MASK & CHI_1_MASK))
	    return CHI_ERR_ILLEGAL_CASTLING_STATE;
	if (chi_wk_castle (pos)) {
	    if (!(pos->w_rooks & CHI_H_MASK & CHI_1_MASK))
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	    if (pos->w_bishops & CHI_H_MASK & CHI_1_MASK)
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	}
	if (chi_wq_castle (pos)) {
	    if (!(pos->w_rooks & CHI_A_MASK & CHI_1_MASK))
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	    if (pos->w_bishops & CHI_A_MASK & CHI_1_MASK)
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	}
    }

    if (chi_bk_castle (pos) || chi_bq_castle (pos)) {
	if (pos->b_kings != (CHI_E_MASK & CHI_8_MASK))
	    return CHI_ERR_ILLEGAL_CASTLING_STATE;
	if (chi_bk_castle (pos)) {
	    if (!(pos->b_rooks & CHI_H_MASK & CHI_8_MASK))
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	    if (pos->b_bishops & CHI_H_MASK & CHI_8_MASK)
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	}
	if (chi_bq_castle (pos)) {
	    if (!(pos->b_rooks & CHI_A_MASK & CHI_8_MASK))
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	    if (pos->b_bishops & CHI_A_MASK & CHI_8_MASK)
		return CHI_ERR_ILLEGAL_CASTLING_STATE;
	}
    }

    *argpos = *pos;

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
