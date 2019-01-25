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
#include <stdlib.h>
#include <string.h>

#include "xmalloca.h"
#include "xstrndup.h"

#include "util.h"
#include "uci-option.h"

UCIOption *
uci_option_new(const char *input)
{
	UCIOption *self = xmalloc(sizeof *self);
	const char *start = ltrim(input);
	const char *end = start;

	memset(self, 0, sizeof *self);

	/* We optimistically assume that the possible order of the keywords
	 * is fixed, although that this is one of the many questions that the
	 * UCI description does not answer.
	 */

	if (strncmp("name", start, 4) != 0)
		goto bail_out;
	
	start = ltrim(start + 4);
	if (start == start + 4)
		goto bail_out;
	
	end = strstr(start, "type");
	if (end == NULL)
		goto bail_out;
	if (!isspace(end[-1]))
		goto bail_out;
	/* We know that the string is already left-trimmed.  So the call
	 * to trim cannot change the pointer.
	 */
	self->name = trim(xstrndup(start, end - start - 1));

	/* Skip "type".  */
	start = ltrim(end + 4);
	if (start == end+4)
		goto bail_out;
	
	end = start;
	while (*end && !isspace(*end))
		++end;
	if (end == start)
		goto bail_out;
	
	if (strncmp(start, "check", 5) == 0)
		self->type = uci_option_type_check;
	else if (strncmp(start, "spin", 4) == 0)
		self->type = uci_option_type_spin;
	else if (strncmp(start, "combo", 5) == 0)
		self->type = uci_option_type_spin;
	else if (strncmp(start, "button", 6) == 0)
		self->type = uci_option_type_spin;
	else if (strncmp(start, "string", 6) == 0)
		self->type = uci_option_type_spin;
	else
		goto bail_out;

	return self;

bail_out:
	uci_option_destroy(self);

	return NULL;
}

void
uci_option_destroy(UCIOption *self)
{
	if (!self) return;

	if (self->name) free(self->name);

	free(self);
}
