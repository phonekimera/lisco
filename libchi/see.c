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

/*
 * This file gets included by movegen.c so because both need the
 * bitmasks from "bitmasks.c".  Yes, this is ugly ...
 */

#include <sys/types.h>

/* This source file gets #include'd itself by movegen.c which already
 * included bitmasks.c.  But since an IDE does not know this, we include
 * it hear once more in order to shut up warnings.
 */
#include "bitmasks.c"

#include "libchi.h"
#include "./magicmoves.h"

bitv64
chi_obvious_attackers(const chi_pos *pos, chi_move mv,
	unsigned *white_attackers, unsigned *black_attackers)
{
	int to = chi_move_to(mv);
	bitv64 not_from_mask = ~(1ULL << chi_move_from(mv));
	bitv64 mask;
	/* In case of an en passant hit, we have to remove the victim from the
	 * occupancy because it obscures parts of its file.  The victim is
	 * EP_TO_OFFSETS away from the actual TO square depending on which site
	 * is on move (does the hit).
	 *
	 * The funny bit fiddling with the ep flag is there to avoid a
	 * a multiplication and a branch:
	 *
	 * If a pawn is hit en passant, it is either -8 or +8 squares "away"
	 * from where it gets hit, depending on whether it is a white or black
	 * pawn.  This is defined in EP_TO_OFFSETS.  And because chi_on_move()
	 * extracts just one single bit from a CHI_POS, that is either 0 or 1,
	 * it can be used as an index into EP_TO_OFFSETS.
	 *
	 * Therefore, TO + this offset is the index of the piece/pawn to remove
	 * from the (bit)board, and shifting 1ULL index bits to the left turns
	 * that into a 1-bit mask for the square of the piece/pawn to be removed.
	 *
	 * CHI_MOVE_IS_EP(MV) isolates the en-passant bit of the position, it is
	 * defined as MV & 0x1000, in other words a 1 shifted left by 12 where
	 * 12 is CHI_MOVE_EP_OFFSET.  This is undone here (by right-shift) so
	 * that the flag becomes a boolean, either 0 or 1.  Multiplying it with
	 * -1 turns it either into 0 or an 64 bit integer with all bits set.
	 *
	 * This 64 bit integer is now ANDed with 1-bit mask obtained above, for
	 * the target square of the pawn that did the double step.  The result of
	 * this operation is either 0 if en passant is not possible for the current
	 * position (the most likely case) or it is a bitboard where all bits are
	 * set except for the one that represents the target square of the pawn's
	 * double step.
	 *
	 * This is now bitwisely removed from the ORed masks of the white and
	 * black pieces, so that OCCUPANCY now represents all pieces that are
	 * effectively on the board.
	 *
	 * Why all that hassle? For a normal capture, the position of the victim
	 * is ignored.  Its place is taken by all pieces taking part in the
	 * capture sequence.  But for en passant hits, the pawn that just did
	 * the double step has to be removed because it may obscure sliding
	 * pieces that can attack the en passant square.
	 */
	int ep_to_offsets[2] = { -8, +8 };
	bitv64 occupancy = (pos->w_pieces | pos->b_pieces)
		& ~(-((bitv64) (chi_move_is_ep(mv) >> chi_move_ep_offset))
		    & (1ULL << (to + ep_to_offsets[chi_on_move(pos)])));
	bitv64 bishop_mask = Bmagic(to, occupancy) & not_from_mask;
	bitv64 rook_mask = Rmagic(to, occupancy) & not_from_mask;
	bitv64 queen_mask = bishop_mask | rook_mask;

	/* White pawn captures.  */
	mask = reverse_pawn_attacks[chi_white][to] & pos->w_pawns & not_from_mask;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*white_attackers++ = from | pawn << 8;
		mask = chi_clear_least_set(mask);
	}

	/* White knight and bishop captures.  */
	mask = knight_attacks[to] & pos->w_knights & not_from_mask;
	mask |= bishop_mask & pos->w_bishops & ~pos->w_rooks;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*white_attackers++ = from | knight << 8;
		mask = chi_clear_least_set(mask);
	}

	/* White rook captures.  */
	mask = rook_mask & ~pos->w_bishops & pos->w_rooks;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*white_attackers++ = from | rook << 8;
		mask = chi_clear_least_set(mask);
	}

	/* White queen captures.  */
	mask = queen_mask & pos->w_bishops & pos->w_rooks;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*white_attackers++ = from | queen << 8;
		mask = chi_clear_least_set(mask);
	}

	/* White king captures.  */
	mask = king_attacks[to] & pos->w_kings & not_from_mask;
	if (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*white_attackers++ = from | king << 8;
	}

	*white_attackers++ = 0;

	/* Black pawn captures.  */
	mask = reverse_pawn_attacks[chi_black][to] & pos->b_pawns & not_from_mask;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*black_attackers++ = from | pawn << 8;
		mask = chi_clear_least_set(mask);
	}

	/* Black knight and bishop captures.  */
	mask = knight_attacks[to] & pos->b_knights & not_from_mask;
	mask |= bishop_mask & pos->b_bishops & ~pos->b_rooks;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*black_attackers++ = from | knight << 8;
		mask = chi_clear_least_set(mask);
	}

	/* Black rook captures.  */
	mask = rook_mask & ~pos->b_bishops & pos->b_rooks;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*black_attackers++ = from | rook << 8;
		mask = chi_clear_least_set(mask);
	}

	/* Black queen captures.  */
	mask = queen_mask & pos->b_bishops & pos->b_rooks;
	while (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*black_attackers++ = from | queen << 8;
		mask = chi_clear_least_set(mask);
	}

	/* Black king captures.  */
	mask = king_attacks[to] & pos->b_kings & not_from_mask;
	if (mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(mask));
		*black_attackers++ = from | king << 8;
	}

	*black_attackers++ = 0;

	return occupancy & not_from_mask;
}

