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

#include "libchi.h"

chi_bool
chi_game_over(chi_pos *pos, chi_result *result)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *end_ptr;

	end_ptr = chi_legal_moves(pos, moves);
	if (end_ptr == moves) {
		if (result) {
			if (chi_check_check(pos)) {
				if (chi_on_move(pos) == chi_white)
					*result = chi_result_black_mates;
				else
					*result = chi_result_white_mates;
			} else {
				*result = chi_result_stalemate;
			}
		}

		return chi_true;
	}

	/* Draws by insufficient material.  First check whether there *is*
	 * sufficient material.  Checking for rooks will automatically check
	 * for queens as well.
	 */
	if (pos->w_rooks | pos->b_rooks)
		goto no_draw;

	/* We now know that neither queens nor rooks are on the board.  That
	 * means that the bishop bitboards are only for bishops.
	 */

	/* King vs. king?  */
	if (0 == (pos->w_bishops | pos->w_knights | pos->b_bishops | pos->b_knights))
		goto draw;
	
no_draw:
	if (result) *result = chi_result_unknown;

	return chi_false;

draw:
	if (result) *result = chi_result_draw_by_insufficient_material;

	return chi_true;
}