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

#include <xalloc.h>

#include "time-management.h"
#include "tate.h"
#include "time-control.h"

/* Stolen from stockfish and ported to C.  */

void
time_limits_init(TimeLimits *self)
{
	memset(self, 0, sizeof *self);

	self->start_time = now();
	if (engine_color == chi_white) {
		self->time[chi_white] = time_left * 10;
		self->time[chi_black] = opp_time * 10;
	} else {
		self->time[chi_white] = opp_time * 10;
		self->time[chi_black] = time_left * 10;
	}
	self->inc[chi_white] = self->inc[chi_black] = inc;

	if (fixed_time) {
		self->movetime = 1000 * fixed_time;
	} else {
		self->movetime = 0;
	}

	/* FIXME! This is normally MAX_PLY.  */
	self->depth = max_depth;
}
