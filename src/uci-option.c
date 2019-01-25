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
uci_optoin_new(const char *input)
{
	UCIOption *self = xmalloc(sizeof *self);
	const char *start = ltrim(input);
	const char *end = start;

	memset(self, 0, sizeof *self);

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
