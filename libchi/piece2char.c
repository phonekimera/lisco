/* piece2char.c - Return ASCII representation for a chess piece.
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

char
chi_piece2char (piece)
     chi_piece_t piece;
{
    switch (piece) {
	case empty:
	    return ' ';
	case pawn: 
	    return 'P';
	case knight:
	    return 'N';
	case bishop:
	    return 'B';
	case rook:
	    return 'R';
	case queen:
	    return 'Q';
	case king:
	    return 'K';
    }
    return '?';
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
