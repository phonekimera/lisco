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

#ifndef _TATEPLAY_TIME_CONTROL_H
# define _TATEPLAY_TIME_CONTROL_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <time.h>

#include "libchi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TimeControl {
	chi_bool fixed_time;
	unsigned long seconds_per_move;

	long moves_per_time_control;
	long seconds_per_time_control;
	long increment;

	size_t num_moves;

	/* When did the engine start thinking? */
	struct timeval started_thinking;

	/* Total time spent thinking. */
	struct timeval thinking_time;

	/* When will the flag fall? */
	struct timeval flag;
} TimeControl;

extern chi_bool time_control_init_st(TimeControl *tc, const char *input);
extern chi_bool time_control_init_level(TimeControl *tc, const char *input);
extern void time_control_start_thinking(TimeControl *tc,
                                        const struct timeval *now);
extern chi_bool time_control_stop_thinking(TimeControl *tc,
                                           const struct timeval *now);

#ifdef __cplusplus
}
#endif

#endif
