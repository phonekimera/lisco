/* brain.c - find the correct move... ;-)
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

#include <stdlib.h>

#include <libchi.h>

int brain_interrupt = 0;

int
find_the_move (pos, mv)
     chi_pos* pos;
     chi_move* mv;
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* move_ptr;
    int num_moves;
    int pick = 0;
    // int max_sort = 0;

    brain_interrupt = 0;

    move_ptr = chi_generate_non_captures (pos, moves);
    move_ptr = chi_generate_non_captures (pos, move_ptr);

    num_moves = move_ptr - moves;

    if (num_moves == 0) {
	fprintf (stdout, "1/2-1/2 {FIXME}\n");
	return -1;
    }

    pick = random () % num_moves;

    *mv = moves[pick];

    return 0;
}
