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
#define EVAL_LOST_CASTLE (20)

static int evaluate_dev_white PARAMS ((TREE* tree, int ply));
static int evaluate_dev_black PARAMS ((TREE* tree, int ply));

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
    int queen_shelter_pawns = 0;
    int king_shelter_pawns = 0;

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

    /* Penalty for premature queen moves.  */
    if (!(w_queens & CHI_D_MASK & CHI_1_MASK) &&
	((castling_state & 0x3) ||
	 (pos->w_knights & (CHI_B_MASK | CHI_G_MASK) & CHI_1_MASK) ||
	 (w_bishops & (CHI_C_MASK | CHI_F_MASK) & CHI_1_MASK)))
	score -= EVAL_PREMATURE_QUEEN_MOVE;

    /* Calculate a vague value for pawn shelter.  */
    if (w_pawns & CHI_A_MASK & CHI_2_MASK)
	++queen_shelter_pawns;
    if (w_pawns & CHI_B_MASK & CHI_2_MASK)
	++queen_shelter_pawns;
    if (w_pawns & CHI_C_MASK & CHI_2_MASK)
	++queen_shelter_pawns;
    if (w_pawns & CHI_D_MASK & CHI_2_MASK)
	++queen_shelter_pawns;
    queen_shelter_pawns *= 3;

    if (w_pawns & CHI_F_MASK & CHI_2_MASK)
	++king_shelter_pawns;
    if (w_pawns & CHI_G_MASK & CHI_2_MASK)
	++king_shelter_pawns;
    if (w_pawns & CHI_H_MASK & CHI_2_MASK)
	++king_shelter_pawns;
    king_shelter_pawns *= 4;

    if (!(castling_state & 0x4)) {
	/* Penalty for not having castled.  */
	score -= EVAL_NOT_CASTLED;

	/* If we have lost one castling right, give a penalty for
	   losing the good one.  */
	if (!(castling_state & 0x7)) {
	    score -= EVAL_LOST_CASTLE;
	} else {
	    if (castling_state & 0x1) {
		if (queen_shelter_pawns > king_shelter_pawns)
		    score -= EVAL_LOST_CASTLE;
	    } else if (castling_state & 0x2) {
		if (king_shelter_pawns > queen_shelter_pawns)
		    score -= EVAL_LOST_CASTLE;
	    }
	}
    } else {
	/* If we have castled, check if it was done to the best side.  */
	if (pos->w_kings & (CHI_F_MASK | CHI_G_MASK | CHI_H_MASK)) {
	    if (queen_shelter_pawns > king_shelter_pawns)
		score -= EVAL_LOST_CASTLE;
	} else {
	    if (queen_shelter_pawns < king_shelter_pawns)
		score -= EVAL_LOST_CASTLE;
	}
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
    int queen_shelter_pawns = 0;
    int king_shelter_pawns = 0;

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

    /* Penalty for premature queen moves.  */
    if (!(b_queens & CHI_D_MASK & CHI_8_MASK) &&
	((castling_state) ||
	 (pos->b_knights & (CHI_B_MASK | CHI_G_MASK) & CHI_8_MASK) ||
	 (b_bishops & (CHI_C_MASK | CHI_F_MASK) & CHI_8_MASK)))
	score += EVAL_PREMATURE_QUEEN_MOVE;

    /* Calculate a vague value for pawn shelter.  */
    if (b_pawns & CHI_A_MASK & CHI_7_MASK)
	++queen_shelter_pawns;
    if (b_pawns & CHI_B_MASK & CHI_7_MASK)
	++queen_shelter_pawns;
    if (b_pawns & CHI_C_MASK & CHI_7_MASK)
	++queen_shelter_pawns;
    if (b_pawns & CHI_D_MASK & CHI_7_MASK)
	++queen_shelter_pawns;
    queen_shelter_pawns *= 3;

    if (b_pawns & CHI_F_MASK & CHI_7_MASK)
	++king_shelter_pawns;
    if (b_pawns & CHI_G_MASK & CHI_7_MASK)
	++king_shelter_pawns;
    if (b_pawns & CHI_H_MASK & CHI_7_MASK)
	++king_shelter_pawns;
    king_shelter_pawns *= 4;

    if (!(castling_state & 0x40)) {
	/* Penalty for not having castled.  */
	score += EVAL_NOT_CASTLED;

	/* If we have lost one castling right, give a penalty for
	   losing the good one.  */
	if (!(castling_state & 0x70)) {
	    score += EVAL_LOST_CASTLE;
	} else {
	    if (castling_state & 0x10) {
		if (queen_shelter_pawns > king_shelter_pawns)
		    score += EVAL_LOST_CASTLE;
	    } else if (castling_state & 0x20) {
		if (king_shelter_pawns > queen_shelter_pawns)
		    score += EVAL_LOST_CASTLE;
	    }
	}
    } else {
	/* If we have castled, check if it was done to the best side.  */
	if (pos->w_kings & (CHI_F_MASK | CHI_G_MASK | CHI_H_MASK)) {
	    if (queen_shelter_pawns > king_shelter_pawns)
		score += EVAL_LOST_CASTLE;
	} else {
	    if (queen_shelter_pawns < king_shelter_pawns)
		score += EVAL_LOST_CASTLE;
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
