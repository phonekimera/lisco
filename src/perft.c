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
#include <time.h>

#include <libchi.h>

#include "state.h"
#include "perft.h"
#include "rtime.h"

static unsigned long long do_perft(chi_pos *pos, unsigned int depth);

unsigned long long
perft(chi_pos *position, unsigned int depth, unsigned long long *counts,
		FILE *out)
{
	rtime_t start;
	unsigned long int elapsed;
	unsigned long long nodes = 0;
	chi_move moves[CHI_MAX_MOVES];
	chi_move *move_end = chi_legal_moves(position, moves);

	if (depth == 0) {
		fprintf(out, "info error: zero-depth argument to perft.\n");
		return 1;
	}

	start = rtime();

	for (chi_move *mv = moves; mv < move_end; ++mv) {
		if (out) {
			char *buf = NULL;
			unsigned int bufsize;

			chi_coordinate_notation(*mv, chi_on_move(position), &buf, &bufsize);
			fprintf(out, "%s: ", buf);
			free(buf);
			fflush(out);
		}

		chi_apply_move(position, *mv);

		unsigned long long nodes_here;
		if (depth > 1)
			nodes_here = do_perft(position, depth - 1);
		else
			nodes_here = 1;

		if (counts)
			*counts++ = nodes_here;

		nodes += nodes_here;

		if (out) {
			fprintf(out, "%llu\n", nodes_here);
		}

		chi_unapply_move(position, *mv);
	}

	elapsed = rdifftime (rtime (), start);

	if (out) {
		fprintf (out, "info nodes: %llu (%lu.%02lu s, nps: %llu)\n",
				nodes, elapsed / 100, elapsed % 100,
				(100 * nodes) / (elapsed ? elapsed : 1));
	}

	return nodes;
}

static unsigned long long
do_perft(chi_pos *pos, unsigned int depth)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *mv;
	chi_move *move_end = chi_legal_moves (pos, moves);
	unsigned long long nodes = 0;

	for (mv = moves; mv < move_end; ++mv) {
		chi_apply_move(pos, *mv);
		if (depth > 1)
			nodes += do_perft(pos, depth - 1);
		else
			++nodes;
		chi_unapply_move(pos, *mv);
	}

	return nodes;
}