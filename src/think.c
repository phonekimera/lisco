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

#include <stdio.h>
#include <string.h>

#include "state.h"

#define DEBUG_SEARCH 0

#define MATE -10000
#define INF ((-(MATE)) << 1)
#define MAXDEPTH 512

typedef struct Line {
	chi_move moves[MAXDEPTH];
	unsigned int num_moves;
} Line;

typedef struct Tree {
	chi_pos position;
	chi_move bestmove;
	int depth;
	unsigned long nodes;
	unsigned long evals;

	Line line;
} Tree;

#if DEBUG_SEARCH
static void
print_line(FILE *stream, chi_pos *start, Line *line)
{
	chi_pos position;
	char *buf = NULL;
	unsigned int bufsize;
	int errnum;

	chi_copy_pos(&position, start);
	for (int i = 0; i < line->num_moves; ++i) {
		chi_move move = line->moves[i];
		errnum = chi_print_move(&position, line->moves[i], &buf, &bufsize, 0);
		chi_apply_move(&position, move);
		fprintf(stream, " %s", buf);
	}

	free(buf);
	fprintf(stream, "<\n");
}

static void
debug_start_search(Tree *tree, chi_move move)
{
	fprintf(stderr, "considering");
	print_line(stderr, &lisco.position, &tree->line);
}

static void
debug_end_search(Tree *tree, chi_move move)
{
	fprintf(stderr, "done considering");
	print_line(stderr, &lisco.position, &tree->line);
}
#endif

static int
evaluate(Tree *tree)
{
	const chi_pos *position = &tree->position;
	int score = 100 * chi_material(position);

	++tree->evals;
	
	return score;
}

static int
minimax(Tree *tree, int depth)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *move_ptr;
	chi_pos *position = &tree->position;
	int bestvalue, value;
	chi_result result;

	++tree->nodes;

	if (chi_game_over(position, &result)) {
		if (chi_result_is_white_win(result) || chi_result_is_black_win(result)) {
			return MATE + (tree->depth - depth);
		} else {
			return 0;
		}
	}

	if (depth == 0) {
		if (chi_on_move(position) == chi_white) {
			return evaluate(tree);
		} else {
			return -evaluate(tree);
		}
	}

	move_ptr = chi_legal_moves(position, moves);

	bestvalue = -INF;

	++tree->line.num_moves;
	while (move_ptr-- > moves) {
		chi_move move = *move_ptr;

		tree->line.moves[tree->line.num_moves - 1] = move;

#if DEBUG_SEARCH
		debug_start_search(tree, move);
#endif

		chi_apply_move(position, move);
		
		value = -minimax(tree, depth - 1);

		chi_unapply_move(position, move);

#if DEBUG_SEARCH
		debug_end_search(tree, move);
		fprintf(stderr, "\tvalue: %d (best: %d)\n", value, bestvalue);
#endif

		if (value > bestvalue) {
			bestvalue = value;
#if DEBUG_SEARCH
				fprintf(stderr, "\tNew best move with best value %d.\n", bestvalue);
#endif
			if (depth == tree->depth) {
#if DEBUG_SEARCH
				fprintf(stderr, "\tNew best root move with best value %d.\n", bestvalue);
#endif
				tree->bestmove = move;
			}
		}
	}
	--tree->line.num_moves;

	return bestvalue;
}


static int
root_search(Tree *tree, int max_depth)
{
	int depth, score;
	chi_bool forced_mate;

	// Iterative deepening.
	for (depth = 1; depth <= max_depth; ++depth) {
#if DEBUG_SEARCH
		fprintf(stderr, "Deepening search to maximum %d plies.\n", depth);
#endif
		tree->depth = depth;
		score = minimax(tree, depth);
		
		forced_mate = score == -MATE -depth;

		if (chi_on_move(&lisco.position) == chi_black)
			score = -score;

		lisco.bestmove = tree->bestmove;
		lisco.bestmove_found = 1;
		lisco.pondermove_found = 0;
		if (forced_mate) {
			break;
		}
	}

	return score;
}

void
think(void)
{
	chi_color_t on_move;
	Tree tree;
	int score;

	if (chi_game_over(&lisco.position, NULL)) return;

	memset(&tree, 0, sizeof tree);

	chi_copy_pos(&tree.position, &lisco.position);

	on_move = chi_on_move(&tree.position);

	score = root_search(&tree, 6);

	// Only print that to the real output channel.
	//fprintf(stderr, "score: %d\n", score);
	//fprintf(stderr, "info nodes searched: %lu\n", tree.nodes);
	//fprintf(stderr, "info nodes evaluated: %lu\n", tree.evals);
}
