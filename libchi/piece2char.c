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

#include <libchi.h>

const char
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
