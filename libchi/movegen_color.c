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

/* This is a code fragment, that gets included by movegen.c.  Do
   not use it directly!  */

#ifndef CHI_MOVEGEN_COLOR
# define CHI_MOVEGEN_COLOR 1

#define DIAG045 ((bitv64) 0x8040201008040201)
#define DIAG235 ((bitv64) 0x0102040810204080)
#define FILE_UP_MASK ((bitv64) 0x0101010101010100)
#define FILE_DN_MASK ((bitv64) 0x0080808080808080)
#define RANKMASK ((bitv64) 0xff)

#include "magicmoves.h"

#endif

void
chi_init_color_position_context(chi_pos *pos, chi_position_context *ctx)
{
	memset(ctx, 0, sizeof *ctx);

	bitv64 occupancy = ctx->occupancy = pos->w_pieces | pos->b_pieces;
	ctx->empty = ~ctx->occupancy;

	bitv64 piece_mask;

	piece_mask = MY_ROOKS(pos);
	while(piece_mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(piece_mask));
		bitv64 attack_mask = Rmagic(from, occupancy);
		ctx->rook_attack_masks[ctx->num_rook_attack_masks++] =
				(chi_attack_mask) {
					from,
					attack_mask
				};

		piece_mask = chi_clear_least_set(piece_mask);
	}

	piece_mask = MY_BISHOPS(pos);
	while(piece_mask) {
		int from = chi_bitv2shift(chi_clear_but_least_set(piece_mask));
		bitv64 attack_mask = Bmagic(from, occupancy);
		ctx->bishop_attack_masks[ctx->num_bishop_attack_masks++] =
				(chi_attack_mask) {
					from,
					attack_mask
				};

		piece_mask = chi_clear_least_set(piece_mask);
	}
}

