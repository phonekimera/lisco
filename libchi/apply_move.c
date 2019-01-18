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
	int from = chi_move_from(move);
	int to = chi_move_to(move);
	chi_piece_t attacker = chi_move_attacker(move);

	if (result)
		return result;

	if (chi_on_move(pos) == chi_white)
		chi_material(pos) += chi_move_material(move);
	else
		chi_material(pos) -= chi_move_material(move);

	chi_on_move(pos) = !chi_on_move(pos);

	++pos->half_moves;

	if (chi_move_victim(move) || attacker == pawn) {
		pos->irreversible[pos->irreversible_count++] = pos->half_moves;
		pos->half_move_clock = 0;
		if (attacker == pawn && abs(to - from) == 16) {
			int file = 7 - (from % 8);
			chi_ep(pos) = 1;
			chi_ep_file(pos) = file;
			pos->double_pawn_moves[pos->double_pawn_move_count] =
				pos->half_moves;
			pos->ep_files[pos->double_pawn_move_count++] = file;
		}
	} else {
		++pos->half_move_clock;
	}

	return 0;
}
