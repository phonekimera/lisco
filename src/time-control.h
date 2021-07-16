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

#ifndef TIME_CONTROL_H
# define TIME_CONTROL_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SYS_TIMEB_H
# include <sys/timeb.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if defined(HAVE_SYS_TIMEB_H) && (defined(HAVE_FTIME) || defined(HAVE_GETTIMEOFDAY)) 
typedef struct timeb rtime_t;
#else
typedef time_t rtime_t;
#endif

extern long int moves_to_tc;
extern long int min_per_game;
extern long int inc;
extern long int time_left;
extern long int sec_per_game;
extern long int opp_time;
extern long int time_cushion;

extern int go_fast;
extern int fixed_time;

extern rtime_t start_time;

/* Parse a level specification.  */
extern void parse_level(const char* level);

/* Calculate the amount of time the program can use in its search, measured
   in centi-seconds (calculate everything in float for more accuracy as
   we go, and return the result as a long int).  */
long int allocate_time(void);

extern rtime_t rtime(void);
extern long int rdifftime(rtime_t end, rtime_t start);

#endif
