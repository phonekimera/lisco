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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "xmalloca.h"

#include "log.h"

int verbose = 0;

static const char *wdays[] = {
	"Sun", "Mon", "Tue", "Wed", "Tue", "Fri", "Sat"
};
static const char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static void vlog_realm(const char *realm, const char *direction, 
                       const char *_msg, va_list ap);

void
log_realm(const char *realm, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_realm(realm, NULL, fmt, ap);
	va_end(ap);
}

void
log_engine_in(const char *engine, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_realm(engine, " <<<", fmt, ap);
	va_end(ap);
}

void
log_engine_out(const char *engine, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_realm(engine, " >>>", fmt, ap);
	va_end(ap);
}

void
log_engine_error(const char *engine, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_realm(engine, "[error]", fmt, ap);
	va_end(ap);
}

void
vlog_realm(const char *realm, const char *direction, const char *_fmt,
           va_list ap)
{
	char *line;
	struct timeval now;
	struct tm tm;
	char *fmt;
	size_t fmt_size;
	size_t percents = 0;
	const char *src;
	char *dest;
	char *escaped_realm;

	if (!verbose) return;

	gettimeofday(&now, NULL);
	localtime_r(&now.tv_sec, &tm);

	/* Escape all percent signs in realm.  */
	src = realm;
	while (*src) {
		if (*src++ == '%') ++percents;
	}
	escaped_realm = xmalloc(strlen(realm) + percents + 1);
	src = realm;
	dest = escaped_realm;
	while (*src) {
		char c = *src++;
		*dest++ = c;
		if (c == '%') *dest++ = '%';
	}

	/* The dimensions of the format are as follows:
	 *
	 * [31][realm]_
	 * 
	 * 31 characters for the timestamp, plus 4 brackets, plus one space as a
	 * separator, plus the escaped realm, plus the original format string,
	 * plus a newline plus one null byte.
	 */
	fmt_size = 38 + strlen(escaped_realm) + strlen(_fmt);
	if (direction) fmt_size += strlen(direction);
	fmt = xmalloc(fmt_size);
	snprintf(fmt, fmt_size,
	         "[%s %s %02u %02u:%02u:%02u.%05u %04u][%s]%s %s\n",
	         wdays[tm.tm_wday],
	         months[tm.tm_mon],
	         tm.tm_mday,
	         tm.tm_hour, tm.tm_min, tm.tm_sec, now.tv_usec,
	         tm.tm_year + 1900,
	         escaped_realm, direction ? direction : "", _fmt);
	vfprintf(stderr, fmt, ap);
	free(fmt);
}