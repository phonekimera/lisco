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

#include "engine.h"
#include "xmalloca.h"

Engine *
engine_new()
{
	Engine *self = xmalloc(sizeof (Engine));

	self->_argv_size = 1;
	self->argv = xmalloc(sizeof self->argv[0]);
	self->argv[0] = NULL;

	return self;
}

void
engine_destroy(Engine *self)
{
	if (self == NULL) return;

	if (self->argv) free(self->argv);

	free(self);
}

extern void
engine_add_argv(Engine *self, const char *arg)
{
	self->argv = xrealloc(self->argv,
	                      ++self->_argv_size * sizeof self->argv[0]);
	self->argv[self->_argv_size - 2] = xstrdup(arg);
	self->argv[self->_argv_size - 1] = NULL;
}
