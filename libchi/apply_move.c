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

int
chi_apply_move(chi_pos *pos, chi_move move)
{
	int result = chi_make_move(pos, move);
	int from = chi_move_from(move);
	int to = chi_move_to(move);
	bitv64 to_mask = ((bitv64) 1 << to);
	chi_piece_t attacker = chi_move_attacker(move);

	if (result)
		return result;

	if (chi_on_move(pos) == chi_white)
		chi_material(pos) += chi_move_material(move);
	else
		chi_material(pos) -= chi_move_material(move);

	chi_on_move(pos) = !chi_on_move(pos);

	++pos->half_moves;

	chi_ep(pos) = 0;

	if (chi_move_victim(move) || attacker == pawn) {
		/* Irreversible move.  Reset half-move clock.  */
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
#define INITIAL_ROOKS_MASK (CHI_A1_MASK | CHI_H1_MASK \
                            | CHI_A8_MASK | CHI_H8_MASK)
	} else {
		++pos->half_move_clock;
	}

	/* The sides are already swapped here! Check if the castling right was
	 * lost because of moving a king or rook.
	 */
	if (attacker == king) {
		if (chi_on_move(pos) == chi_black) {
			if (chi_wq_castle(pos)) {
				pos->lost_wq_castle = pos->half_moves;
				chi_wq_castle(pos) = 0;
			}
			if (chi_wk_castle(pos)) {
				pos->lost_wk_castle = pos->half_moves;
				chi_wk_castle(pos) = 0;
			}
		} else {
			if (chi_bq_castle(pos)) {
				pos->lost_bq_castle = pos->half_moves;
				chi_bq_castle(pos) = 0;
			}
			if (chi_bk_castle(pos)) {
				pos->lost_bk_castle = pos->half_moves;
				chi_bk_castle(pos) = 0;
			}
		}
	} else if (attacker == rook) {
		if (chi_on_move(pos) == chi_black) {
			if (chi_wq_castle(pos) && from == CHI_A1) {
				pos->lost_wq_castle = pos->half_moves;
				chi_wq_castle(pos) = 0;
			}
			if (chi_wk_castle(pos) && from == CHI_H1) {
				pos->lost_wk_castle = pos->half_moves;
				chi_wk_castle(pos) = 0;
			}
		} else {
			if (chi_bq_castle(pos) && from == CHI_A8) {
				pos->lost_bq_castle = pos->half_moves;
				chi_bq_castle(pos) = 0;
			}
			if (chi_bk_castle(pos) && from == CHI_H8) {
				pos->lost_bk_castle = pos->half_moves;
				chi_bk_castle(pos) = 0;
			}
		}
	}
	
	/* If a rook was captured, the castling right is also lost.  */
	if (chi_move_victim(move) == rook
	         && (to_mask & INITIAL_ROOKS_MASK)) {
		if (chi_on_move(pos) == chi_white) {
			if (chi_wq_castle(pos) && to_mask == CHI_A1_MASK) {
				pos->lost_wq_castle = pos->half_moves;
				chi_wq_castle(pos) = 0;
			} else if (chi_wk_castle(pos) && to_mask == CHI_H1_MASK) {
				pos->lost_wk_castle = pos->half_moves;
				chi_wk_castle(pos) = 0;
			}
		} else {
			if (chi_bq_castle(pos) && to_mask == CHI_A8_MASK) {
				pos->lost_bq_castle = pos->half_moves;
				chi_bq_castle(pos) = 0;
			} else if (chi_bk_castle(pos) && to_mask == CHI_H8_MASK) {
				pos->lost_bk_castle = pos->half_moves;
				chi_bk_castle(pos) = 0;
			}
		}
	}

	return 0;
}