chi_move *
chi_generate_color_captures(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	bitv64 her_squares = HER_PIECES(pos);
	bitv64 empty_squares = ctx->empty;

	bitv64 piece_mask = MY_PAWNS(pos);

	/* Pawn captures.  */
	if (piece_mask) {
		bitv64 target_squares = her_squares;
		int ep_offset = 0;

		if (chi_ep(pos)) {
			ep_offset = chi_coords2shift(chi_ep_file(pos), EP_RANK);
			target_squares |= ((bitv64) 1) << ep_offset;
		}

		while (piece_mask) {
			int from = chi_bitv2shift(chi_clear_but_least_set(piece_mask));
			bitv64 from_mask = ((bitv64) 1) << from;
			bitv64 to_mask = 0;

			/* Pawn promotions.  */
			if (from_mask & PAWN_PRE_PROMOTE_RANK_MASK) {
				bitv64 to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);

				if (to_mask & empty_squares) {
					chi_move move = from | ((from + SINGLE_PAWN_OFFSET) << 6)
							| ((~pawn & 0x7) << 13);

					*moves++ = move | (queen << 19) | (8 << 22);
					*moves++ = move | (rook << 19) | (4 << 22);
					*moves++ = move | (bishop << 19) | (2 << 22);
					*moves++ = move | (knight << 19) | (2 << 22);
				}
			}

			if (from_mask & ~CHI_A_MASK) {
				to_mask = ((bitv64) 1) << (from + LEFT_PAWN_CAPTURE_OFFSET);

				if (to_mask & target_squares) {
					bitv64 target_square = to_mask & target_squares;
					int to = from + LEFT_PAWN_CAPTURE_OFFSET;
					chi_move move = from | (to << 6) | ((~pawn & 0x7) << 13);
					int material = 1 << 22;
					chi_piece_t victim = pawn << 16;
					int ep_flag = 0;
					chi_move filled;

					if (target_square & HER_KNIGHTS(pos)) {
						material = 3 << 22;
						victim = knight << 16;
					} else if (target_square & HER_BISHOPS(pos)) {
						material = 3 << 22;
						victim = bishop << 16;
						if (target_square & HER_ROOKS(pos)) {
							material = 9 << 22;
							victim = queen << 16;
						}
					} else if (target_square & HER_ROOKS(pos)) {
						material = 5 << 22;
						victim = rook << 16;
					} else if (to == ep_offset) {
						ep_flag = 0x1000;
					}

					filled = move | victim | ep_flag | material;

					if (to_mask & PAWN_PROMOTE_RANK_MASK) {
						*moves++ = filled | (queen << 19) | (8 << 22);
						*moves++ = filled | (rook << 19) | (4 << 22);
						*moves++ = filled | (bishop << 19) | (2 << 22);
						*moves++ = filled | (knight << 19) | (2 << 22);
					} else {
						*moves++ = filled;
					}
				}
			}

			if (from_mask & ~CHI_H_MASK) {
				to_mask = ((bitv64) 1) << (from + RIGHT_PAWN_CAPTURE_OFFSET);

				if (to_mask & target_squares) {
					bitv64 target_square = to_mask & target_squares;
					int to = from + RIGHT_PAWN_CAPTURE_OFFSET;
					chi_move move = from | (to << 6) | ((~pawn & 0x7) << 13);
					int material = 1 << 22;
					chi_piece_t victim = pawn << 16;
					int ep_flag = 0;
					chi_move filled;

					if (target_square & HER_KNIGHTS(pos)) {
						material = 3 << 22;
						victim = knight << 16;
					} else if (target_square & HER_BISHOPS(pos)) {
						material = 3 << 22;
						victim = bishop << 16;
						if (target_square & HER_ROOKS(pos)) {
							material = 9 << 22;
							victim = queen << 16;
						}
					} else if (target_square & HER_ROOKS(pos)) {
						material = 5 << 22;
						victim = rook << 16;
					} else if (to == ep_offset) {
						ep_flag = 0x1000;
					}

					filled = move | victim | ep_flag | material;

					if (to_mask & PAWN_PROMOTE_RANK_MASK) {
						*moves++ = filled | (queen << 19) | (8 << 22);
						*moves++ = filled | (rook << 19) | (4 << 22);
						*moves++ = filled | (bishop << 19) | (2 << 22);
						*moves++ = filled | (knight << 19) | (2 << 22);
					} else {
						*moves++ = filled;
					}
				}
			}

			piece_mask = chi_clear_least_set(piece_mask);
		}
	}

	/* King captures.  */
	piece_mask = MY_KINGS(pos);
	while (piece_mask) {
		unsigned int from =
				chi_bitv2shift(chi_clear_but_least_set(piece_mask));
		bitv64 attack_mask = king_attacks[from] & her_squares;

		while (attack_mask) {
			unsigned int to =
					chi_bitv2shift(chi_clear_but_least_set(attack_mask));
			bitv64 dest_mask = ((bitv64) 1) << to;
			int material = 1;
			chi_piece_t victim = pawn;

			if (dest_mask & HER_KNIGHTS(pos)) {
				material = 3;
				victim = knight;
			} else if (dest_mask & HER_BISHOPS(pos)) {
				material = 3;
				victim = bishop;
				if (dest_mask & HER_ROOKS(pos)) {
					material = 9;
					victim = queen;
				}
			} else if (dest_mask & HER_ROOKS(pos)) {
				material = 5;
				victim = rook;
			}

			*moves++ = from | (to << 6) | ((~king & 0x7) << 13) | (victim << 16)
					| (material << 22);

			attack_mask = chi_clear_least_set(attack_mask);
		}
	
		break;
	}

	return moves;
}

chi_move *
chi_generate_color_pawn_double_steps(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	bitv64 piece_mask = PAWN_START_MASK & MY_PAWNS(pos);

	if (piece_mask) {
		bitv64 empty_squares = ~(pos->w_pieces | pos->b_pieces);

		/* Pawn double steps.  */
		while (piece_mask) {
			unsigned int from =
			chi_bitv2shift(chi_clear_but_least_set(piece_mask));
			bitv64 to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);
			
			if (to_mask & empty_squares) {
				to_mask = ((bitv64) 1) << (from + DOUBLE_PAWN_OFFSET);

				if (to_mask & empty_squares) {
					*moves++ = (from | ((from + DOUBLE_PAWN_OFFSET) << 6)
							| ((~pawn & 0x7) << 13));
				}
			}

			piece_mask = chi_clear_least_set(piece_mask);
		}
	}

	return moves;
}

chi_move *
chi_generate_color_pawn_single_steps(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	bitv64 piece_mask = MY_PAWNS(pos) & ~PAWN_PRE_PROMOTE_RANK_MASK;

	if (piece_mask) {
		bitv64 empty_squares = ~(pos->w_pieces | pos->b_pieces);

		while (piece_mask) {
			unsigned int from =
			chi_bitv2shift(chi_clear_but_least_set(piece_mask));
			bitv64 to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);

			if (to_mask & empty_squares)
				*moves++ = (from | ((from + SINGLE_PAWN_OFFSET) << 6)
						| ((~pawn & 0x7) << 13));

			piece_mask = chi_clear_least_set(piece_mask);
		}
	}

	return moves;
}

