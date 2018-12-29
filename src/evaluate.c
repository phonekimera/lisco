/* evaluate.c - Evaluate a position.
 * Copyright (C) 2002 Guido Flohr (guido@imperia.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <system.h>

#include <error.h>

#include <libchi.h>

#include "brain.h"
#include "board.h"

#ifdef abs
# undef abs
#endif

#define abs(a) ((a) >= 0 ? (a) : -(a))

#define EVAL_BLOCKED_C_PAWN (6)
#define EVAL_BLOCKED_CENTER_PAWN (12)
#define EVAL_PREMATURE_QUEEN_MOVE (12)
#define EVAL_NOT_CASTLED (24)
#define EVAL_LOST_CASTLE (10)

static int evaluate_dev_white PARAMS ((TREE* tree, int ply));
static int evaluate_dev_black PARAMS ((TREE* tree, int ply));
static int evaluate_mobility PARAMS ((TREE* tree));

#include <libchi/bitmasks.h>

#define M1 ((bitv64) 0x5555555555555555)
#define M2 ((bitv64) 0x3333333333333333)

/* Give the compiler a chance to inline this.  */
static unsigned int 
find_first (b)
     bitv64 b;
{
    unsigned int n;

    bitv64 a = b - 1 - (((b - 1) >> 1) & M1);
    bitv64 c = (a & M2) + ((a >> 2) & M2);
    
    n = ((unsigned int) c) + ((unsigned int) (c >> 32));
    n = (n & 0x0f0f0f0f) + ((n >> 4) & 0x0f0f0f0f);
    n = (n & 0xffff) + (n >> 16);
    n = (n & 0xff) + (n >> 8);
    
    return n;    
}

static unsigned int
popcount (b)
     bitv64 b;
{
    unsigned int n;

    bitv64 a = b - ((b >> 1) & M1);
    bitv64 c = (a & M2) + ((a >> 2) & M2);
    
    n = ((unsigned int) c) + ((unsigned int) (c >> 32));
    n = (n & 0x0f0f0f0f) + ((n >> 4) & 0x0f0f0f0f);
    n = (n & 0xffff) + (n >> 16);
    n = (n & 0xff) + (n >> 8);
    
    return n;    
}

#define chi_bitv2shift(b) find_first (b)

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

