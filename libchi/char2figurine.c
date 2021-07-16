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

#include <string.h>

#include <libchi.h>

#if CHI_USE_UTF_8
const char *
chi_char2figurine(char c)
{
	switch (c) {
		case 'K':
			return "♔";
		case 'Q':
			return "♕";
		case 'R':
			return "♖";
		case 'B':
			return "♗";
		case 'N':
			return "♘";
		case 'P':
			return "♙";
		case 'k':
			return "♚";
		case 'q':
			return "♛";
		case 'r':
			return "♜";
		case 'b':
			return "♝";
		case 'n':
			return "♞";
		case 'p':
			return "♟";
	}

	return " ";
}
#endif