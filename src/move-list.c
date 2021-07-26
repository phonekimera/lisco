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

#include <sys/types.h>

#include <xalloc.h>

#include <libchi.h>

#include "lisco.h"

void
move_list_init(MoveList *self)
{
	self->num_moves = 0;
	self->moves = NULL;
}

void
move_list_destroy(MoveList *self)
{
	if (self->moves)
		free((void *) self->moves);
}

void
move_list_add(MoveList *self, chi_move move)
{
	self->moves = xrealloc((void *) self->moves, ++self->num_moves);
	self->moves[self->num_moves - 1] = move;
}