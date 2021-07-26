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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

long long int 
rdifftime (struct timeval end, struct timeval start)
{
	long long int timediff = 1000 * (end.tv_sec - start.tv_sec);
	timediff += (end.tv_usec - start.tv_usec) / 1000;

	return timediff;
}

struct timeval 
rtime () 
{
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0) {
		fprintf(stderr, "Error getting current time: %s\n",
		        strerror(errno));
		
	}

	return now;
}
