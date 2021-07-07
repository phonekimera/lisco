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

#include <stdlib.h>
#include <stdio.h>

#include <libchi.h>

#include "basename.h"
#include "closeout.h"
#include "error.h"
#include "progname.h"
#include "xalloc.h"

#include "tate.h"
#include "uci-engine.h"
#include "xmalloca-debug.h"

chi_zk_handle zk_handle;
UCIEngineOptions uci_options;

static void greeting();

#ifdef DEBUG_XMALLOC
# include "xmalloc-debug.c"
#endif

int
main (argc, argv)
int argc;
char *argv[];
{
	int errnum;

#ifdef DEBUG_XMALLOC
	init_xmalloc_debug();
#endif

	set_program_name(argv[0]);

	atexit(close_stdout);

	greeting();

	errnum = chi_zk_init(&zk_handle);
	if (errnum)
		error (EXIT_FAILURE,
		       0,
		       "Cannot initialize Zobrist key array: %s",
		       chi_strerror (errnum));

	uci_init(&uci_options);
	uci_main(&uci_options,
	         stdin, "[standard input]",
	         stdout, "[standard output]");

	return EXIT_SUCCESS;
}

static void
greeting()
{
	printf("%s %s\n", basename(program_name), VERSION);
	printf("Copyright (C) %s cantanea EOOD (http://www.cantanea.com)\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
",
	       "2002-2021");
	printf("Written by Guido Flohr.\n");
}
