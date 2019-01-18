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
chi_apply_move(chi_pos *pos, chi_move move)
{
	int result = chi_make_move(pos, move);

	if (result)
		return result;

	if (chi_on_move(pos) == chi_white)
		chi_material(pos) += chi_move_material(move);
	else
		chi_material(pos) -= chi_move_material(move);

	chi_on_move(pos) = !chi_on_move(pos);

	if (chi_move_victim(move) || chi_move_attacker(move) == pawn) {
		pos->irreversible[pos->irreversible_count++] = pos->half_moves;
		pos->half_move_clock = 0;
	} else {
		++pos->half_move_clock;
	}
	return 0;
}
