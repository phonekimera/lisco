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

	if (pos->half_move_clock >= 100) {
		if (result) *result = chi_result_draw_by_50_moves_rule;
		return chi_true;
	}

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

	if (result) *result = chi_result_unknown;

	return chi_false;
}