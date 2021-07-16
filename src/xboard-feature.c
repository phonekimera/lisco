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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloca-debug.h"
#include "xstrndup.h"

#include "util.h"
#include "xboard-feature.h"

XboardFeature *
xboard_feature_new(const char *input, const char **endptr)
{
	XboardFeature *self = xmalloc(sizeof *self);
	const char *start = ltrim(input);
	const char *end = start;

	memset(self, 0, sizeof *self);

	while (*end && !isspace(*end) && *end != '=')
		++end;
	if (start == end) goto bail_out;

	self->name = xstrndup(start, end - start);

	while (isspace(*end))
		++end;
	if (*end != '=')
		goto bail_out;
	++end;
	while (isspace(*end))
		++end;
	if (*end == '\0')
		goto bail_out;

	start = end;
	if (*end == '"') {
		/* There is no documented escape mechanism, so read everything up to
		 * a closing quote.
		 */
		++end;
		start = end;
		while (*end && *end != '"')
			++end;
		if (!*end)
			goto bail_out;
		self->value = xstrndup(start, end - start);
		++end;
	} else {
		while (*end && !isspace(*end))
			++end;
		if (start == end) {
			self->value = xstrdup("");
		} else {
			self->value = xstrndup(start, end - start);
		}
	}

	if (endptr)
		*endptr = end;

	return self;

bail_out:
	if (endptr)
		*endptr = end;

	xboard_feature_destroy(self);

	return NULL;
}

void
xboard_feature_destroy(XboardFeature *self)
{
	if (!self) return;

	if (self->name) free(self->name);
	if (self->value) free(self->value);

	free(self);
}