chi_move *
chi_generate_color_knight_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	bitv64 piece_mask = MY_KNIGHTS(pos);

	if (piece_mask) {
		while (piece_mask) {
			unsigned int from =
			chi_bitv2shift(chi_clear_but_least_set(piece_mask));
			bitv64 attack_mask = knight_attacks[from]
					& (ctx->empty | HER_PIECES(pos));

			while (attack_mask) {
				unsigned int to =
					chi_bitv2shift(chi_clear_but_least_set(attack_mask));
				chi_move move = from | (to << 6) | ((~knight & 0x7) << 13);

				bitv64 to_mask = 1ULL << to;

				if (HER_PIECES(pos) & to_mask) {
					unsigned int material = CHI_VALUE_PAWN / 100;
					chi_piece_t victim = pawn;

					if (to_mask & HER_KNIGHTS(pos)) {
						material = 3;
						victim = knight;
					} else if (to_mask & HER_BISHOPS(pos)) {
						material = 3;
						victim = bishop;
						if (to_mask & HER_ROOKS(pos)) {
							material = 9;
							victim = queen;
						}
					} else if (to_mask & HER_ROOKS(pos)) {
						material = 5;
						victim = rook;
					}

					move |= (victim << 16) | (material << 22);
				}

				*moves++ = move;

				attack_mask = chi_clear_least_set(attack_mask);
			}

			piece_mask = chi_clear_least_set(piece_mask);
		}
	}

	return moves;
}

chi_move *
chi_generate_color_bishop_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	for (size_t i = 0; i < ctx->num_bishop_attack_masks; ++i) {
		chi_attack_mask *attack_mask = &ctx->bishop_attack_masks[i];
		int from = attack_mask->from;
		bitv64 mask = attack_mask->mask & (ctx->empty | HER_PIECES(pos));
		chi_piece_t piece = ((((bitv64) 1) << from) & MY_ROOKS(pos)) ?
				(~queen & 0x7) : (~bishop & 0x7);

		while (mask) {
			int to = chi_bitv2shift(chi_clear_but_least_set(mask));

			chi_move move = (from | (to << 6) | (piece << 13));
			bitv64 to_mask = 1ULL << to;

			/* FIXME! It is probably smarter to go over all moves at once
			 * and fill victims and values.  That will also make it possible
			 * to omit that step altogether, for example when evading a
			 * check or when doing a perft test.
			 */
			if (HER_PIECES(pos) & to_mask) {
				unsigned int material = CHI_VALUE_PAWN / 100;
				chi_piece_t victim = pawn;

				/* FIXME! The first if should be a pawn capture because this
				 * is the most likely one.
				 */
				if (to_mask & HER_KNIGHTS(pos)) {
					material = 3;
					victim = knight;
				} else if (to_mask & HER_BISHOPS(pos)) {
					material = 3;
					victim = bishop;
					if (to_mask & HER_ROOKS(pos)) {
						material = 9;
						victim = queen;
					}
				} else if (to_mask & HER_ROOKS(pos)) {
					material = 5;
					victim = rook;
				}

				move |= (victim << 16) | (material << 22);
			}

			*moves++ = move;
			mask = chi_clear_least_set(mask);
		}
	}

	return moves;
}

chi_move *
chi_generate_color_rook_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	for (size_t i = 0; i < ctx->num_rook_attack_masks; ++i) {
		chi_attack_mask *attack_mask = &ctx->rook_attack_masks[i];
		int from = attack_mask->from;
		bitv64 mask = attack_mask->mask & (ctx->empty | HER_PIECES(pos));
		chi_piece_t piece = ((((bitv64) 1) << from) & MY_BISHOPS(pos)) ?
				(~queen & 0x7) : (~rook & 0x7);

		/* This whole while loop can be factored out because it is identical
		 * to the bishop routine.
		 */
		while (mask) {
			int to = chi_bitv2shift(chi_clear_but_least_set(mask));

			chi_move move = (from | (to << 6) | (piece << 13));
			bitv64 to_mask = 1ULL << to;

			/* FIXME! It is probably smarter to go over all moves at once
			 * and fill victims and values.  That will also make it possible
			 * to omit that step altogether, for example when evading a
			 * check or when doing a perft test.
			 */
			if (HER_PIECES(pos) & to_mask) {
				unsigned int material = CHI_VALUE_PAWN / 100;
				chi_piece_t victim = pawn;

				/* FIXME! The first if should be a pawn capture because this
				 * is the most likely one.
				 */
				if (to_mask & HER_KNIGHTS(pos)) {
					material = 3;
					victim = knight;
				} else if (to_mask & HER_BISHOPS(pos)) {
					material = 3;
					victim = bishop;
					if (to_mask & HER_ROOKS(pos)) {
						material = 9;
						victim = queen;
					}
				} else if (to_mask & HER_ROOKS(pos)) {
					material = 5;
					victim = rook;
				}

				move |= (victim << 16) | (material << 22);
			}
			*moves++ = move;
			mask = chi_clear_least_set(mask);
		}
	}

	return moves;
}

