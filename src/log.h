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

#ifndef _LOG_H
# define _LOG_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int verbose;

extern void log_realm(const char *realm, const char *message, ...);
extern void info_realm(const char *realm, const char *message, ...);
extern void log_engine_in(const char *realm, const char *message, ...);
extern void log_engine_out(const char *realm, const char *message, ...);
extern void log_engine_error(const char *realm, const char *message, ...);
extern void log_engine_fatal(const char *realm, const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif
