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
chi_coordinate_notation(chi_move move, chi_color_t on_move,
                        char **buf, unsigned int *bufsize)
{
	unsigned int from = chi_move_from(move);
	unsigned int to = chi_move_to(move);
	chi_piece_t promotion;
	const char* label;

	if (from > 63 || to > 63 || !buf || !bufsize)
		return CHI_ERR_YOUR_FAULT;

	if (!*buf || *bufsize < 6) {
		char* new_buf = realloc (*buf, 6);
	
		if (!new_buf)
			return CHI_ERR_ENOMEM;
	
		*buf = new_buf;
		*bufsize = 6;
	}

	if (chi_move_attacker(move) == king 
	    && from == chi_coords2shift(4, 0)
            && to == from - 2) {
		strcpy(*buf, "O-O");
	} else if (chi_move_attacker(move) == king 
	    && from == chi_coords2shift(4, 0)
            && to == from + 2) {
		strcpy(*buf, "O-O-O");
	} else if (chi_move_attacker(move) == king 
	    && from == chi_coords2shift(4, 7)
            && to == from - 2) {
		strcpy(*buf, "O-O");
	} else if (chi_move_attacker(move) == king 
	    && from == chi_coords2shift(4, 7)
            && to == from + 2) {
		strcpy(*buf, "O-O-O");
	} else {
		label = chi_shift2label(from);
		(*buf)[0] = label[0];
		(*buf)[1] = label[1];

		label = chi_shift2label(to);
		(*buf)[2] = label[0];
		(*buf)[3] = label[1];

		promotion = chi_move_promote(move);

		if (promotion) {
			(*buf)[4] = chi_piece2char(promotion);
			(*buf)[5] = '\0';
		} else {
			(*buf)[4] = '\0';
		}
	}

	return 0;
}
