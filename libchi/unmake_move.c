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

static const int rotate90[64] = {
	7, 15, 23, 31, 39, 47, 55, 63,
	6, 14, 22, 30, 38, 46, 54, 62,
	5, 13, 21, 29, 37, 45, 53, 61,
	4, 12, 20, 28, 36, 44, 52, 60,
	3, 11, 19, 27, 35, 43, 51, 59,
	2, 10, 18, 26, 34, 42, 50, 58,
	1,  9, 17, 25, 33, 41, 49, 57,
	0,  8, 16, 24, 32, 40, 48, 56,
};

static void unmake_white_move(chi_pos *pos, chi_move move);
static void unmake_black_move(chi_pos *pos, chi_move move);
static inline void restore_black_victim(chi_pos *pos, chi_move move);
static inline void restore_white_victim(chi_pos *pos, chi_move move);
static inline void unmove_white_piece(chi_pos *pos, chi_move move);
static inline void unmove_black_piece(chi_pos *pos, chi_move move);
static inline void uncastle_white(chi_pos *pos, chi_move move);
static inline void uncastle_black(chi_pos *pos, chi_move move);

int
chi_unmake_move (chi_pos *pos, chi_move move)
{
	if (chi_on_move(pos) == chi_white) {
		unmake_white_move(pos, move);
	} else {
		unmake_black_move(pos, move);
	}

	return 0;
}

static void
unmake_white_move(chi_pos *pos, chi_move move)
{
	int from = chi_move_from (move);
	int to = chi_move_to (move);
	bitv64 from_mask = ((bitv64) 1 << from);
	bitv64 to_mask = ((bitv64) 1 << to);
	bitv64 from90_mask = ((bitv64) 1 << rotate90[from]);
	bitv64 to90_mask = ((bitv64) 1 << rotate90[to]);
	chi_piece_t victim = chi_move_victim (move);

	unmove_white_piece(pos, move);

	pos->w_pieces &= ~to_mask;
	pos->w_pieces |= from_mask;
	pos->w_pieces90 &= ~to90_mask;
	pos->w_pieces90 |= from90_mask;

	if (victim) {
		restore_black_victim(pos, move);
	}
}

static void
unmake_black_move(chi_pos *pos, chi_move move)
{
	int from = chi_move_from (move);
	int to = chi_move_to (move);
	bitv64 from_mask = ((bitv64) 1 << from);
	bitv64 to_mask = ((bitv64) 1 << to);
	bitv64 from90_mask = ((bitv64) 1 << rotate90[from]);
	bitv64 to90_mask = ((bitv64) 1 << rotate90[to]);
	chi_piece_t victim = chi_move_victim (move);

	unmove_black_piece(pos, move);
	pos->b_pieces &= ~to_mask;
	pos->b_pieces |= from_mask;
	pos->b_pieces90 &= ~to90_mask;
	pos->b_pieces90 |= from90_mask;

	if (victim) {
		restore_white_victim(pos, move);
	}
}

inline void
restore_black_victim(chi_pos *pos, chi_move move)
{
	int to = chi_move_to(move);
	bitv64 to_mask = ((bitv64) 1 << to);
	bitv64 to90_mask = ((bitv64) 1 << rotate90[to]);
	chi_piece_t victim = chi_move_victim (move);

	if (!chi_move_is_ep(move)) {
		pos->b_pieces |= to_mask;
		pos->b_pieces90 |= to90_mask;
	}

	switch (victim) {
		case pawn:
			if (chi_move_is_ep(move)) {
				int ep_target = to - 8;
				bitv64 ep_mask = ((bitv64) 1) << ep_target;
				bitv64 ep_mask90 = ((bitv64) 1) << rotate90[ep_target];
		
				pos->b_pieces |= ep_mask;
				pos->b_pawns |= ep_mask;
				pos->b_pieces90 |= ep_mask90;
			} else {
				pos->b_pawns |= to_mask;
			}
			break;
		case knight:
			pos->b_knights |= to_mask;
			break;
		case bishop:
			pos->b_bishops |= to_mask;
			break;
		case rook:
			pos->b_rooks |= to_mask;
			break;
		default:  /* Queen.  */
			pos->b_rooks |= to_mask;
			pos->b_bishops |= to_mask;
			break;
	}
}