int
max(int a, int b) {
	int diff = a - b;
	int dsgn = diff >> 31;

	return a - (diff & dsgn);
}

int
chi_see(chi_pos *position, chi_move move, unsigned *piece_values)
{
	unsigned attackers[32];
	int gain[32];
	off_t depth = 0;
	chi_color_t side_to_move = ~chi_on_move(position) & 0x1;

	bitv64 occupancy =
			chi_obvious_attackers(position, move, attackers, attackers + 16);

	gain[0] = piece_values[chi_move_victim(move)];

	off_t attacker = chi_move_attacker(move);

	unsigned promote = chi_move_promote(move);
	if (promote) {
		/* unlikely */
		gain[0] += piece_values[promote] - piece_values[pawn];
		attacker = promote;
	}

	/* One more reason to combine bishops and rooks of both sides into
	 * one bitboard each.
	 */
	bitv64 sliding_mask = position->w_bishops | position->w_rooks
			| position->b_bishops | position->b_rooks;
	bitv64 sliding_bishops_mask = position->w_bishops | position->b_bishops;
	bitv64 sliding_rooks_mask = position->w_rooks | position->b_rooks;
	unsigned to = chi_move_to(move);

	/*
	 * These two pointers get incremented in turn.  If an x-ray attack is
	 * detected, the currently moving piece gets replaced with the piece
	 * that had been obscured, and the obscured piece is moved to its
	 * correct position by continuous swaps.
	 */
	unsigned *attackers_ptr[2] = { attackers, attackers + 16 };

	while(1) {
		++depth;

		gain[depth] = piece_values[attacker] - gain[depth - 1];

		unsigned attacker_def = *(attackers_ptr[side_to_move]++);
		if (!attacker_def) break;

		// FIXME! Maybe cast attacker_def to an array of two shorts.
		attacker = attacker_def >> 8;
		unsigned from = attacker_def & 0xff;

		/* Can we prune? */
		if (max(-gain[depth - 1], gain[depth]) < 0)
			break;

		occupancy &= ~(1ULL << from);

		/* Add x-ray attackers.  */
		bitv64 obscured_mask = obscured_masks[from][to];
		if (sliding_mask & obscured_mask) {
			/* This is now the slow but unlikely part.  */
			unsigned direction = obscurance_directions[from][to];
			unsigned is_bishop = direction & 1;
			direction >>= 1;
			bitv64 mask = 0, piece_mask = 0;
			chi_color_t color = chi_white;
			chi_piece_t piece = bishop;

			if (is_bishop
			    && (obscured_mask & sliding_bishops_mask)) {
				mask = sliding_bishops_mask & Bmagic(to, occupancy) & occupancy;
			} else if (!is_bishop
			           && (obscured_mask & sliding_rooks_mask)) {
				mask = sliding_rooks_mask & Rmagic(to, occupancy) & occupancy;
				piece = rook;
			}
			if (obscured_mask & mask) {
				if (direction == 0) {
					/* Right/lower.  Most significant bit.  */
					piece_mask = chi_clear_but_most_set(obscured_mask & mask);
				} else {
					/* Left/higher.  Least significant bit.  */
					piece_mask = chi_clear_but_least_set(obscured_mask & mask);
				}
				if (piece_mask) {
					if (piece_mask & position->b_pieces) {
						color = chi_black;
						if (piece_mask & position->b_bishops & position->b_rooks) {
							piece = queen;
						}
					} else {
						if (piece_mask & position->w_bishops & position->w_rooks) {
							piece = queen;
						}
					}

					/* Now insert the x-ray attacker into the list.  Since the
					* piece is encoded in the upper byte, we can do a simple,
					* unmasked comparison.
					*/
	#define swap(a, b) ((((a) -= (b)), ((b) += (a)), ((a) = (b) - (a))))
					unsigned *ptr = --attackers_ptr[color];
					*ptr = piece << 16 | chi_bitv2shift(piece_mask);
					while (*++ptr) {
						if (ptr[0] < ptr[-1]) {
							swap(ptr[-1], ptr[0]);
						}
					}
				}
			}
		}

		side_to_move = ~side_to_move & 0x1;
	}

	while (--depth) {
		gain[depth - 1]= -max(-gain[depth - 1], gain[depth]);
	}

	return gain[0];
}
