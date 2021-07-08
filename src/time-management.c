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

#if defined(HAVE_GETTIMEOFDAY)
# include <sys/time.h>
#else
# if defined(HAVE_FTIME) && defined(HAVE_SYS_TIMEB_H)
#  include <sys/timeb.h>
# else
#  include <time.h>
# endif
#endif

#include <math.h>
#include <float.h>

#include <libchi.h>

#include "tate.h"
#include "time-management.h"

/* Stolen from stockfish and ported to C.  */

#define MOVE_HORIZON 50
#define MAX_RATIO 7.3
#define STEAL_RATIO 0.34

TimeManagement timer;

static double move_importance(int ply);
static TimePoint remaining(TimePoint my_time, int moves_to_go, int ply,
                           TimePoint slow_mover, chi_bool optimum);

void
time_management_init(TimeManagement *self, TimeLimits *limits, chi_color_t us,
                     int ply)
{
	TimePoint hyp_my_time;
	TimePoint t1, t2;
	int hyp_mtg, used_mtg;
	int max_mtg;

	if (npmsec) {
		if (!self->available_nodes)
			self->available_nodes = npmsec * limits->time[us];
		
		limits->time[us] = (TimePoint) self->available_nodes;
		limits->inc[us] *= npmsec;
		limits->npmsec = npmsec;
	}

	self->start_time = limits->start_time;
	self->optimum_time = self->maximum_time
			= limits->time[us] > min_thinking_time
			? limits->time[us] : min_thinking_time;
	
	if (limits->movestogo) {
		max_mtg = limits->movestogo < MOVE_HORIZON
				? limits->movestogo : MOVE_HORIZON;
	} else {
		max_mtg = MOVE_HORIZON;		
	}

	for (hyp_mtg = 1; hyp_mtg <= max_mtg; ++hyp_mtg) {
		used_mtg = hyp_mtg < 40 ? hyp_mtg : 40;
		hyp_my_time = limits->time[us]
                      + limits->inc[us] * (hyp_mtg - 1)
                      - move_overhead * (2 + used_mtg);

		if (hyp_my_time < 0) hyp_my_time = 0;

		t1 = min_thinking_time
		     + remaining(hyp_my_time, hyp_mtg, ply, slow_mover, chi_true);
		t2 = min_thinking_time
		     + remaining(hyp_my_time, hyp_mtg, ply, slow_mover, chi_false);

		if (t1 < self->optimum_time) self->optimum_time = t1;
		if (t2 < self->maximum_time) self->maximum_time = t2;
	}

	if (pondering)
		self->optimum_time += self->optimum_time >> 2;
}

static TimePoint
remaining(TimePoint my_time, int moves_to_go, int ply,
          TimePoint slow_mover, chi_bool optimum)
{
	double tmax_ratio = optimum ? 1.0 : MAX_RATIO;
	double tsteal_ratio = optimum ? 0.0 : STEAL_RATIO;
	double importance = (move_importance(ply) * slow_mover) / 100.0;
	double others_importance = 0.0;
	double ratio1, ratio2;

	for (int i = 1; i < moves_to_go; ++i) {
		others_importance  += move_importance(ply + 2 * i);
	}

	ratio1 = (tmax_ratio * importance)
			/ (tmax_ratio * importance + others_importance);
	ratio2 = (importance + tsteal_ratio * others_importance)
			/ (importance + others_importance);

	return ratio1 < ratio2 ? ratio1 : ratio2;	
}

static double
move_importance(int ply)
{
	double xscale = 6.85;
	double xshift = 64.5;
	double skew = 0.171;

    return pow((1 + exp((ply - xshift) / xscale)), -skew) + DBL_MIN;
}

TimePoint
now () 
{
#if defined(HAVE_GETTIMEOFDAY)
    struct timeval tv;

    gettimeofday (&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
# if defined(HAVE_FTIME) && defined(HAVE_SYS_TIMEB_H)
	struct timeb tp;
	ftime (&tp);

	return (TimePoint) (1000 * tp.time + tp.millitm);
# else
	return (time (0)) * 1000;
# endif
#endif
}
