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

#include <stdlib.h>

#include <libchi.h>

static unsigned long int perft_pre_check(chi_pos*, unsigned int);
static unsigned long int perft_post_check(chi_pos*, unsigned int);
unsigned long int
chi_perft (pos, depth, post_flag)
     chi_pos* pos;
     unsigned int depth;
     int post_flag;
{
    if (post_flag)
	return perft_post_check (pos, depth);
    else
	return perft_pre_check (pos, depth);
}

static unsigned long int
perft_pre_check (pos, depth)
     chi_pos* pos;
     unsigned int depth;
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* move_end;
    chi_move* mv;
    chi_pos tmp_pos;
    chi_pos* saved_pos = &tmp_pos;
    unsigned long int nodes = 0;
    
    chi_copy_pos (saved_pos, pos);
    move_end = chi_legal_moves (pos, moves);
    for (mv = moves; mv < move_end; ++mv) {
	chi_apply_move (pos, *mv);
	if (depth > 1)
	    nodes += perft_pre_check (pos, depth - 1);
	else
	    ++nodes;

	chi_copy_pos (pos, saved_pos);
    }

    return nodes;
}
     
static unsigned long int
perft_post_check (pos, depth)
     chi_pos* pos;
     unsigned int depth;
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* move_end;
    chi_move* mv;
    chi_pos tmp_pos;
    chi_pos* saved_pos = &tmp_pos;
    unsigned long int nodes = 0;

    chi_copy_pos (saved_pos, pos);
    move_end = chi_generate_captures (pos, moves);
    move_end = chi_generate_non_captures (pos, move_end);
    for (mv = moves; mv < move_end; ++mv) {
	if (chi_illegal_move (pos, *mv, 0))
	    continue;

	if (depth > 1)
	    nodes += perft_post_check (pos, depth - 1);
	else
	    ++nodes;

	chi_copy_pos (pos, saved_pos);
    }

    return nodes;
}
     
/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
