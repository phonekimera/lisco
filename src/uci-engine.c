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

#include <string.h>
#include <errno.h>

#define TEST_UCI_ENGINE 1

#include "util.h"
#include "uci-engine.h"

#define DELIM " \n\t\v\f\r"

void
uci_init(UCIEngineOptions *options)
{
	memset(options, 0, sizeof *options);

	options->option_threads = 1;
}

int
uci_main(UCIEngineOptions *options, FILE *in, const char *inname,
         FILE *out, const char *outname)
{
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen = 1;

	while ((linelen = getline(&line, &linecap, in)) > 0) {
		char *trimmed = trim(line);
		const char *command = strsep(&trimmed, DELIM);
		int go_on = -1;

		switch(command[0]) {
			case 'g':
				if(strcmp(command + 1, "o") == 0) {
					go_on = uci_handle_go(options, trim(trimmed), out);
				}
				break;
			case 'q':
				if(strcmp(command + 1, "uit") == 0) {
					go_on = uci_handle_quit(options);
				}
				break;
			case 'u':
				if(strcmp(command + 1, "ci") == 0) {
					go_on = uci_handle_uci(options, trim(trimmed), out);
				}
				break;
			case 'd':
				if(strcmp(command + 1, "ebug") == 0) {
					go_on = uci_handle_debug(options, trim(trimmed), out);
				}
				break;
			case 0:
				go_on = 1;
				break;
		}

		if (go_on < 0) {
			fprintf(out, "Unknown command: %s\n", command);
		} else if (go_on == 0) {
			break;
		}
	}

	if (linelen < 0) {
		if (!(feof(in))) {
			fprintf(out, "info error reading from '%s': %s!\n", inname,
		            strerror(errno));
			return 1;
		}
	}

	return 0;
}

int
uci_handle_quit(UCIEngineOptions *options)
{
	return 0;
}

int
uci_handle_uci(UCIEngineOptions *options, const char *args, FILE *out)
{
	fprintf(out, "id name %s %s\n", PACKAGE, PACKAGE_VERSION);
	fprintf(out, "id author %s\n", "Guido Flohr <guido.flohr@cantanea.com>");
	fprintf(out, "option name Threads type spin default 1 min 1 max %u\n",
	        UCI_ENGINE_MAX_THREADS);
	fprintf(out, "uciok\n");

	return 1;
}

int
uci_handle_debug(UCIEngineOptions *options, const char *args, FILE *out)
{
	if (strcmp(args, "on") == 0) {
		options->debug = 1;
	} else if (strcmp(args, "off") == 0) {
		options->debug = 0;
	}

	return 1;
}

int
uci_handle_go(UCIEngineOptions *options, const char *args, FILE *out)
{
	fprintf(out, "bestmove e2e4 ponder e7e5\n");

	return 1;
}
