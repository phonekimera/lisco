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

#ifndef _UTIL_H
# define _UTIL_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libchi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *trim(char *str);
extern const char *ltrim(const char *str);
extern chi_bool parse_integer(long *result, const char *string);
extern chi_bool parse_double(double *result, const char *string);
extern void time_diff(struct timeval *result, const struct timeval *start,
                      const struct timeval *end);
extern void time_add(struct timeval *total, const struct timeval *elapsed);
extern void time_subtract(struct timeval *total, const struct timeval *elapsed);
extern chi_bool time_is_left(const struct timeval *deadline,
                             const struct timeval *now);

#ifdef __cplusplus
}
#endif

#endif
