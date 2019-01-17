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

#include <string.h>

#include "xmalloca.h"

#include "tateplay-game.h"

Game *
game_new()
{
	Game *self = xmalloc(sizeof *self);

	memset(self, 0, sizeof *self);

	self->white = engine_new();
	self->black = engine_new();

	return self;
}

void
game_destroy(Game *self)
{
	if (self == NULL) return;

	if (self->white) engine_destroy(self->white);
	if (self->black) engine_destroy(self->black);
	if (self->moves) free(self->moves);

	free(self);
}
