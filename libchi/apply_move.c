/* apply_move.c - Apply a move to a position.
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
chi_apply_move (pos, move)
     chi_pos* pos;
     chi_move move;
{
    int result = chi_make_move (pos, move);

    if (result)
	return result;

    if (chi_on_move (pos) == chi_white)
	pos->material += chi_move_material (move);
    else
	pos->material -= chi_move_material (move);

    chi_on_move (pos) = !chi_on_move (pos);

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
