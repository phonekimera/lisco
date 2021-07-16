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

#include "bitmasks.h"

chi_move * 
chi_generate_captures(chi_pos *pos, chi_position_context *ctx, chi_move *moves)
{
	if (chi_on_move(pos) == chi_white)
		return chi_generate_white_captures(pos, ctx, moves);
	else
		return chi_generate_black_captures(pos, ctx, moves);
}

chi_move * 
chi_generate_non_captures(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_non_captures(pos, ctx, moves);
	else
		return chi_generate_black_non_captures(pos, ctx, moves);
}

chi_move * 
chi_generate_pawn_double_steps(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_pawn_double_steps(pos, ctx, moves);
	else
		return chi_generate_black_pawn_double_steps(pos, ctx, moves);
}

chi_move * 
chi_generate_pawn_single_steps(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_pawn_single_steps(pos, ctx, moves);
	else
		return chi_generate_black_pawn_single_steps(pos, ctx, moves);
}

chi_move * 
chi_generate_knight_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_knight_moves (pos, ctx, moves);
	else
		return chi_generate_black_knight_moves (pos, ctx, moves);
}

chi_move * 
chi_generate_bishop_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_bishop_moves (pos, ctx, moves);
	else
		return chi_generate_black_bishop_moves (pos, ctx, moves);
}

chi_move * 
chi_generate_rook_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_rook_moves (pos, ctx, moves);
	else
		return chi_generate_black_rook_moves (pos, ctx, moves);
}

chi_move * 
chi_generate_king_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_king_moves (pos, ctx, moves);
	else
		return chi_generate_black_king_moves (pos, ctx, moves);
}

chi_move * 
chi_generate_king_castling_moves(chi_pos *pos, chi_position_context *ctx,
	chi_move *moves)
{
	if (chi_on_move (pos) == chi_white)
		return chi_generate_white_king_castling_moves (pos, ctx, moves);
	else
		return chi_generate_black_king_castling_moves (pos, ctx, moves);
}

int
chi_check_check(chi_pos *pos)
{
	if (chi_on_move (pos) == chi_white)
		return chi_white_check_check (pos);
	else
		return chi_black_check_check (pos);
}

/* Include the code for white and black moves respectively.  */
#define chi_init_color_position_context chi_init_white_position_context
#define chi_generate_color_captures chi_generate_white_captures
#define chi_generate_color_non_captures chi_generate_white_non_captures
#define chi_generate_color_pawn_double_steps \
    chi_generate_white_pawn_double_steps
#define chi_generate_color_pawn_single_steps \
    chi_generate_white_pawn_single_steps
#define chi_generate_color_knight_moves chi_generate_white_knight_moves
#define chi_generate_color_bishop_moves chi_generate_white_bishop_moves
#define chi_generate_color_rook_moves chi_generate_white_rook_moves
#define chi_generate_color_king_castling_moves \
    chi_generate_white_king_castling_moves
#define chi_generate_color_king_moves chi_generate_white_king_moves
#define chi_color_check_check chi_white_check_check
#define MY_PIECES(p) ((p)->w_pieces)
#define HER_PIECES(p) ((p)->b_pieces)
#define MY_PAWNS(p) ((p)->w_pawns)
#define HER_PAWNS(p) ((p)->b_pawns)
#define PAWN_START_MASK CHI_2_MASK
#define MY_KNIGHTS(p) ((p)->w_knights)
#define HER_KNIGHTS(p) ((p)->b_knights)
#define SINGLE_PAWN_OFFSET (8)
#define DOUBLE_PAWN_OFFSET (16)
#define LEFT_PAWN_CAPTURE_OFFSET (9)
#define LEFT_PAWN_CAPTURE_SHIFT(m) ((m) << 9)
#define RIGHT_PAWN_CAPTURE_OFFSET (7)
#define RIGHT_PAWN_CAPTURE_SHIFT(m) ((m) << 7)
#define PAWN_PROMOTE_RANK_MASK CHI_8_MASK
#define PAWN_PRE_PROMOTE_RANK_MASK CHI_7_MASK
#define EP_RANK CHI_RANK_6
#define MY_BISHOPS(p) ((p)->w_bishops)
#define HER_BISHOPS(p) ((p)->b_bishops)
#define MY_ROOKS(p) ((p)->w_rooks)
#define HER_ROOKS(p) ((p)->b_rooks)
#define MY_KINGS(p) ((p)->w_kings)
#define HER_KINGS(p) ((p)->b_kings)
#define QUEEN_CASTLE(p) chi_wq_castle ((p))
#define QUEEN_CASTLE_CROSS_MASK 0x70
#define QUEEN_CASTLE_MOVE (0x03 | 0x05 << 6 | (~king & 0x7) << 13)
#define KING_CASTLE(p) chi_wk_castle ((p))
#define KING_CASTLE_CROSS_MASK 0x06
#define KING_CASTLE_MOVE (0x03 | 0x01 << 6 | (~king & 0x7) << 13)

