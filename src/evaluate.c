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

#include <libchi/bitmasks.h>

#define M1 ((bitv64) 0x5555555555555555)
#define M2 ((bitv64) 0x3333333333333333)

/* Give the compiler a chance to inline this.  */
static unsigned int 
find_first(bitv64 b)
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

#if 0
/* FIXME! What was that for? */
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
#endif

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

static const int white_pawn_advances[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	 12,  12,  12,  12,  12,  12,  12,  12,
	 24,  24,  24,  24,  24,  24,  24,  24,
	 48,  48,  48,  48,  48,  48,  48,  48,
	100, 100, 100, 100, 100, 100, 100, 100,
	200, 200, 200, 200, 200, 200, 200, 200,
	  0,   0,   0,   0,   0,   0,   0,   0,
};

static const int black_pawn_advances[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	200, 200, 200, 200, 200, 200, 200, 200,
	100, 100, 100, 100, 100, 100, 100, 100,
	 48,  48,  48,  48,  48,  48,  48,  48,
	 24,  24,  24,  24,  24,  24,  24,  24,
	 12,  12,  12,  12,  12,  12,  12,  12,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
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
evaluate(TREE *tree, int ply, int alpha, int beta)
{
	chi_pos* pos = &tree->pos;
	bitv64 signature = tree->signatures[ply];
	int score;
	int total_white_pieces, total_black_pieces;

	++tree->evals;

	/* Check for a cache hit first.  */
	if (probe_ev(pos, signature, &score)) {
		++tree->ev_hits;
		return score;
	}

	if (tree->in_check[ply]) {
		chi_move moves[CHI_MAX_MOVES];
		chi_move* mv = moves;

		mv = chi_legal_moves (pos, moves);
		if (mv - moves == 0) {
			store_ev_entry (pos, signature, MATE - ply);
			return MATE - ply;
		}
	}

	/* We will miss a stalemate here.  Is that a problem? We will see
	   it at the next ply.  */
	if (pos->half_move_clock >= 100) {
		store_ev_entry (pos, signature, DRAW);
		return DRAW;
	}

	total_white_pieces = popcount (pos->w_pieces);
	total_black_pieces = popcount (pos->b_pieces);

	if (!pos->w_pawns && !pos->b_pawns) {
		/* Check for draw by lack of material.  */
		if (!pos->w_rooks && !pos->b_rooks && 
		    !pos->w_bishops && !pos->b_bishops) {
			store_ev_entry (pos, signature, DRAW);
			return DRAW;
		} else if (!pos->w_rooks && !pos->b_rooks) {
			/* Only bishops and knights left.  We report two knights
			   as not sufficient.  */
			int white_bishops = popcount (pos->w_bishops);
			int black_bishops = popcount (pos->b_bishops);
			int white_knights = popcount (pos->w_knights);
			int black_knights = popcount (pos->b_knights);

			if ((white_bishops < 2 && !white_knights)
			    || (black_bishops < 2 && !black_knights))
				store_ev_entry (pos, signature, DRAW);
			return DRAW;
		}
    }

	score = 100 * chi_material (pos);

    if ((abs (score - alpha) > MAX_POS_SCORE)
	    && (abs (score - beta) > MAX_POS_SCORE)) {
		++tree->lazy_evals;
		if (chi_on_move (pos) == chi_white)
			return score;
		else
	    	return -score;
    } else if (total_white_pieces > 10 && total_black_pieces > 10) {
		if (!(tree->w_castled && tree->b_castled)) {
			score += evaluate_dev_white(tree, ply);
			score += evaluate_dev_black(tree, ply);
		}
	}

	score += evaluate_mobility (tree);

    if (total_black_pieces > 10
	    || ((total_white_pieces + total_black_pieces) > 20)) {
		int king_shift = 
			chi_bitv2shift (chi_clear_but_least_set (pos->w_kings));
		bitv64 king_wall;

		for (king_wall = pos->w_pawns & ((bitv64) 0x7) << (king_shift + 7);
		     king_wall;
		     score += 2, king_wall &= king_wall - 1);
		for (king_wall = pos->w_pieces & ((bitv64) 0x7) << (king_shift + 7);
		     king_wall;
		     score += 2, king_wall &= king_wall - 1);

		score -= white_king_heat[king_shift];
	}

	if (total_white_pieces > 10
	    || ((total_white_pieces + total_black_pieces) > 20)) {
		int king_shift = 
			chi_bitv2shift (chi_clear_but_least_set (pos->b_kings));
		bitv64 king_wall;

		for (king_wall = pos->b_pawns & ((bitv64) 0x7) << (king_shift - 9);
			king_wall;
			score -= 2, king_wall &= king_wall - 1);
		for (king_wall = pos->b_pieces & ((bitv64) 0x7) << (king_shift - 9);
			king_wall;
			score -= 2, king_wall &= king_wall - 1);

		score += black_king_heat[king_shift];
    }

    if (total_white_pieces + total_black_pieces <= 20) {
		/* Preliminary pawn evaluation.  */
		bitv64 piece_mask = pos->w_pawns;
		while (piece_mask) {
			int square = chi_bitv2shift (chi_clear_but_least_set (piece_mask));
			int rank = square >> 3;
			int file = square & 0x7;  /* Really file - 7.  */
			bitv64 front = (((bitv64) 1) << ((rank + 1) << 3)) - 1;
			bitv64 rear = (((bitv64) 1) << ((rank - 1) << 3)) - 1;
			bitv64 ahead_mask = front & (CHI_H_MASK << file);
			bitv64 rear_mask = rear & (CHI_H_MASK << file);
			bitv64 left_mask = (bitv64) 0;
			bitv64 right_mask = (bitv64) 0;
			int pawn_score = white_pawn_advances[square];
			bitv64 rel_mask;

			if (file > 0) left_mask = CHI_H_MASK << (file + 1);
			if (file < 7) right_mask = CHI_H_MASK << (file - 1);

			/* Penalty for foes ahead.  */
			for (rel_mask = ahead_mask & pos->b_pawns; 
				rel_mask;
				rel_mask = chi_clear_least_set (rel_mask), 
				           pawn_score -= (pawn_score >> 1));
			/* Penalty for foes left or right ahead.  */
			for (rel_mask = ahead_mask & pos->b_pawns & (left_mask | right_mask);
				rel_mask;
				rel_mask = chi_clear_least_set (rel_mask),
				           pawn_score -= (pawn_score >> 2));
			/* Bonus for friends left and right behind.  */
			for (rel_mask = rear_mask & pos->w_pawns & (left_mask | right_mask);
				rel_mask;
				rel_mask = chi_clear_least_set (rel_mask),
				           pawn_score += (pawn_score >> 2));

			/* Never give a penalty for bad pawns.  */
			if (score > 0)
				score += pawn_score;

			piece_mask = chi_clear_least_set (piece_mask);
		}

		piece_mask = pos->b_pawns;
		while (piece_mask) {
			int square = chi_bitv2shift (chi_clear_but_least_set (piece_mask));
			int rank = square >> 3;
			int file = square & 0x7;  /* Really file - 7.  */
			bitv64 rear = (((bitv64) 1) << ((rank + 1) << 3)) - 1;
			bitv64 front = (((bitv64) 1) << ((rank - 1) << 3)) - 1;
			bitv64 ahead_mask = front & (CHI_H_MASK << file);
			bitv64 rear_mask = rear & (CHI_H_MASK << file);
			bitv64 left_mask = (bitv64) 0;
			bitv64 right_mask = (bitv64) 0;
			int pawn_score = black_pawn_advances[square];
			bitv64 rel_mask;

			if (file > 0) left_mask = CHI_H_MASK << (file + 1);
			if (file < 7) right_mask = CHI_H_MASK << (file - 1);

			/* Penalty for foes ahead.  */
			for (rel_mask = ahead_mask & pos->w_pawns; 
				rel_mask;
				rel_mask = chi_clear_least_set (rel_mask), 
				           pawn_score -= (pawn_score >> 1));
			/* Penalty for foes left or right ahead.  */
			for (rel_mask = ahead_mask & pos->w_pawns & (left_mask | right_mask);
				rel_mask;
				rel_mask = chi_clear_least_set (rel_mask),
				           pawn_score -= (pawn_score >> 2));
			/* Bonus for friends left and right behind.  */
			for (rel_mask = rear_mask & pos->b_pawns & (left_mask | right_mask);
				rel_mask;
				rel_mask = chi_clear_least_set (rel_mask),
				           pawn_score += (pawn_score >> 2));

			/* Never give a penalty for bad pawns.  */
			if (score > 0) score -= pawn_score;

			piece_mask = chi_clear_least_set (piece_mask);
		}
	}

    if (chi_on_move (pos) != chi_white) score = -score;

	store_ev_entry (pos, signature, score);

	return score;
}

int
evaluate_dev_white(TREE *tree, int ply)
{
	int score = 0;
	chi_pos* pos = &tree->pos;
	bitv64 w_pawns = pos->w_pawns;
	bitv64 w_bishops = pos->w_bishops & ~pos->w_rooks;
	bitv64 center_pawns = w_pawns & (CHI_D_MASK | CHI_E_MASK);
	bitv64 w_queens = pos->w_bishops & pos->w_rooks;

	// FIXME: We can precompute the relevant mask.
	while (w_pawns) {
		unsigned int from = 
			chi_bitv2shift (chi_clear_but_least_set (w_pawns));
		score += white_outposts[from] << 1;
		w_pawns = chi_clear_least_set (w_pawns);
	}

	/* Penalty for premature queen moves.  */
	if (!(w_queens & CHI_D_MASK & CHI_1_MASK)
	    && (!tree->w_castled
		    || (pos->w_knights & (CHI_B_MASK | CHI_G_MASK) & CHI_1_MASK)
			|| (w_bishops & (CHI_C_MASK | CHI_F_MASK) & CHI_1_MASK)))
		score -= EVAL_PREMATURE_QUEEN_MOVE;

	/* Do not block c pawn in queen pawn openings.  */
	if (!((w_pawns & CHI_D_MASK & CHI_4_MASK)
		&& (w_pawns & CHI_E_MASK & CHI_4_MASK))) {
		if ((w_pawns & CHI_C_MASK & CHI_2_MASK) &&
			((w_bishops | pos->w_knights) & CHI_C_MASK & CHI_3_MASK))
			score -= EVAL_BLOCKED_C_PAWN;
		}

		/* Penalty for blocked center pawns.  */
		if ((center_pawns << 8) & (pos->w_pieces | pos->b_pieces))
			score -= EVAL_BLOCKED_CENTER_PAWN;

		if (!tree->w_castled && !chi_w_castled(pos)) {
			/* Penalty for not having castled.  */
			score -= EVAL_NOT_CASTLED;

		if (!chi_wk_castle(pos)) score -= EVAL_LOST_CASTLE;

		if (!chi_wq_castle(pos)) score -= EVAL_LOST_CASTLE;
	}

	return score;
}

int
evaluate_dev_black(TREE *tree, int ply)
{
	int score = 0;
	chi_pos* pos = &tree->pos;
	bitv64 b_pawns = pos->b_pawns;
	bitv64 b_bishops = pos->b_bishops & ~pos->b_rooks;
	bitv64 center_pawns = b_pawns & (CHI_D_MASK | CHI_E_MASK);
	bitv64 b_queens = pos->b_bishops & pos->b_rooks;

	while (b_pawns) {
		unsigned int from = 
			chi_bitv2shift (chi_clear_but_least_set (b_pawns));
		score -= black_outposts[from] << 1;
		b_pawns = chi_clear_least_set (b_pawns);
	}

	/* Penalty for premature queen moves.  */
	if (!(b_queens & CHI_D_MASK & CHI_8_MASK)
	    && (!chi_b_castled (pos)
		    || (pos->b_knights & (CHI_B_MASK | CHI_G_MASK) & CHI_8_MASK)
			|| (b_bishops & (CHI_C_MASK | CHI_F_MASK) & CHI_8_MASK)))
		score += EVAL_PREMATURE_QUEEN_MOVE;

	/* Do not block c pawn in queen pawn openings.  */
    if (!((b_pawns & CHI_D_MASK & CHI_5_MASK) &&
	  (b_pawns & CHI_E_MASK & CHI_5_MASK))) {
		if ((b_pawns & CHI_C_MASK & CHI_7_MASK)
		    && ((b_bishops | pos->b_knights) & CHI_C_MASK & CHI_6_MASK))
			score += EVAL_BLOCKED_C_PAWN;
    	}

	/* Penalty for blocked center pawns.  */
	if ((center_pawns >> 8) & (pos->w_pieces | pos->b_pieces))
		score += EVAL_BLOCKED_CENTER_PAWN;

	if (!tree->b_castled && !chi_b_castled(pos)) {
		/* Penalty for not having castled.  */
		score += EVAL_NOT_CASTLED;

		if (!chi_bk_castle(pos)) score += EVAL_LOST_CASTLE;

		if (!chi_bq_castle(pos)) score += EVAL_LOST_CASTLE;
	}

	return score;
}

int 
evaluate_mobility (TREE *tree)
{
	chi_pos* pos = &tree->pos;
	int score = 0;
	bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
	bitv64 occ90_squares = pos->w_pieces90 | pos->b_pieces90;
	bitv64 empty_squares = ~occ_squares;
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
			chi_bitv2shift (chi_clear_but_least_set(piece_mask));
		bitv64 attack_mask = knight_attacks[from];
		for (; attack_mask; ++score, attack_mask &= attack_mask - 1);
		piece_mask = chi_clear_least_set(piece_mask);	
	}

    piece_mask = pos->b_knights;
    while (piece_mask) {
		unsigned int from = 
			chi_bitv2shift (chi_clear_but_least_set(piece_mask));
		bitv64 attack_mask = knight_attacks[from];
		for (; attack_mask; --score, attack_mask &= attack_mask - 1);
		piece_mask = chi_clear_least_set(piece_mask);	
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
		bitv64 state = (rank_masks[from] & occ_squares) >>  ((from >> 3) << 3);
		bitv64 state90 = (file_masks[from] & occ90_squares) >> 
			((rotate90[from] >> 3) << 3);
		bitv64 target_mask = rook_hor_slide_masks[from][state] |
			rook_ver_slide_masks[from][state90];
		target_mask |= (rook_hor_attack_masks[from][state] & occ_squares);
		target_mask |= (rook_ver_attack_masks[from][state90] & occ_squares);

		for (; target_mask; ++score, target_mask &= target_mask - 1)
			;
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

    return score;
}
