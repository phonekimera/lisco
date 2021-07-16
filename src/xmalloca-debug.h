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

#ifndef _XMALLOCA_DEBUG_H
# define _XMALLOCA_DEBUG_H        /* Allow multiple inclusion.  */

#include "xmalloca.h"

#ifdef DEBUG_XMALLOC
# define xmalloc(size) xmalloc_debug(size)
# define xrealloc(address, size) xrealloc_debug(address, size)
# define xstrdup(address) xstrdup_debug(address)
# define xstrndup(address, size) xstrndup_debug(address, size)
# if free == rpl_free
#  undef free
#  define free_orig(address) rpl_free(address)
# endif
# define free(address) xmalloc_debug_free(address)

extern void *xmalloc(size_t size);
extern void *xrealloc(void *address, size_t size);
extern char *xstrdup(const char *string);
extern char *xstrndup(const char *string, size_t n);
extern void free(void *address);
#endif

#endif
