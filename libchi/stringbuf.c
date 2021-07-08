/* This file is part of the chess engine tate.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libchi.h"
#include "stringbuf.h"

#ifdef xmalloc
# undef xmalloc
#endif
static void *
xmalloc(size_t size)
{
	void *buf = malloc(size);
	if (!buf) {
		perror("fatal error");
		exit(255);
	}

	return buf;
}

#ifdef xreaalloc
# undef xrealloc
#endif
static void *
xrealloc(void *buf, size_t new_size)
{
	void *new_buf = realloc(buf, new_size);
	if (!new_buf) {
		perror("fatal error");
		exit(255);
	}

	return new_buf;
}

chi_stringbuf *
_chi_stringbuf_new(size_t size)
{
	chi_stringbuf *self = xmalloc(sizeof *self);

	self->size = size ? size : 80;
	self->chunk_size = (self->size >> 2) || self->size;
	self->length = 0;

	self->str = xmalloc(self->size);
	*self->str = '\0';

	return self;
}

void
_chi_stringbuf_destroy(chi_stringbuf *self)
{
	if (self == NULL) return;
	
	free(self->str);

	free(self);
}

void
_chi_stringbuf_append(chi_stringbuf *self, const char *string)
{
	size_t length = strlen(string);
	
	if (length + self->length + 1 > self->size) {
		size_t missing = self->length + length + 1 - self->size;
		size_t chunk_size = self->chunk_size || (length << 1);
		if (chunk_size < missing)
			chunk_size = missing;
		self->size += chunk_size;
		self->str = xrealloc(self->str, self->size);
	}

	strcpy(self->str + self->length, string);
	self->length += length;
}

void
_chi_stringbuf_append_char(chi_stringbuf *self, char c)
{
	if (self->length + 2 > self->size) {
		size_t chunk_size = self->chunk_size ? self->chunk_size : 2;
		self->size += chunk_size;
		self->str = xrealloc(self->str, self->size);
	}

	self->str[self->length++] = c;
	self->str[self->length] = '\0';
}

void
_chi_stringbuf_append_unsigned(chi_stringbuf *self, bitv64 number, int base)
{
	const char digits[] =
		"zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";
	char *forward;
	char *backwards;
	size_t added = 0;

	if (base < 2 || base > 36)
		return;
	
	do {
		bitv64 old = number;
		char c;

		number /= base;
		c = digits[35 + (old - number * base)];
		_chi_stringbuf_append_char(self, c);
		++added;
	} while (number);

	/* Reverse the digits.  */
	backwards = self->str + self->length - 1;
	forward = backwards - added + 1;

	while (forward < backwards) {
			int c = *forward;
			*forward = *backwards;
			*backwards = c;
			++forward; --backwards;
	}
}

const char *
_chi_stringbuf_get_string(const chi_stringbuf *self)
{
	return self->str;
}
