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

static inline chi_bool min2bits_set(bitv64 value);

chi_bool
chi_game_over(chi_pos *pos, chi_result *result)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *end_ptr;
    /* White bishop is on white fields, black bishop ... */
    chi_bool wbisw, bbisw;

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
	if (pos->w_rooks | pos->b_rooks | pos->w_pawns | pos->b_pawns)
		goto no_draw;

	/* We now know that neither queens nor rooks are on the board.  That
	 * means that the bishop bitboards are only for bishops.
	 */

	/* Two pieces on one side can always mate.  */
	if (pos->w_bishops && pos->w_knights)
		goto no_draw;
	if (pos->b_bishops && pos->b_knights)
		goto no_draw;
	if (min2bits_set(pos->w_bishops))
		goto no_draw;
	if (min2bits_set(pos->b_bishops))
		goto no_draw;
	if (min2bits_set(pos->w_knights))
		goto no_draw;
	if (min2bits_set(pos->b_knights))
		goto no_draw;

	/* Neither side has queens, rooks, or pawns. And neither side has more
	 * than one bishop or knight.
	 */
	if (!(pos->w_bishops && pos->b_bishops))
		goto draw;

	/* Both sides have exactly one bishop.  */
    wbisw = (pos->w_bishops & CHI_WHITE_MASK) ? chi_true : chi_false;
    bbisw = (pos->b_bishops & CHI_WHITE_MASK) ? chi_true : chi_false;

	if (wbisw == bbisw)
		goto draw;
	/* FALLTHROUGH */
no_draw:
	if (result) *result = chi_result_unknown;

	return chi_false;

draw:
	if (result) *result = chi_result_draw_by_insufficient_material;

	return chi_true;
}

static inline chi_bool
min2bits_set(bitv64 value)
{
	unsigned count = 0;

	while (value) {
		value &= (value - 1);
		if (count++) return chi_true;
	}

	return chi_false;
}
