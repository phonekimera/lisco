/* check_legality.c - Check if a move is legal.
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
