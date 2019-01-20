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

#ifndef _STRINGBUF_H
# define _STRINGBUF_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>

#include "libchi.h"

CHI_BEGIN_DECLS

typedef struct {
	char *str;
	size_t size;
	size_t length;
	size_t chunk_size;
} chi_stringbuf;

extern chi_stringbuf *_chi_stringbuf_new(size_t initial_size);
extern void _chi_stringbuf_destroy(chi_stringbuf *stringbuf);

/* Add a string.  */
extern void _chi_stringbuf_append(chi_stringbuf *stringbuf, const char *string);

/* Add a single character.  */
extern void _chi_stringbuf_append_char(chi_stringbuf *stringbuf, char c);

/* Add a number represented with base BASE (2 <= base <= 36).  */
extern void _chi_stringbuf_append_unsigned(chi_stringbuf *stringbuf,
                                           bitv64 number, int base);

/* Get the string.  Attention! The pointer will become invalid after
 * _chi_stringbuf_destroy() was called.
 */
extern const char *_chi_stringbuf_get_string(const chi_stringbuf *stringbuf);

CHI_END_DECLS

#endif
