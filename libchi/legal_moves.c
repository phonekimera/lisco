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

chi_move*
chi_legal_moves(chi_pos *pos, chi_move *move_stack)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move* move_ptr;
	chi_move* m;

	if (!pos || !move_stack)
		return move_stack;

	move_ptr = chi_generate_captures(pos, moves);
	move_ptr = chi_generate_non_captures(pos, move_ptr);

	for (m = moves; m < move_ptr; ++m) {
		chi_pos tmp_pos;

		chi_copy_pos(&tmp_pos, pos);

		if (chi_move_from(*m) == 3) {
			bitv64 from_mask = ((bitv64) 1) << 3;
			int to = chi_move_to(*m);

		if ((to == 5 || to == 1) && from_mask & pos->w_kings) {
			int tmp_to = (3 + to) >> 1;

		if (chi_check_check(pos))
			continue;

		chi_move_set_to(*m, tmp_to);
		chi_make_move(&tmp_pos, *m);
		if (chi_check_check(&tmp_pos))
			continue;
		chi_copy_pos(&tmp_pos, pos);
		chi_move_set_to(*m, to);
		}
	} else if (chi_move_from(*m) == 59) {
		bitv64 from_mask = ((bitv64) 1) << 59;
		int to = chi_move_to(*m);

		if ((to == 61 || to == 57) && from_mask & pos->b_kings) {
			int tmp_to = (59 + to) >> 1;

			if (chi_check_check(pos))
				continue;

			chi_move_set_to(*m, tmp_to);
			chi_make_move(&tmp_pos, *m);
			if (chi_check_check(&tmp_pos))
				continue;
			chi_copy_pos(&tmp_pos, pos);
			chi_move_set_to(*m, to);
		}
	}

	chi_make_move(&tmp_pos, *m);
	if (chi_check_check(&tmp_pos))
		continue;

		*move_stack++ = *m;
	}

	return move_stack;
}