static const int square_values[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 5, 5, 5, 5, 2, 1,
    1, 2, 5, 9, 9, 5, 2, 1,
    1, 2, 5, 9, 9, 5, 2, 1,
    1, 2, 5, 5, 5, 5, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static const int white_outposts[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  4,  4,  0,  0,  0,
    0,  0,  8, 10, 10,  8,  0,  0,
    0,  0,  8, 12, 12,  8,  0,  0,
    0,  0,  4,  8,  8,  4,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
};

static const int black_outposts[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  4,  8,  8,  4,  0,  0,
    0,  0,  8, 12, 12,  8,  0,  0,
    0,  0,  8, 10, 10,  8,  0,  0,
    0,  0,  0,  4,  4,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
};

int white_king_heat[64] = { 
    1, 0, 2, 3, 3, 2, 0, 1,
    1, 1, 2, 3, 3, 2, 1, 1,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8,
};

int black_king_heat[64] = {
    8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    1, 1, 2, 3, 3, 2, 1, 1,
    1, 0, 2, 3, 3, 2, 0, 1,
};

int
evaluate (tree, ply, alpha, beta)
     TREE* tree;
     int ply;
     int alpha;
     int beta;
{
    chi_pos* pos = &tree->pos;
    int score = 100 * pos->material;

    if (tree->in_check[ply]) {
	chi_move moves[CHI_MAX_MOVES];
	chi_move* mv = moves;

	mv = chi_legal_moves (pos, moves);
	if (mv - moves == 0)
	    return MATE;
    }

    if (pos->half_move_clock >= 100)
	return DRAW;

    ++tree->evals;
    if ((abs (score - alpha) > CHI_VALUE_PAWN) ||
	(abs (score - beta) > CHI_VALUE_PAWN)) {
	++tree->lazy_one_pawn;
    } else {
	int root_castling_state = tree->castling_states[0];

	if (root_castling_state & 0x3)
	    score += evaluate_dev_white (tree, ply);
	if (root_castling_state & 0x30)
	    score += evaluate_dev_black (tree, ply);
    }

    score += evaluate_mobility (tree);

    if (chi_on_move (pos) == chi_white)
	return score;
    else
	return -score;
}

static
int evaluate_dev_white (tree, ply)
     TREE* tree;
     int ply;
{
    int score = 0;
    chi_pos* pos = &tree->pos;
    bitv64 w_pawns = pos->w_pawns;
    bitv64 w_bishops = pos->w_bishops & ~pos->w_rooks;
    bitv64 center_pawns = w_pawns & (CHI_D_MASK | CHI_E_MASK);
    bitv64 w_queens = pos->w_bishops & pos->w_rooks;
    int castling_state = tree->castling_states[ply];

    while (w_pawns) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (w_pawns));
	score += white_outposts[from] << 1;
	w_pawns = chi_clear_least_set (w_pawns);
    }

    /* Penalty for premature queen moves.  */
    if (!(w_queens & CHI_D_MASK & CHI_1_MASK) &&
	((castling_state & 0x3) ||
	 (pos->w_knights & (CHI_B_MASK | CHI_G_MASK) & CHI_1_MASK) ||
	 (w_bishops & (CHI_C_MASK | CHI_F_MASK) & CHI_1_MASK)))
	score -= EVAL_PREMATURE_QUEEN_MOVE;

    /* Do not block c pawn in queen pawn openings.  */
    if (!((w_pawns & CHI_D_MASK & CHI_4_MASK) &&
	  (w_pawns & CHI_E_MASK & CHI_4_MASK))) {
	if ((w_pawns & CHI_C_MASK & CHI_2_MASK) &&
	    ((w_bishops | pos->w_knights) & CHI_C_MASK & CHI_3_MASK))
	    score -= EVAL_BLOCKED_C_PAWN;
    }

    /* Penalty for blocked center pawns.  */
    if ((center_pawns << 8) & (pos->w_pieces | pos->b_pieces))
	score -= EVAL_BLOCKED_CENTER_PAWN;

    if (!(castling_state & 0x4)) {
	/* Penalty for not having castled.  */
	score -= EVAL_NOT_CASTLED;

	if (!(castling_state & 0x2))
	    score -= EVAL_LOST_CASTLE;

	if (!(castling_state & 0x1))
	    score -= EVAL_LOST_CASTLE;
    }

    return score;
}

static
int evaluate_dev_black (tree, ply)
     TREE* tree;
     int ply;
{
    int score = 0;
    chi_pos* pos = &tree->pos;
    bitv64 b_pawns = pos->b_pawns;
    bitv64 b_bishops = pos->b_bishops & ~pos->b_rooks;
    bitv64 center_pawns = b_pawns & (CHI_D_MASK | CHI_E_MASK);
    bitv64 b_queens = pos->b_bishops & pos->b_rooks;
    int castling_state = tree->castling_states[ply];

    while (b_pawns) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (b_pawns));
	score -= black_outposts[from] << 1;
	b_pawns = chi_clear_least_set (b_pawns);
    }

    /* Penalty for premature queen moves.  */
    if (!(b_queens & CHI_D_MASK & CHI_8_MASK) &&
	((castling_state & 0x30) ||
	 (pos->b_knights & (CHI_B_MASK | CHI_G_MASK) & CHI_8_MASK) ||
	 (b_bishops & (CHI_C_MASK | CHI_F_MASK) & CHI_8_MASK)))
	score += EVAL_PREMATURE_QUEEN_MOVE;

    /* Do not block c pawn in queen pawn openings.  */
    if (!((b_pawns & CHI_D_MASK & CHI_5_MASK) &&
	  (b_pawns & CHI_E_MASK & CHI_5_MASK))) {
	if ((b_pawns & CHI_C_MASK & CHI_7_MASK) &&
	    ((b_bishops | pos->b_knights) & CHI_C_MASK & CHI_6_MASK))
	    score += EVAL_BLOCKED_C_PAWN;
    }

    /* Penalty for blocked center pawns.  */
    if ((center_pawns >> 8) & (pos->w_pieces | pos->b_pieces))
	score += EVAL_BLOCKED_CENTER_PAWN;

    if (!(castling_state & 0x40)) {
	/* Penalty for not having castled.  */
	score += EVAL_NOT_CASTLED;

	if (!(castling_state & 0x20))
	    score += EVAL_LOST_CASTLE;

	if (!(castling_state & 0x10))
	    score += EVAL_LOST_CASTLE;
    }

    return score;
}

