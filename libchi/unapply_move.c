/* This file is part of the chess engine lisco.
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

/*
 * FIXME! We can save space and probably time (test!) by distinguishing between
 * pawn double steps and other irreversible moves.  When undoing a pawn double
 * step update the half move clock as well, and only for the other cases
 * of irreversible moves (captures, castlings, losing castling rights), check
 * again.
 */
/* FIXME! Check the size of the array.
 */
/*
 * FIXME! There is no need to clean the arrays double_pawn_moves, ep_files,
 * and irreversible after their respective counters have been decreasee.
 * Instead, write a function that does this cleanup outside of
 * chi_unapply_move() and call this function before calling chi_cmp_pos() in
 * the tests.
 */
int
chi_unapply_move(chi_pos *pos, chi_move move)
{
	if (pos->irreversible[pos->irreversible_count - 1] == pos->half_moves) {
		--pos->irreversible_count;
		/* This is only needed for chi_cmp_pos and should be removed.  */
		pos->irreversible[pos->irreversible_count] = 0;
	}

	pos->half_move_clock = pos->half_moves
			- pos->irreversible[pos->irreversible_count - 1] - 1;

	if (pos->double_pawn_move_count
	    && pos->double_pawn_moves[pos->double_pawn_move_count - 1]
		== pos->half_moves) {
		--pos->double_pawn_move_count;
		/* This is only needed for chi_cmp_pos and should be removed.  */
		pos->double_pawn_moves[pos->double_pawn_move_count] = 0;
		pos->ep_files[pos->double_pawn_move_count] = 0;
	}

	if (pos->double_pawn_move_count
		&& pos->double_pawn_moves[pos->double_pawn_move_count - 1]
	== pos->half_moves - 1) {
		chi_ep(pos) = 1;
		chi_ep_file(pos) = pos->ep_files[pos->double_pawn_move_count - 1];
	} else {
		chi_ep(pos) = 0;
		chi_ep_file(pos) = 0;
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
