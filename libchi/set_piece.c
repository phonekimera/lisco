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

int
chi_set_piece (chi_pos *pos, chi_piece_t piece, chi_color_t color,
               int file, int rank)
{
    int shift = chi_coords2shift (file, rank);
    int shift90 = chi_coords2shift90 (file, rank);
    bitv64 mask = ((bitv64) 1) << shift;
    bitv64 mask90 = ((bitv64) 1) << shift90;

    if (file < CHI_FILE_A || file > CHI_FILE_H
	|| rank < CHI_RANK_1 || rank > CHI_RANK_8)
	return CHI_ERR_YOUR_FAULT;

    pos->w_pieces &= ~mask;
    pos->b_pieces &= ~mask;

    pos->w_pieces90 &= ~mask90;
    pos->b_pieces90 &= ~mask90;

    pos->w_pawns &= ~mask;
    pos->w_knights &= ~mask;
    pos->w_bishops &= ~mask;
    pos->w_rooks &= ~mask;
    pos->w_kings &= ~mask;

    pos->b_pawns &= ~mask;
    pos->b_knights &= ~mask;
    pos->b_bishops &= ~mask;
    pos->b_rooks &= ~mask;
    pos->b_kings &= ~mask;

    if (piece == empty)
	return 0;

    if (color == chi_white) {
	pos->w_pieces |= mask;
	pos->w_pieces90 |= mask90;
    } else {
	pos->b_pieces |= mask;
	pos->b_pieces90 |= mask90;
    }

    switch (piece) {
	case pawn:
	    if (color == chi_white)
		pos->w_pawns |= mask;
	    else
		pos->b_pawns |= mask;
	    break;
	case knight:
	    if (color == chi_white)
		pos->w_knights |= mask;
	    else
		pos->b_knights |= mask;
	    break;
	case bishop:
	    if (color == chi_white)
		pos->w_bishops |= mask;
	    else
		pos->b_bishops |= mask;
	    break;
	case rook:
	    if (color == chi_white)
		pos->w_rooks |= mask;
	    else
		pos->b_rooks |= mask;
	    break;
	case queen:
	    if (color == chi_white) {
		pos->w_bishops |= mask;
		pos->w_rooks |= mask;
	    } else {
		pos->b_bishops |= mask;
		pos->b_rooks |= mask;
	    }
	    break;
	case king:
	    if (color == chi_white)
		pos->w_kings |= mask;
	    else
		pos->b_kings |= mask;
	    break;
	case empty:
	    break;
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
