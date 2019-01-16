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

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basename.h"
#include "closeout.h"
#include "error.h"
#include "progname.h"

static void usage(int status);
static void version(void);

static const struct option long_options[] = {
	{ "white", required_argument, NULL, 'w' },
	{ "black", required_argument, NULL, 'b' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' }
};

int
main(int argc, char *argv[])
{
	int optchar;
	bool do_help = false;
	bool do_version = false;

	set_program_name(argv[0]);

	atexit(close_stdout);
	
	while ((optchar = getopt_long(argc, argv,
	                              "w:b:hV",
								  long_options, NULL)) != EOF) {
		switch (optchar) {
			case '\0':
				/* Long option.  */
				break;

			case 'w':
				break;

			case 'b':
				break;

			case 'h':
				do_help = true;
				break;

			case 'V':
				do_version = true;
				break;

			default:
				usage(EXIT_FAILURE);
				/* NOTREACHED */
		}
	}

	if (do_version) version();
	if (do_help) usage(EXIT_SUCCESS);

	return 0;
}

static void
usage(int status)
{
	if (status != EXIT_SUCCESS)
		fprintf(stderr, "Try '%s --help' for more information.\n",
		        program_name);
	else {
		printf("\
Usage: %s [OPTION]...\n\
", program_name);
		printf("\n");
		printf("\
Let two chess engines play against each other.\n\
");
		printf("\n");
		printf("\
Mandatory arguments to long options are mandatory for short options too.\n\
Similarly for optional arguments.\n\
");
		printf("\n");
		printf("\
Engine selection (repeat for options and arguments for the engine):\n");
		printf("\
  -w, --white=CMD_OR_ARG      white engine\n");
		printf("\
  -b, --black=CMD_OR_ARG      black engine\n");
		printf("\n");
		printf ("\
Informative output:\n");
		printf ("\
  -h, --help                  display this help and exit\n");
		printf ("\
  -V, --version               output version information and exit\n");
		printf ("\n");
		fputs ("Report bugs at <https://github.com/gflohr/tate/issues>.\n",
		       stdout);
	}
}

static void
version(void)
{
	printf("%s (%s) %s\n", basename(program_name), PACKAGE, VERSION);
	printf("Copyright (C) %s cantanea EOOD (http://www.cantanea.com)\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
",
              "2002-2019");
	printf("Written by Guido Flohr.\n");
	exit (EXIT_SUCCESS);

}
