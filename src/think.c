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

#include "lisco.h"
#include "rtime.h"

#define DEBUG_SEARCH 0
#define DEBUG_TIME_CONTROL 1

static void update_tree(Tree *tree, int ply, chi_pos *position, chi_move move);

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

static void
print_pv(Tree *tree, int depth)
{
	FILE *out = lisco.uci.out;
	long elapsed = rdifftime(rtime(), tree->start_time);
	long nps = elapsed ? 1000 * tree->nodes / elapsed : tree->nodes;
	char *buf = NULL;
	unsigned int bufsize;

	fprintf(out, "info depth %d multipv 1 score cp %d nodes %llu nps %ld"
			" tbhits %llu time %ld pv",
			tree->depth, tree->score, tree->nodes, nps, tree->tt_hits, elapsed);

	chi_pos position;
	chi_copy_pos(&position, &tree->position);
	Line *line = &tree->line;
	for (int i = 0; i < line->num_moves; ++i) {
		chi_move move = line->moves[i];
		chi_coordinate_notation(line->moves[i], chi_on_move(&position), &buf, &bufsize);
		chi_apply_move(&position, move);
		fprintf(out, " %s", buf);
	}
	free(buf);

	fprintf(out, "\n");
}

static void
time_control(Tree *tree)
{
	rtime_t now = rtime();
	long elapsed = rdifftime(now, tree->start_time);
	unsigned long long nps = 1000 * (tree->nodes / elapsed);
	tree->nodes_to_tc = nps / 1000;
#if DEBUG_TIME_CONTROL
	fprintf(stderr, "elapsed: %ld ms, nodes: %llu, nps: %lld, nodes to next tc: %lld.\n",
		elapsed, tree->nodes, nps, tree->nodes_to_tc);
#endif
	if (elapsed > 1000 * tree->fixed_time - 200) {
		tree->move_now = 1;
#if DEBUG_TIME_CONTROL
		fprintf(stderr, "Time's up, move now!\n");
#endif
	}
}

static int
alphabeta(Tree *tree, int depth, int alpha, int beta)
{
	chi_pos *position = &tree->position;
	int value;
	chi_result result;
	int ply = tree->depth - depth;

	++tree->nodes;
	if (--tree->nodes_to_tc <= 0) {
		time_control(tree);
	}

	if (chi_game_over(position, &result)) {
		if (chi_result_is_white_win(result) || chi_result_is_black_win(result)) {
			return MATE + (tree->depth - depth);
		} else {
			return 0;
		}
	}

	if (depth == 0) {
		return quiesce(tree, ply, alpha, beta);
	}

	/*
	int alpha, beta;
	++tree->tt_probes;
	unsigned int tt_hit = probe_tt (position, tree->signatures[ply], depth,
			&alpha, &beta);
	if (tt_hit != HASH_UNKNOWN) {
		++tree->tt_hits;
#if DEBUG_SEARCH
		fprintf(stderr, "\ttable hit type %u.\n", tt_hit);
#endif
		switch (tt_hit) {
			case HASH_EXACT:
				return alpha;

			case HASH_BETA:
				return beta;

			case HASH_ALPHA:
				return alpha;
		}
	}
	*/

	MoveSelector selector;
	move_selector_init(&selector, tree,
		tree->depth == depth && tree->bestmove ? tree->bestmove : 0);

	++tree->line.num_moves;
	chi_move move;
	while ((move = move_selector_next(&selector))) {
		if (tree->move_now) {
			return alpha;
		}

		tree->line.moves[tree->line.num_moves - 1] = move;

#if DEBUG_SEARCH
		debug_start_search(tree, move);
#endif

		chi_apply_move(position, move);
		update_tree(tree, ply, position, move);

		value = -alphabeta(tree, depth - 1, -beta, -alpha);

		/*
		store_tt_entry(position, tree->signatures[ply + 1], move, depth, value,
				HASH_EXACT);
		*/

		chi_unapply_move(position, move);

#if DEBUG_SEARCH
		debug_end_search(tree, move);
		fprintf(stderr, "\tvalue: %d (best: %d)\n", value, alpha);
#endif

		if (value >= beta) {
			/* Fail high.  */
#if DEBUG_SEARCH
			fprintf(stderr, "\tfail high: value(%d) >= beta(%d)\n", value, beta);
#endif
			--tree->line.num_moves;
			return beta;
		}

		if (value > alpha) {
			alpha = value;
#if DEBUG_SEARCH
			fprintf(stderr, "\tNew best move with best value %d.\n", alpha);
#endif
			if (depth == tree->depth) {
#if DEBUG_SEARCH
				fprintf(stderr, "\tNew best root move with best value %d.\n", alpha);
#endif
				tree->bestmove = move;
				tree->score =
					chi_on_move(position) == chi_white ? value : -value;
				print_pv(tree, depth);
			}
		}
	}

	--tree->line.num_moves;

	return alpha;
}

static int
root_search(Tree *tree, int max_depth)
{
	int depth, score;
	chi_bool forced_mate;

	tree->start_time = rtime();
	tree->move_now = 0;
	tree->fixed_time = 120;
	tree->score = 0;

	// Initially assume 1,000 nodes per second.  That give us 100 nodes
	// to estimate the timing.
	tree->nodes_to_tc = 100;

	// Iterative deepening.
	for (depth = 1; depth <= max_depth; ++depth) {
#if DEBUG_SEARCH
		fprintf(stderr, "Deepening search to maximum %d plies.\n", depth);
#endif
		tree->depth = depth;
		score = alphabeta(tree, depth, -INF, +INF);
		
		forced_mate = score == -MATE -depth;

		if (chi_on_move(&lisco.position) == chi_black)
			score = -score;

		lisco.bestmove = tree->bestmove;
		lisco.bestmove_found = 1;
		lisco.pondermove_found = 0;
		if (forced_mate) {
			break;
		}

		if (tree->move_now)
			break;
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

	tree.signatures[0] = chi_zk_signature(lisco.zk_handle, &tree.position);

	score = root_search(&tree, 110);

	// Only print that to the real output channel.
	//fprintf(stderr, "score: %d\n", score);
	//fprintf(stderr, "info nodes searched: %lu\n", tree.nodes);
	//fprintf(stderr, "info nodes evaluate_locald: %lu\n", tree.evals);
}

static void
update_tree(Tree *tree, int ply, chi_pos *position, chi_move move)
{
	tree->signatures[ply + 1] = chi_zk_update_signature(lisco.zk_handle,
		tree->signatures[ply], move, chi_on_move(position));
}