inline void
restore_white_victim(chi_pos *pos, chi_move move)
{
	int to = chi_move_to(move);
	bitv64 to_mask = ((bitv64) 1 << to);
	bitv64 to90_mask = ((bitv64) 1 << rotate90[to]);
	chi_piece_t victim = chi_move_victim (move);

	if (!chi_move_is_ep(move)) {
		pos->w_pieces |= to_mask;
		pos->w_pieces90 |= to90_mask;
	}

	switch (victim) {
		case pawn:
			if (chi_move_is_ep(move)) {
				int ep_target = to + 8;
				bitv64 ep_mask = ((bitv64) 1) << ep_target;
				bitv64 ep_mask90 = ((bitv64) 1) << rotate90[ep_target];

				pos->w_pieces |= ep_mask;
				pos->w_pawns |= ep_mask;
				pos->w_pieces90 |= ep_mask90;
			} else {
				pos->w_pawns |= to_mask;
			}
			break;
		case knight:
			pos->w_knights |= to_mask;
			break;
		case bishop:
			pos->w_bishops |= to_mask;
			break;
		case rook:
			pos->w_rooks |= to_mask;
			break;
		default: /* Queen.  */
			pos->w_rooks |= to_mask;
			pos->w_bishops |= to_mask;
			break;
	}
}

static inline void
unmove_white_piece(chi_pos *pos, chi_move move)
{
	int to = chi_move_to(move);
	int from = chi_move_from(move);
	bitv64 from_mask = ((bitv64) 1 << from);
	bitv64 to_mask = ((bitv64) 1 << to);
	chi_piece_t promote = chi_move_promote(move);

	switch (chi_move_attacker(move)) {
		case pawn:
			pos->w_pawns |= from_mask;
			switch (promote) {
				case knight:
					pos->w_knights &= ~to_mask;
					break;
				case bishop:
					pos->w_bishops &= ~to_mask;
					break;
				case rook:
					pos->w_rooks &= ~to_mask;
					break;
				case queen:
					pos->w_rooks &= ~to_mask;
					pos->w_bishops &= ~to_mask;
					break;
				default:
					pos->w_pawns &= ~to_mask;
					break;
			}
			break;
		case knight:
			pos->w_knights |= from_mask;
			pos->w_knights &= ~to_mask;
			break;
		case bishop:
			pos->w_bishops |= from_mask;
			pos->w_bishops &= ~to_mask;
			break;
		case rook:
			pos->w_rooks |= from_mask;
			pos->w_rooks &= ~to_mask;
			break;
		case queen:
			pos->w_bishops |= from_mask;
			pos->w_bishops &= ~to_mask;
			pos->w_rooks |= from_mask;
			pos->w_rooks &= ~to_mask;
			break;
		case king:
			pos->w_kings |= from_mask;
			pos->w_kings &= ~to_mask;
			if (from_mask == (CHI_E_MASK & CHI_1_MASK)) {
				uncastle_white(pos, move);
			}
			break;
	}
}

static inline void
unmove_black_piece(chi_pos *pos, chi_move move)
{
	int to = chi_move_to(move);
	int from = chi_move_from(move);
	bitv64 from_mask = ((bitv64) 1 << from);
	bitv64 to_mask = ((bitv64) 1 << to);
	chi_piece_t promote = chi_move_promote(move);

	switch (chi_move_attacker(move)) {
		case pawn:
			pos->b_pawns |= from_mask;
			switch (promote) {
				case knight:
					pos->b_knights &= ~to_mask;
					break;
				case bishop:
					pos->b_bishops &= ~to_mask;
					break;
				case rook:
					pos->b_rooks &= ~to_mask;
					break;
				case queen:
					pos->b_rooks &= ~to_mask;
					pos->b_bishops &= ~to_mask;
					break;
				default:
					pos->b_pawns &= ~to_mask;
					break;
			}
			break;
		case knight:
			pos->b_knights |= from_mask;
			pos->b_knights &= ~to_mask;
			break;
		case bishop:
			pos->b_bishops |= from_mask;
			pos->b_bishops &= ~to_mask;
			break;
		case rook:
			pos->b_rooks |= from_mask;
			pos->b_rooks &= ~to_mask;
			break;
		case queen:
			pos->b_bishops |= from_mask;
			pos->b_bishops &= ~to_mask;
			pos->b_rooks |= from_mask;
			pos->b_rooks &= ~to_mask;
			break;
		case king:
			pos->b_kings |= from_mask;
			pos->b_kings &= ~to_mask;
			if (from_mask == (CHI_E_MASK & CHI_8_MASK)) {
				uncastle_black(pos, move);
			}
			break;
	}
}

