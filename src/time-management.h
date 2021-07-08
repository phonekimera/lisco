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

#ifndef TIME_MANAGEMENT_H
# define TIME_MANAGEMENT_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

/* Stolen from Stockfish.  */

typedef bitv64 TimePoint;

typedef struct TimeManagement {
	TimePoint start_time;
	TimePoint optimum_time;
	TimePoint maximum_time;

	bitv64 available_nodes;
} TimeManagement;

typedef struct TimeLimits {
	TimePoint time[2];
	TimePoint inc[2];
	TimePoint npmsec;
	TimePoint start_time;
	bitv64 movetime;

	int movestogo;
	int depth;
	int mate;
	int perft;
	int infinite;

	bitv64 nodes;
} TimeLimits;

extern TimeManagement timer;

extern void time_limits_init(TimeLimits *self);
extern void time_management_init(TimeManagement *self, struct TimeLimits *limits,
                                 chi_color_t us, int ply);
extern TimePoint now();

#endif
