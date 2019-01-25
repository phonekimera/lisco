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

#include <ctype.h>
#include <string.h>

char *
trim(char *str)
{
	char *end_ptr;

	while (isspace((unsigned char) *str)) ++str;

	if (*str =='\0') return str;

	end_ptr = str + strlen(str) - 1;
	while (end_ptr > str && isspace((unsigned char) *end_ptr)) --end_ptr;

	end_ptr[1] = '\0';

	return str;
}

char *
ltrim(char *str)
{
	while (isspace((unsigned char) *str)) ++str;

	return str;
}
