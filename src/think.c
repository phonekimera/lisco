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

#include <time.h>

#include "state.h"

#define MATE -10000

static int
evaluate(chi_pos *position)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *move_ptr;
	int score = 100 * chi_material(position);
	int factor = chi_on_move(position) ? -1 : +1;
	chi_result result;
	int num_moves;

	if (chi_game_over(position, &result)) {
		if (chi_result_is_white_win(result)) {
			return -MATE;
		} else if (chi_result_is_black_win(result)) {
			return +MATE;
		} else {
			return 0;
		}
	}

	move_ptr = chi_legal_moves(position, moves);
	num_moves = move_ptr - moves;
	score += num_moves * factor;

	// Try a "null move".
	position->flags.on_move = !position->flags.on_move;
	move_ptr = chi_legal_moves(position, moves);
	num_moves = move_ptr - moves;
	score -= num_moves * factor;
	position->flags.on_move = !position->flags.on_move;

	return score;
}

static int seeded = 0;

void
think(void)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *move_ptr;
	int bestscore, score;
	chi_color_t on_move;
	chi_pos position;
	chi_move move;

	if (chi_game_over(&tate.position, NULL)) return;

	position = tate.position;

	on_move = chi_on_move(&position);
	bestscore = on_move == chi_white ? MATE : -MATE;

	if (!seeded) {
		srand(time(NULL));
		seeded = 1;
	}

	move_ptr = chi_legal_moves(&tate.position, moves);
	while (move_ptr-- > moves) {
		move = *move_ptr;
		chi_apply_move(&position, move);
		score = evaluate(&position);

		if ((on_move == chi_white && score >= bestscore)
			|| (on_move == chi_black && score <= bestscore)) {
			bestscore = score;
			tate.bestmove = move;
			tate.bestmove_found = 1;
		}
		chi_unapply_move(&position, move);
	}

	tate.pondermove_found = 0;
}
