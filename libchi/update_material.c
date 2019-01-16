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

void
chi_update_material (pos)
     chi_pos* pos;
{
    int score = 0;

    bitv64 piece_mask;

    piece_mask = pos->w_pawns;
    while (piece_mask) {
	++score;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->b_pawns;
    while (piece_mask) {
	--score;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->w_knights;
    while (piece_mask) {
	score += 3;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->b_knights;
    while (piece_mask) {
	score -= 3;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->w_bishops;
    while (piece_mask) {
	score += 3;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->b_bishops;
    while (piece_mask) {
	score -= 3;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->w_rooks;
    while (piece_mask) {
	score += 5;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->b_rooks;
    while (piece_mask) {
	score -= 5;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->w_rooks & pos->w_bishops;
    while (piece_mask) {
	++score;
	piece_mask &= (piece_mask - 1);
    }
    piece_mask = pos->b_rooks & pos->b_bishops;
    while (piece_mask) {
	--score;
	piece_mask &= (piece_mask - 1);
    }

    chi_material (pos) = score;
}


/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