static int
evaluate_mobility (tree)
     TREE* tree;
{
    chi_pos* pos = &tree->pos;
    int score = 0;
    bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
    bitv64 occ90_squares = pos->w_pieces90 | pos->b_pieces90;
    bitv64 empty_squares = ~occ_squares;
    int total_white_pieces, total_black_pieces;
    register bitv64 piece_mask;

    /* Pawn mobility.  */
    /* Double steps.  */
    piece_mask = pos->w_pawns & CHI_2_MASK;
    piece_mask <<= 8;
    piece_mask &= empty_squares;
    piece_mask <<= 8;
    piece_mask &= empty_squares;
    for (; piece_mask; ++score, piece_mask &= piece_mask - 1);

    piece_mask = pos->b_pawns & CHI_7_MASK;
    piece_mask >>= 8;
    piece_mask &= empty_squares;
    piece_mask >>= 8;
    piece_mask &= empty_squares;
    for (; piece_mask; --score, piece_mask &= piece_mask - 1);

    /* Single steps.  */
    piece_mask = (pos->w_pawns << 8) & empty_squares;
    for (; piece_mask; ++score, piece_mask &= piece_mask - 1);
    piece_mask = (pos->b_pawns >> 8) & empty_squares;
    for (; piece_mask; --score, piece_mask &= piece_mask - 1);
    
    /* Pawn captures.  */
    piece_mask = ((pos->w_pawns & ~CHI_H_MASK) << 7) & occ_squares;
    for (; piece_mask; ++score, piece_mask &= piece_mask - 1);
    piece_mask = ((pos->b_pawns & ~CHI_H_MASK) >> 9) & occ_squares;
    for (; piece_mask; --score, piece_mask &= piece_mask - 1);
    piece_mask = ((pos->w_pawns & ~CHI_A_MASK) << 9) & occ_squares;
    for (; piece_mask; ++score, piece_mask &= piece_mask - 1);
    piece_mask = ((pos->b_pawns & ~CHI_A_MASK) >> 7) & occ_squares;
    for (; piece_mask; --score, piece_mask &= piece_mask - 1);

    /* Knight moves, captures, and defenses.  */
    piece_mask = pos->w_knights;
    while (piece_mask) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 attack_mask = knight_attacks[from];
	for (; attack_mask; ++score, attack_mask &= attack_mask - 1);
	piece_mask = chi_clear_least_set (piece_mask);	
    }

    piece_mask = pos->b_knights;
    while (piece_mask) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 attack_mask = knight_attacks[from];
	for (; attack_mask; --score, attack_mask &= attack_mask - 1);
	piece_mask = chi_clear_least_set (piece_mask);	
    }

    /* Bishop moves, captures, and defenses.  */
    piece_mask = pos->w_bishops & ~(CHI_H_MASK | CHI_8_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask <<= 7;
	for (; target_mask; ++score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_H_MASK | CHI_8_MASK);
    }
    piece_mask = pos->b_bishops & ~(CHI_H_MASK | CHI_8_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask <<= 7;
	for (; target_mask; --score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_H_MASK | CHI_8_MASK);
    }

    piece_mask = pos->w_bishops & ~(CHI_A_MASK | CHI_8_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask <<= 9;
	for (; target_mask; ++score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_A_MASK | CHI_8_MASK);
    }
    piece_mask = pos->b_bishops & ~(CHI_A_MASK | CHI_8_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask <<= 9;
	for (; target_mask; --score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_A_MASK | CHI_8_MASK);
    }

    piece_mask = pos->w_bishops & ~(CHI_H_MASK | CHI_1_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask >>= 9;
	for (; target_mask; ++score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_H_MASK | CHI_1_MASK);
    }
    piece_mask = pos->b_bishops & ~(CHI_H_MASK | CHI_1_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask >>= 9;
	for (; target_mask; --score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_H_MASK | CHI_1_MASK);
    }

    piece_mask = pos->w_bishops & ~(CHI_A_MASK | CHI_1_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask >>= 7;
	for (; target_mask; ++score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_A_MASK | CHI_1_MASK);
    }
    piece_mask = pos->b_bishops & ~(CHI_A_MASK | CHI_1_MASK);
    while (piece_mask) {
	bitv64 target_mask = piece_mask >>= 7;
	for (; target_mask; --score, target_mask &= target_mask - 1);
	piece_mask &= empty_squares & ~(CHI_A_MASK | CHI_1_MASK);
    }

    piece_mask = pos->w_rooks;
    while (piece_mask) {
	int from = chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 state = (rank_masks[from] & occ_squares) >> 
	    ((from >> 3) << 3);
	bitv64 state90 = (file_masks[from] & occ90_squares) >> 
	    ((rotate90[from] >> 3) << 3);
	bitv64 target_mask = rook_hor_slide_masks[from][state] |
	    rook_ver_slide_masks[from][state90];
	target_mask |= (rook_hor_attack_masks[from][state] & occ_squares);
	target_mask |= (rook_ver_attack_masks[from][state90] & occ_squares);

	for (; target_mask; ++score, target_mask &= target_mask - 1);
	piece_mask = chi_clear_least_set (piece_mask);
    }
    piece_mask = pos->b_rooks;
    while (piece_mask) {
	int from = chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 state = (rank_masks[from] & occ_squares) >> 
	    ((from >> 3) << 3);
	bitv64 state90 = (file_masks[from] & occ90_squares) >> 
	    ((rotate90[from] >> 3) << 3);
	bitv64 target_mask = rook_hor_slide_masks[from][state] |
	    rook_ver_slide_masks[from][state90];
	target_mask |= (rook_hor_attack_masks[from][state] & occ_squares);
	target_mask |= (rook_ver_attack_masks[from][state90] & occ_squares);

	for (; target_mask; --score, target_mask &= target_mask - 1);
	piece_mask = chi_clear_least_set (piece_mask);
    }

    total_white_pieces = popcount (pos->w_pieces);
    total_black_pieces = popcount (pos->b_pieces);

    if (chi_on_move (pos) == chi_white) {
	if (total_black_pieces > 10 || 
	    ((total_white_pieces + total_black_pieces) > 20)) {
	    int king_shift = 
		chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 king_wall;

	    for (king_wall = pos->w_pawns & ((bitv64) 0x7) << (king_shift + 7);
		 king_wall;
		 score += 2, king_wall &= king_wall - 1);
	    for (king_wall = pos->w_pieces & ((bitv64) 0x7) << (king_shift + 7);
		 king_wall;
		 score += 2, king_wall &= king_wall - 1);
	    
	    score -= white_king_heat[king_shift];
	}
    } else {
	if (total_white_pieces > 10 || 
	    ((total_white_pieces + total_black_pieces) > 20)) {
	    int king_shift = 
		chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 king_wall;

	    for (king_wall = pos->b_pawns & ((bitv64) 0x7) >> (king_shift - 9);
		 king_wall;
		 score -= 2, king_wall &= king_wall - 1);
	    for (king_wall = pos->b_pieces & ((bitv64) 0x7) >> (king_shift - 9);
		 king_wall;
		 score -= 2, king_wall &= king_wall - 1);
	    
	    score += white_king_heat[king_shift];
	}
    }

    return score;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
