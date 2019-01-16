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

#include <libchi.h>

int
chi_check_legality (pos, usermove)
     chi_pos* pos;
     chi_move usermove;
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* move_ptr;
    chi_move* m;

    if (!pos)
	return CHI_ERR_YOUR_FAULT;

    move_ptr = chi_legal_moves (pos, moves);

    for (m = moves; m < move_ptr; ++m) {
	if ((chi_move_packed (*m) & 0xfff) == 
	    (chi_move_packed (usermove) & 0xfff))
	    return 0;
    }

    return CHI_ERR_ILLEGAL_MOVE;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
