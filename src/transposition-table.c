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

#include <string.h>
#include <sys/types.h>

#include <libchi.h>

#include "lisco.h"
#include "util.h"

typedef struct TTEntry {
	unsigned age: 9;
	unsigned type: 2;
	unsigned move: 22;
	unsigned draft: 15;
	unsigned value: 16;
	unsigned signature_hi: 32;
	unsigned signature_lo: 32;
} TTEntry;

// Roughly 0.6 MB.
#define MIN_TT_SIZE (sizeof (TTEntry) * 20000)

static TTEntry *tt = NULL;
static void *tt_free_me = NULL;
static size_t tt_size = 0;

void
tt_init(size_t size)
{
	tt_destroy();

	if (size < MIN_TT_SIZE)
		size = MIN_TT_SIZE;
	
	tt_size = size;

	tt = xmalloc_aligned(&tt_free_me, 64, tt_size * sizeof *tt);

	tt_clear();
}

void
tt_clear(void)
{
	memset(tt, 0, tt_size * sizeof *tt);
}

void
tt_destroy(void)
{
	if (tt_free_me)
		free(tt_free_me);
	
	tt_free_me = NULL;
	tt = NULL;
	tt_size = 0;
}
