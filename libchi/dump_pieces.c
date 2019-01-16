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

int
chi_dump_pieces (pos, buf, bufsize)
     chi_pos* pos;
     char** buf;
     unsigned int* bufsize;
{
	int i;
	char *squares;
	bitv64 empty_squares = ~(pos->w_pieces | pos->b_pieces);
	bitv64 mask = CHI_A_MASK & CHI_8_MASK;
	
	if (!buf || !bufsize || !pos)
		return CHI_ERR_YOUR_FAULT;
	
	if (!*buf || *bufsize < 65) {
		char* new_buf = realloc (*buf, 65);
		
		if (!new_buf)
			return CHI_ERR_ENOMEM;
		
		*buf = new_buf;
		*bufsize = 65;
	}
	
	squares = *buf;

	for (i = 63; i >= 0; --i, mask >>= 1) {
		if (mask & empty_squares) {
			squares[i] = ' ';
		} else if (mask & pos->w_pawns) {
			squares[i] = 'P';
		} else if (mask & pos->w_knights) {
			squares[i] = 'N';
		} else if (mask & pos->w_bishops) {
			if (mask & pos->w_rooks) 
				squares[i] = 'Q';
			else
				squares[i] = 'B';
		} else if (mask & pos->w_rooks) {
			if (mask & pos->w_bishops) 
				squares[i] = 'Q';
			else
				squares[i] = 'R';
		} else if (mask & pos->w_kings) {
			squares[i] = 'K';
		} else if (mask & pos->b_pawns) {
			squares[i] = 'p';
		} else if (mask & pos->b_knights) {
			squares[i] = 'n';
		} else if (mask & pos->b_bishops) {
			if (mask & pos->b_rooks) 
				squares[i] = 'q';
			else
				squares[i] = 'b';
		} else if (mask & pos->b_rooks) {
			if (mask & pos->b_bishops) 
				squares[i] = 'q';
			else
				squares[i] = 'r';
		} else if (mask & pos->b_kings) {
			squares[i] = 'k';
		} else {
			squares[i] = ' ';
		}
	}
	squares[64] = 0;

	return 0;
}
	
/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