#include "movegen_color.c"

#undef chi_init_color_position_context
#undef chi_generate_color_captures
#undef chi_generate_color_non_captures
#undef chi_generate_color_pawn_double_steps
#undef chi_generate_color_pawn_single_steps
#undef chi_generate_color_knight_moves
#undef chi_generate_color_bishop_moves
#undef chi_generate_color_rook_moves
#undef chi_generate_color_king_castling_moves
#undef chi_generate_color_king_moves
#undef chi_color_check_check
#undef MY_PIECES
#undef HER_PIECES
#undef MY_PAWNS
#undef HER_PAWNS
#undef PAWN_START_MASK
#undef MY_KNIGHTS
#undef HER_KNIGHTS
#undef SINGLE_PAWN_OFFSET
#undef DOUBLE_PAWN_OFFSET
#undef LEFT_PAWN_CAPTURE_OFFSET
#undef LEFT_PAWN_CAPTURE_SHIFT
#undef RIGHT_PAWN_CAPTURE_OFFSET
#undef RIGHT_PAWN_CAPTURE_SHIFT
#undef PAWN_PROMOTE_RANK_MASK
#undef PAWN_PRE_PROMOTE_RANK_MASK
#undef EP_RANK
#undef MY_BISHOPS
#undef HER_BISHOPS
#undef MY_ROOKS
#undef HER_ROOKS
#undef MY_KINGS
#undef HER_KINGS
#undef QUEEN_CASTLE
#undef QUEEN_CASTLE_CROSS_MASK
#undef QUEEN_CASTLE_MOVE
#undef KING_CASTLE
#undef KING_CASTLE_CROSS_MASK
#undef KING_CASTLE_MOVE

#define chi_init_color_position_context chi_init_black_position_context
#define chi_generate_color_captures chi_generate_black_captures
#define chi_generate_color_non_captures chi_generate_black_non_captures
#define chi_generate_color_pawn_double_steps \
    chi_generate_black_pawn_double_steps
#define chi_generate_color_pawn_single_steps \
    chi_generate_black_pawn_single_steps
#define chi_color_check_check chi_black_check_check
#define chi_generate_color_knight_moves chi_generate_black_knight_moves
#define chi_generate_color_bishop_moves chi_generate_black_bishop_moves
#define chi_generate_color_rook_moves chi_generate_black_rook_moves
#define chi_generate_color_king_castling_moves \
    chi_generate_black_king_castling_moves
#define chi_generate_color_king_moves chi_generate_black_king_moves
#define MY_PIECES(p) ((p)->b_pieces)
#define HER_PIECES(p) ((p)->w_pieces)
#define MY_PAWNS(p) ((p)->b_pawns)
#define HER_PAWNS(p) ((p)->w_pawns)
#define PAWN_START_MASK CHI_7_MASK
#define MY_KNIGHTS(p) ((p)->b_knights)
#define HER_KNIGHTS(p) ((p)->w_knights)
#define SINGLE_PAWN_OFFSET (-8)
#define DOUBLE_PAWN_OFFSET (-16)
#define LEFT_PAWN_CAPTURE_OFFSET (-7)
#define LEFT_PAWN_CAPTURE_SHIFT(m) ((m) >> 7)
#define RIGHT_PAWN_CAPTURE_OFFSET (-9)
#define RIGHT_PAWN_CAPTURE_SHIFT(m) ((m) >> 9)
#define PAWN_PROMOTE_RANK_MASK CHI_1_MASK
#define PAWN_PRE_PROMOTE_RANK_MASK CHI_2_MASK
#define EP_RANK CHI_RANK_3
#define MY_BISHOPS(p) ((p)->b_bishops)
#define HER_BISHOPS(p) ((p)->w_bishops)
#define MY_ROOKS(p) ((p)->b_rooks)
#define HER_ROOKS(p) ((p)->w_rooks)
#define MY_KINGS(p) ((p)->b_kings)
#define HER_KINGS(p) ((p)->w_kings)
#define QUEEN_CASTLE(p) chi_bq_castle ((p))
#define QUEEN_CASTLE_CROSS_MASK (((bitv64) 0x70) << 56)
#define QUEEN_CASTLE_MOVE (0x3b | (0x3d << 6) | ((~king & 0x7) << 13))
#define KING_CASTLE(p) chi_bk_castle ((p))
#define KING_CASTLE_CROSS_MASK (((bitv64) 0x06) << 56)
#define KING_CASTLE_MOVE (0x3b | (0x39 << 6) | ((~king & 0x7) << 13))
#include "movegen_color.c"
