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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <error.h>

#include <libchi.h>

#include "brain.h"
#include "tate.h"
#include "time_ctrl.h"
#include "board.h"

bitv64 total_nodes = 0;
bitv64 total_centiseconds = 0;
bitv64 nps_peak = 0;

void
evaluate_move (chi_move move)
{
	fprintf (stdout, "  todo\n");
}

int
think (mv, epd)
     chi_move* mv;
     chi_epd_pos* epd;
{
	chi_move moves[CHI_MAX_MOVES];
	chi_move* move_ptr;

	int num_moves;

	move_ptr = chi_legal_moves(&current, moves);

	num_moves = move_ptr - moves;

#if 0
	current_score = chi_material (&current) * 100;
	if (chi_on_move (&current) != chi_white)
	current_score = - current_score;
#endif
	fprintf (stdout, "Current score is: %d\n", current_score);
    if (num_moves == 0) {
        if (chi_check_check (&current)) {
            if (chi_on_move (&current) == chi_white) {
                fprintf (stdout, "0-1 {Black mates}\n");
            } else {
                fprintf (stdout, "1-0 {White mates}\n");
            }
        } else {
            fprintf (stdout, "1/2-1/2 {Stalemate}\n");
        }

        return EVENT_GAME_OVER;
    }

	if (current.half_move_clock >= 100) {
		fprintf (stdout, "1/2-1/2 {Fifty-move rule}\n");
		return EVENT_GAME_OVER;
	}

	*mv = moves[0];

	/* Better than nothing ...  */
	if (num_moves == 1) {
		return EVENT_CONTINUE;
	}


#if DEBUG_BRAIN
	max_ply = DEBUG_BRAIN;
	tree.time_for_move = 999999;
#endif

    // TODO: Find a move.

	return EVENT_CONTINUE;
}

#if DEBUG_BRAIN
void
indent_output (TREE* tree, int ply)
{
	int i;

//    int ply = tree->current_depth - depth;

//    for (i = depth; i < tree->current_depth; ++i)
//        fprintf (stderr, " ");

	for (i = 0; i < ply; ++i)
		fputc (' ', stderr);

	/* Assumed to be called *after* a move has been applied.  */
	if (chi_on_move(&tree->pos) != chi_white)
		fprintf(stderr, " [%s(%d)]: ", 
		        ply < tree->iteration_sdepth ? "BLACK" : "black", ply);
	else
		fprintf(stderr, " [%s(%d)]: ", 
		        ply < tree->iteration_sdepth ? "WHITE" : "white", ply);
}
#endif

void
my_print_move (chi_move mv)
{
	switch (chi_move_attacker (mv)) {
	case knight:
		fputc ('N', stderr);
		break;
	case bishop:
		fputc ('B', stderr);
		break;
	case rook:
		fputc ('R', stderr);
		break;
	case queen:
		fputc ('Q', stderr);
		break;
	case king:
		fputc ('K', stderr);
		break;
	}

    fprintf(stderr, "%s%c%s", 
	        chi_shift2label (chi_move_from (mv)),
	        chi_move_victim (mv) ? 'x' : '-',
	        chi_shift2label (chi_move_to (mv)));
	switch (chi_move_promote (mv)) {
	case knight:
		fprintf (stderr, "=N");
		break;
	case bishop:
	    fprintf (stderr, "=B");
	    break;
	case rook:
		fprintf (stderr, "=R");
		break;
	case queen:
		fprintf (stderr, "=Q");
		break;
	}
}
