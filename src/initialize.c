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

#include <stdlib.h>
#include <stdio.h>

#include <error.h>
#include <progname.h>

#include <libchi.h>

#include "lisco.h"
#include "uci-engine.h"

#ifdef DEBUG_XMALLOC
# include "xmalloc-debug.c"
#endif

Lisco lisco;

void
lisco_initialize(const char *program_name)
{
	int errnum;

	set_program_name(program_name);

	memset(&lisco, 0, sizeof lisco);

	chi_init_position (&lisco.position);

	uci_init(&lisco.uci, stdin, "[standard input]",
			stdout, "[standard output]");
	chi_mm_init();
	tt_init(LISCO_DEFAULT_TT_SIZE * 1 << 20);
	init_ev_hash(1024 * 1024 * 100);
	errnum = chi_zk_init(&lisco.zk_handle);
	if (errnum) {
		error (EXIT_FAILURE, 0,
		       "Cannot initialize Zobrist key array: %s",
		       chi_strerror (errnum));
	}
}