static inline void
uncastle_white(chi_pos *pos, chi_move move)
{
	int to = chi_move_to(move);
	bitv64 to_mask = ((bitv64) 1 << to);

	if (to_mask == (CHI_G_MASK & CHI_1_MASK)) {
		bitv64 rook_from_mask = ((bitv64) 1) << (to - 1);
		bitv64 rook_to_mask = ((bitv64) 1) << (to + 1);
		bitv64 rook_from90_mask = ((bitv64) 1) << rotate90[to - 1];
		bitv64 rook_to90_mask = ((bitv64) 1) << rotate90[to + 1];

		chi_w_castled(pos) = 0;

		pos->w_rooks &= ~rook_to_mask;
		pos->w_rooks |= rook_from_mask;
		pos->w_pieces &= ~rook_to_mask;
		pos->w_pieces |= rook_from_mask;
		pos->w_pieces90 &= ~rook_to90_mask;
		pos->w_pieces90 |= rook_from90_mask;
	} else if (to_mask == (CHI_C_MASK & CHI_1_MASK)) {
		bitv64 rook_from_mask = ((bitv64) 1) << (to + 2);
		bitv64 rook_to_mask = ((bitv64) 1) << (to - 1);
		bitv64 rook_from90_mask = ((bitv64) 1) << rotate90[to + 2];
		bitv64 rook_to90_mask = ((bitv64) 1) <<
		rotate90[to - 1];

		chi_w_castled(pos) = 0;

		pos->w_rooks &= ~rook_to_mask;
		pos->w_rooks |= rook_from_mask;
		pos->w_pieces &= ~rook_to_mask;
		pos->w_pieces |= rook_from_mask;
		pos->w_pieces90 &= ~rook_to90_mask;
		pos->w_pieces90 |= rook_from90_mask;
	}
}

static inline void
uncastle_black(chi_pos *pos, chi_move move)
{
	int to = chi_move_to(move);
	bitv64 to_mask = ((bitv64) 1 << to);

	if (to_mask == (CHI_G_MASK & CHI_8_MASK)) {
		bitv64 rook_from_mask = ((bitv64) 1) << (to - 1);
		bitv64 rook_to_mask = ((bitv64) 1) << (to + 1);
		bitv64 rook_from90_mask = ((bitv64) 1) << rotate90[to - 1];
		bitv64 rook_to90_mask = ((bitv64) 1) << rotate90[to + 1];

		chi_b_castled(pos) = 0;

		pos->b_rooks &= ~rook_to_mask;
		pos->b_rooks |= rook_from_mask;
		pos->b_pieces &= ~rook_to_mask;
		pos->b_pieces |= rook_from_mask;
		pos->b_pieces90 &= ~rook_to90_mask;
		pos->b_pieces90 |= rook_from90_mask;
	} else if (to_mask == (CHI_C_MASK & CHI_8_MASK)) {
		bitv64 rook_from_mask = ((bitv64) 1) << (to + 2);
		bitv64 rook_to_mask = ((bitv64) 1) << (to - 1);
		bitv64 rook_from90_mask = ((bitv64) 1) << rotate90[to + 2];
		bitv64 rook_to90_mask = ((bitv64) 1) << rotate90[to - 1];

		chi_b_castled(pos) = 0;

		pos->b_rooks &= ~rook_to_mask;
		pos->b_rooks |= rook_from_mask;
		pos->b_pieces &= ~rook_to_mask;
		pos->b_pieces |= rook_from_mask;
		pos->b_pieces90 &= ~rook_to90_mask;
		pos->b_pieces90 |= rook_from90_mask;
	}
}