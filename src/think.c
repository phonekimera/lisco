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

#include <stdio.h>
#include <string.h>

#include "state.h"

#define MATE -10000
#define INF ((-(MATE)) << 1)
#define MOVEMAX 512

typedef struct Line {
	chi_move moves[MOVEMAX];
	unsigned int num_moves;
} Line;

typedef struct Tree {
	chi_pos position;
	chi_move bestmove;
	int depth;
	unsigned long nodes;
	unsigned long evals;
} Tree;

static int
evaluate(Tree *tree)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *move_ptr;
	chi_pos *position = &tree->position;
	int factor = chi_on_move(position) ? +1 : -1;
	int score = factor * 100 * chi_material(position);
	chi_result result;
	int num_moves;

	++tree->evals;

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
	score -= num_moves;

	// Try a "null move".
	position->flags.on_move = !position->flags.on_move;
	move_ptr = chi_legal_moves(position, moves);
	num_moves = move_ptr - moves;
	score += num_moves;
	position->flags.on_move = !position->flags.on_move;

	return score;
}

/*
static void
print_pv(chi_pos *position, Line *line)
{
	FILE *out = stderr;
	char *buf = NULL;
	unsigned int bufsize;
	chi_move move;

	fprintf(out, "pv");
	for (int i = 0; i < line->cmove; ++i) {
		move = line->moves[i];
		(void) chi_coordinate_notation(move, chi_on_move(position), &buf, &bufsize);
		fprintf(out, " %s", buf);
	}
	fprintf(out, "\n");

	if (buf)
		free(buf);
}
*/

static int
minimax(Tree *tree, int depth)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *move_ptr;
	chi_pos *position = &tree->position;
	int bestvalue, value;

	++tree->nodes;

	if (depth == 0) {
		return evaluate(tree);
	}

	move_ptr = chi_legal_moves(position, moves);
	if (move_ptr == moves) {
		// FIXME! It's not as simple as that. We also have to considers draws.
		return evaluate(tree);
	}

	value = -INF;

	while (move_ptr-- > moves) {
		chi_move move = *move_ptr;

		chi_apply_move(position, move);

		value = -minimax(tree, depth - 1);

		chi_unapply_move(position, move);

		if (value > bestvalue) {
			bestvalue = value;
			if (depth == tree->depth)
				tree->bestmove = move;
		}
	}

	return value;
}

void
think(void)
{
	chi_color_t on_move;
	Tree tree;

	if (chi_game_over(&tate.position, NULL)) return;

	memset(&tree, 0, sizeof tree);

	tree.position = tate.position;
	tree.depth = 4;
	on_move = chi_on_move(&tree.position);

	(void) minimax(&tree, tree.depth);
	tate.bestmove = tree.bestmove;
	tate.bestmove_found = 1;

	tate.pondermove_found = 0;

	fprintf(stderr, "info nodes searched: %lu\n", tree.nodes);
	fprintf(stderr, "info nodes evaluated: %lu\n", tree.evals);
}