chi_move *
chi_generate_color_king_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	bitv64 piece_mask = MY_KINGS(pos);

	if (piece_mask) {
		bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
		bitv64 empty_squares = ~occ_squares;
		unsigned int from =
			chi_bitv2shift(chi_clear_but_least_set(piece_mask));
		bitv64 attack_mask = king_attacks[from] & empty_squares;

		while (attack_mask) {
			unsigned int to =
			chi_bitv2shift(chi_clear_but_least_set(attack_mask));
			
			*moves++ = from | (to << 6) | ((~king & 0x7) << 13);
			attack_mask = chi_clear_least_set(attack_mask);
		}
	}

	return moves;
}

chi_move*
chi_generate_color_king_castling_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	bitv64 occ_squares = pos->w_pieces | pos->b_pieces;

	if (KING_CASTLE(pos) && (!(occ_squares & KING_CASTLE_CROSS_MASK)))
		*moves++ = KING_CASTLE_MOVE;

	if (QUEEN_CASTLE(pos) && (!(occ_squares & QUEEN_CASTLE_CROSS_MASK)))
		*moves++ = QUEEN_CASTLE_MOVE;

	return moves;
}

chi_move *
chi_generate_color_non_captures(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	moves = chi_generate_color_king_castling_moves(pos, ctx, moves);
	moves = chi_generate_color_pawn_double_steps(pos, ctx, moves);
	moves = chi_generate_color_pawn_single_steps(pos, ctx, moves);
	moves = chi_generate_color_knight_moves(pos, ctx, moves);
	moves = chi_generate_color_bishop_moves(pos, ctx, moves);
	moves = chi_generate_color_rook_moves(pos, ctx, moves);
	moves = chi_generate_color_king_moves(pos, ctx, moves);

	return moves;
}

int
chi_color_check_check(pos)
	chi_pos* pos;
{
	bitv64 king_mask = MY_KINGS(pos);
	register unsigned int king_shift =
			chi_bitv2shift(chi_clear_but_least_set(king_mask));
	bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
	bitv64 her_bishops, her_rooks;

	if (knight_attacks[king_shift] & HER_KNIGHTS(pos))
		return 1;

	if (king_attacks[king_shift] & HER_KINGS(pos))
		return 1;

	/* Pawn attacks.  */
	if (king_mask & ~PAWN_PROMOTE_RANK_MASK) {
		if ((king_mask & ~CHI_A_MASK)
		    && (HER_PAWNS(pos) & (LEFT_PAWN_CAPTURE_SHIFT(king_mask))))
			return 1;
		if ((king_mask & ~CHI_H_MASK)
		    && (HER_PAWNS(pos) & (RIGHT_PAWN_CAPTURE_SHIFT(king_mask))))
			return 1;
	}

	/* Bishop attacks.  */
	her_bishops = HER_BISHOPS(pos) & bishop_king_attacks[king_shift];
	while (her_bishops) {
		int bishop_shift =
				chi_bitv2shift(chi_clear_but_least_set(her_bishops));

		if (!(bishop_king_intermediates[king_shift][bishop_shift]
		    & occ_squares))
			return 1;

		her_bishops = chi_clear_least_set(her_bishops);
	}

	/* Rook attacks.  */
	her_rooks = HER_ROOKS(pos) & rook_king_attacks[king_shift];
	while (her_rooks) {
		int rook_shift =
			chi_bitv2shift(chi_clear_but_least_set(her_rooks));

		if (!(rook_king_intermediates[king_shift][rook_shift] & occ_squares))
			return 1;

		her_rooks = chi_clear_least_set(her_rooks);
	}

	return 0;
}
