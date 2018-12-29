/* legal_move.c - Check one single move for legality.
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
chi_illegal_move (pos, move)
     chi_pos* pos;
     chi_move move;
{
    chi_pos tmp_pos;
    int errnum;

    if (!pos)
	return CHI_ERR_YOUR_FAULT;

    tmp_pos = *pos;

    if (chi_move_from (move) == 3) {
	bitv64 from_mask = ((bitv64) 1) << 3;
	int to = chi_move_to (move);
	
	if ((to == 5 || to == 1) && from_mask & pos->w_kings) {
	    int tmp_to = (3 + to) >> 1;
	    
	    if (chi_check_check (pos))
		return 1;
	    
	    chi_move_set_to (move, tmp_to);
	    chi_make_move (&tmp_pos, move);
	    if (chi_check_check (&tmp_pos))
		return 1;
	    tmp_pos = *pos;
	    chi_move_set_to (move, to);
	}
    } else if (chi_move_from (move) == 59) {
	bitv64 from_mask = ((bitv64) 1) << 59;
	int to = chi_move_to (move);
	
	if ((to == 61 || to == 57) && from_mask & pos->b_kings) {
	    int tmp_to = (3 + to) >> 1;
	    
	    if (chi_check_check (pos))
		return 1;
	    
	    chi_move_set_to (move, tmp_to);
	    chi_make_move (&tmp_pos, move);
	    if (chi_check_check (&tmp_pos))
		return 1;
	    tmp_pos = *pos;
	    chi_move_set_to (move, to);
	}
    }
    
    if (0 != (errnum = chi_make_move (&tmp_pos, move)))
	return errnum;
    
    if (chi_check_check (&tmp_pos))
	return CHI_ERR_IN_CHECK;
    
    *pos = tmp_pos;

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
