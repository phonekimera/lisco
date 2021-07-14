/* This file is part of the chess engine tate.
 *
 * Copyright (C) 2002-2021 cantanea EOOD.
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
chi_init_position (pos)
     chi_pos* pos;
{
	chi_clear_position (pos);
	
	pos->w_pawns = CHI_2_MASK;
	pos->b_pawns = CHI_7_MASK;
	
	pos->w_knights = (CHI_1_MASK & CHI_B_MASK) | 
		(CHI_1_MASK & CHI_G_MASK);
	pos->b_knights = (CHI_8_MASK & CHI_B_MASK) |
		(CHI_8_MASK & CHI_G_MASK);
	
	pos->w_bishops = (CHI_1_MASK & CHI_C_MASK) |
		(CHI_1_MASK & CHI_D_MASK) |
		(CHI_1_MASK & CHI_F_MASK);
	pos->b_bishops = (CHI_8_MASK & CHI_C_MASK) |
		(CHI_8_MASK & CHI_D_MASK) |
		(CHI_8_MASK & CHI_F_MASK);
	
	pos->w_rooks = (CHI_1_MASK & CHI_A_MASK) |
		(CHI_1_MASK & CHI_D_MASK) |
		(CHI_1_MASK & CHI_H_MASK);
	
	pos->b_rooks = (CHI_8_MASK & CHI_A_MASK) |
		(CHI_8_MASK & CHI_D_MASK) |
		(CHI_8_MASK & CHI_H_MASK);
	
	pos->w_kings = CHI_E_MASK & CHI_1_MASK;
	pos->b_kings = CHI_E_MASK & CHI_8_MASK;
	
	pos->w_pieces = CHI_1_MASK | CHI_2_MASK;
	pos->b_pieces = CHI_8_MASK | CHI_7_MASK;
	pos->w_pieces90 = CHI_A_MASK | CHI_B_MASK;
	pos->b_pieces90 = CHI_G_MASK | CHI_H_MASK;

	chi_wk_castle (pos) = 1;
	chi_wq_castle (pos) = 1;
	chi_bk_castle (pos) = 1;
	chi_bq_castle (pos) = 1;

	pos->half_move_clock = pos->half_moves = 0;

	pos->irreversible_count = 1;
	pos->irreversible[0] = 0;

	chi_on_move (pos) = chi_white;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
