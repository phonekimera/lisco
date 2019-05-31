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

#include "search.h"
#include "time_ctrl.h"

int
search(TREE *tree, int ply, int alpha, int beta)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *end = chi_legal_moves(&tree->pos, moves);
	size_t num_moves = end - moves;
	size_t i;
	int best_score;

	++tree->nodes;

	/* Check for time control and user input.  */
	--next_time_control;
	if (next_time_control < 0) {
		if (event_pending) {
			int result = get_event ();
			if (result & EVENTMASK_ENGINE_STOP) {
				tree->status = result;
			}

			return beta;
		}

		if (rdifftime (rtime (), start_time) >= tree->time_for_move) {
			tree->status = EVENT_MOVE_NOW;
			return beta;
		}
		next_time_control = MOVES_PER_TIME_CONTROL;
	}

	if (ply >= max_ply || num_moves == 0) {
		int score = evaluate(tree, ply, alpha, beta);
#if DEBUG_BRAIN
		indent_output(tree, ply);
		fprintf(stderr, "--> evaluation: %d\n", score);
#endif
		return score;
	}

	best_score = -INF;
	for (i = 0; i < num_moves; ++i) {
		chi_move *move = &moves[i];
		chi_apply_move(&tree->pos, *move);
		tree->in_check[ply] = chi_check_check (&tree->pos);
		tree->signatures[ply + 1] = chi_zk_update_signature(
			zk_handle, tree->signatures[ply], *move,
			chi_on_move(&tree->pos));
#if DEBUG_BRAIN
		indent_output(tree, ply + 1);
		my_print_move(*move);
		fprintf(stderr, "\n");
		fflush(stderr);
#endif
		int node_score = -search(tree, ply + 1, -beta, -alpha);
		chi_unapply_move(&tree->pos, *move);
		if (node_score > best_score) {
			best_score = node_score;
		}
		if (node_score > alpha) {
			alpha = node_score;
		}
		if (alpha >= beta || tree->status & EVENTMASK_ENGINE_STOP) {
			break;
		}
	}

	return best_score;
}
