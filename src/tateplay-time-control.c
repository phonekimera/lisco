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

#include "error.h"

#include "libchi.h"
#include "tateplay-time-control.h"
#include "util.h"
#include "xmalloca-debug.h"

static char delimiter[256];
static chi_bool delimiter_initialized = chi_false;

chi_bool
time_control_init_level(TimeControl *self, const char *_input)
{
	unsigned char c;
	size_t i;
	char *input;
	char *endptr;
	char *moves_per_time_control_str;
	char *minutes_per_time_control_str;
	char *seconds_str;
	long minutes, seconds = 0L;
	char *increment_str;

	/* FIXME! Be less permissive here! We may need more characters, especially
	 * the plus sign.
	 */
	if (!delimiter_initialized) {
		for (i = 0, c = '\001'; c < '0' && i < sizeof delimiter; ++c) {
			delimiter[i++] = c;
		}
		for (c = ';'; c > 0 && i < sizeof delimiter; ++c) {
			delimiter[i++] = c;
		}
		delimiter[i++] = '+';
		delimiter[i] = '\0';
		delimiter_initialized = chi_true;
	}

	memset(self, 0, sizeof *self);

	input = xstrdup(_input);

	endptr = trim(input);

	/* First tokenize the three parts.  */
	moves_per_time_control_str = strsep(&endptr, delimiter);
	if (!(moves_per_time_control_str && *moves_per_time_control_str)) {
		free(input);
		return chi_false;
	}
	minutes_per_time_control_str = strsep(&endptr, delimiter);
	if (!(minutes_per_time_control_str && *minutes_per_time_control_str)) {
		free(input);
		return chi_false;
	}
	increment_str = strsep(&endptr, delimiter);
	if (!(increment_str && *increment_str)) {
		free(input);
		return chi_false;
	}
	if (endptr) {
		free(input);
		return chi_false;
	}

	/* We never have to take negative numbers into account.  The minus sign
	 * is part of our delimiter and will be discarded.
	 */

	if (!parse_integer(&self->moves_per_time_control,
	                   moves_per_time_control_str)) {
		free(input);
		return chi_false;
	}

	/* Possibly split the minutes.  */
	endptr = minutes_per_time_control_str;
	minutes_per_time_control_str = strsep(&endptr, ":");
	if (!(minutes_per_time_control_str && *minutes_per_time_control_str)) {
		/* Cannot happen.  */
		free(input);
		return chi_false;
	}
	seconds_str = strsep(&endptr, ":");
	if (seconds_str && endptr && *endptr) {
		free(input);
		return chi_false;
	}
	if (!parse_integer(&minutes, minutes_per_time_control_str)) {
		free(input);
		return chi_false;
	}
	if (seconds_str && !parse_integer(&seconds, seconds_str)) {
		free(input);
		return chi_false;
	}
	self->seconds_per_time_control = 60 * minutes + seconds;

	if (!parse_integer(&self->increment, increment_str)) {
		free(input);
		return chi_false;
	}

	free(input);

	return chi_true;
}

chi_bool
time_control_init_st(TimeControl *self, const char *_input)
{
	long seconds_per_move;
	char *buf;
	char *input;

	memset(self, 0, sizeof *self);

	buf = xstrdup(_input);
	input = trim(buf);

	if (!parse_integer(&seconds_per_move, input)) {
		free(buf);
		return chi_false;
	}
	free(buf);
	if (seconds_per_move <= 0)
		return chi_false;

	self->seconds_per_move = seconds_per_move;
	self->fixed_time = chi_true;

	return chi_true;
}

extern void
time_control_start_thinking(TimeControl *self, const struct timeval *now)
{
	self->started_thinking.tv_sec = now->tv_sec;
	self->started_thinking.tv_usec = now->tv_usec;

	if (self->fixed_time) {
		self->flag.tv_sec = self->started_thinking.tv_sec
				+ self->seconds_per_move;
		self->flag.tv_usec = self->started_thinking.tv_usec;
	} else {
		// TODO!
	}
}
