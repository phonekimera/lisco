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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include "util.h"

char *
trim(char *str)
{
	char *end_ptr;

	while (isspace((unsigned char) *str)) ++str;

	if (*str =='\0') return str;

	end_ptr = str + strlen(str) - 1;
	while (end_ptr > str && isspace((unsigned char) *end_ptr)) --end_ptr;

	end_ptr[1] = '\0';

	return str;
}

const char *
ltrim(const char *str)
{
	while (isspace((unsigned const char) *str)) ++str;

	return str;
}

chi_bool
parse_integer(long *result, const char *string)
{
	long number;
	char *endptr;

	errno = 0;
	number = strtoul(string, &endptr, 10);
	if (number == LONG_MAX && errno) {
		return chi_false;
	} else if (string[0] && *endptr == 0) {
		*result = number;
		return chi_true;
	} else if (endptr == string) {
		return chi_false;
	} else if (*endptr) {
		/* Trailing garbage.  */
		return chi_false;
	}

	return chi_false;
}

chi_bool
parse_double(double *result, const char *string)
{
	long number;
	char *endptr;

	errno = 0;
	number = strtod(string, &endptr);
	if (number == HUGE_VAL && errno) {
		return chi_false;
	} else if (string[0] && *endptr == 0) {
		*result = number;
		return chi_true;
	} else if (endptr == string) {
		return chi_false;
	} else if (*endptr) {
		/* Trailing garbage.  */
		return chi_false;
	}

	return chi_false;
}

void
time_diff(struct timeval *result,
          const struct timeval *start, const struct timeval *end)
{
	result->tv_sec = end->tv_sec - start->tv_sec;
	if (end->tv_usec >= start->tv_usec) {
		result->tv_usec = end->tv_usec - start->tv_usec;
	} else {
		--result->tv_sec;
		result->tv_usec = 1000000 + end->tv_usec - start->tv_usec;
	}
}

void
time_add(struct timeval *total, const struct timeval *elapsed)
{
	total->tv_sec += elapsed->tv_sec;
	total->tv_usec += elapsed->tv_usec;
	if (total->tv_usec >= 1000000) {
		++total->tv_sec;
		total->tv_usec -= 1000000;
	}
}

void
time_subtract(struct timeval *total, const struct timeval *elapsed)
{
	total->tv_sec -= elapsed->tv_sec;
	if (total->tv_usec >= elapsed->tv_usec) {
		total->tv_usec -= elapsed->tv_usec;
	} else {
		--total->tv_sec;
		total->tv_usec = 1000000 + total->tv_usec - elapsed->tv_usec;
	}
}

chi_bool
time_is_left(const struct timeval *deadline, const struct timeval *now)
{
	return now->tv_sec < deadline->tv_sec || now->tv_usec < deadline->tv_usec;
}

long
num_cpus(void)
{
#ifdef _WIN32
# ifndef _SC_NPROCESSORS_ONLN
SYSTEM_INFO info;
GetSystemInfo(&info);
#  define sysconf(a) info.dwNumberOfProcessors
#  define _SC_NPROCESSORS_ONLN
# endif
#endif

#ifdef _SC_NPROCESSORS_ONLN
	return sysconf(_SC_NPROCESSORS_ONLN);
#else
# error "don't know how to determine number of CPUs"
#endif
}