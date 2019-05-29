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
chi_unapply_move(chi_pos *pos, chi_move move)
{
	if (chi_move_victim(move) || chi_move_attacker(move) == pawn) {
		if (pos->irreversible_count > 1) {
			pos->half_move_clock = pos->half_moves - 1
				- pos->irreversible[--pos->irreversible_count - 1];
		} else {
			pos->irreversible_count = 0;
			pos->half_move_clock = pos->irreversible[0] - 1;
		}
	} else {
		--pos->half_move_clock;
	}

	if (pos->double_pawn_move_count
	    && pos->double_pawn_moves[pos->double_pawn_move_count - 1]
		== pos->half_moves) {
		--pos->double_pawn_move_count;
	}

	if (pos->double_pawn_move_count
	    && pos->double_pawn_moves[pos->double_pawn_move_count - 1]
	    == pos->half_moves - 1) {
		--pos->double_pawn_move_count;
		chi_ep(pos) = 1;
		chi_ep_file(pos) = pos->ep_files[pos->double_pawn_move_count];
	} else {
		chi_ep(pos) = 0;
	}

	chi_on_move(pos) = !chi_on_move(pos);

	if (chi_on_move(pos) == chi_black) {
		chi_material(pos) += chi_move_material(move);
	} else {
		chi_material(pos) -= chi_move_material(move);
	}

	if (pos->lost_wk_castle == pos->half_moves)
		chi_wk_castle(pos) = 1;
	if (pos->lost_wq_castle == pos->half_moves)
		chi_wq_castle(pos) = 1;
	if (pos->lost_bk_castle == pos->half_moves)
		chi_bk_castle(pos) = 1;
	if (pos->lost_bq_castle == pos->half_moves)
		chi_bq_castle(pos) = 1;

	--pos->half_moves;

	return chi_unmake_move(pos, move);
}
