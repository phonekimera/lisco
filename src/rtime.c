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

#include "rtime.h"

long int 
rdifftime(rtime_t end, rtime_t start)
{
	/* Determine the time taken between start and the current time in
		centi-seconds.  */

	/* Using ftime().  */
#if defined(HAVE_SYS_TIMEB_H) && (defined(HAVE_FTIME) || defined(HAVE_GETTIMEOFDAY))
	return ((end.time - start.time) * 100 + 
		(end.millitm-start.millitm) / 10);

	/* Using time(). */
#else
    return (100 * (long int) difftime (end, start));
#endif
}

rtime_t 
rtime() 
{
	/* Using ftime().  */
#if defined(HAVE_FTIME) && defined(HAVE_SYS_TIMEB_H)
	rtime_t temp;
	ftime (&temp);
	return temp;

	/* -------------------------------------------------- */

	/* gettimeofday replacement by Daniel Clausen.  */
#else
# if defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIMEB_H)
	rtime_t temp;
	struct timeval tmp;

	gettimeofday (&tmp, NULL);
	temp.time = tmp.tv_sec;
	temp.millitm = tmp.tv_usec / 1000;
	temp.timezone = 0;
	temp.dstflag = 0;

	return temp;

# else
	return (time (0));
# endif  
#endif
}
