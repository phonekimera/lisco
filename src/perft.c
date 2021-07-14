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
#include <time.h>

#include "state.h"
#include "perft.h"
#include "rtime.h"

static unsigned long do_perft(chi_pos *pos, unsigned int depth);

int
perft(unsigned int depth, FILE *out)
{
	rtime_t start;
	unsigned long int elapsed;
	unsigned long int nodes;
	chi_pos position;

	if (depth == 0) {
		fprintf(out, "info error: zero-depth argument to perft.\n");
		return 1;
	}

	chi_copy_pos(&position, &tate.position);

	start = rtime();
	nodes = do_perft(&position, depth);
	elapsed = rdifftime (rtime (), start);

	fprintf (stdout, "info nodes: %lu (%lu.%02lu s, nps: %lu)\n",
			 nodes, elapsed / 100, elapsed % 100,
			 (100 * nodes) / (elapsed ? elapsed : 1));

	return 1;
}

static unsigned long
do_perft(chi_pos *pos, unsigned int depth)
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move *mv;
	chi_move *move_end = chi_legal_moves (pos, moves);
	unsigned long nodes = 0;

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