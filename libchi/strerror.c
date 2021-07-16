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

static const char* error_strings[] = {
    "no error",
    "virtual memory exhausted",
    "parser error",
    "move from an empty square",
    "no matching move",
    "side not on move is in check",
    "illegal fen position",
    "one side has two or more kings",
    "one side has no kings",
    "illegal castling state",
    "cannot parse EPD string",
    "illegal library usage",
	"ambiguous move"
};

const char*
chi_strerror (errnum)
     int errnum;
{
    if (errnum < CHI_ERR_SUCCESS || errnum > CHI_ERRMAX)
	return "Unknown error";
    else
	return error_strings[errnum];
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
