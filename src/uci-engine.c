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

#include "uci-engine.h"

#include "state.h"
#include "think.h"
#include "util.h"
#include "perft.h"

#define DELIM " \n\t\v\f\r"

static char *
next_token(char **string)
{
	if (!*string)
		return NULL;

	*string = (char *) ltrim(*string);
	return strsep(string, DELIM);
}

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
			case 'p':
				if(strcmp(command + 1, "osition") == 0) {
					go_on = uci_handle_position(options, trim(trimmed), out);
				}
				break;
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

		fflush(out);

		if (go_on < 0) {
			fprintf(out, "Unknown command: %s\n", command);
			fflush(out);
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
uci_handle_uci(UCIEngineOptions *options, char *args, FILE *out)
{
	fprintf(out, "id name %s %s\n", PACKAGE, PACKAGE_VERSION);
	fprintf(out, "id author %s\n", "Guido Flohr <guido.flohr@cantanea.com>");
	fprintf(out, "option name Threads type spin default 1 min 1 max %u\n",
	        UCI_ENGINE_MAX_THREADS);
	fprintf(out, "uciok\n");

	return 1;
}

int
uci_handle_debug(UCIEngineOptions *options, char *args, FILE *out)
{
	if (strcmp(args, "on") == 0) {
		options->debug = 1;
	} else if (strcmp(args, "off") == 0) {
		options->debug = 0;
	}

	return 1;
}

int
uci_handle_position(UCIEngineOptions *options, char *args, FILE *out)
{
	char *rest;
	const char *type;
	const char *command;
	int errnum;
	chi_move move;
	const char *movestr;

	if (!args) {
		fprintf(out, "info Command 'position' requires an argument.\n");
		return 1;
	}

	rest = (char *) args;
	type = strsep(&rest, DELIM);
	if (strcmp("fen", type) == 0) {
		if (!rest) {
			fprintf(out, "info Command 'position fen' requires an argument.\n");
			return 1;
		}

		rest = trim(rest);
		errnum = chi_extract_position(&tate.position, rest, (const char **) &rest);
		if (errnum) {
			fprintf(out, "info Invalid FEN string (%d).\n", errnum);
			return 1;
		}
	} else if (strcmp("startpos", type) == 0) {
		chi_init_position(&tate.position);
	} else {
		fprintf(out, "info Command 'position' requires one of 'startpos' or"
			" 'fen' as an argument.\n");
		return 1;
	}

	rest = trim(rest);
	if (!rest || !*rest)
		return 1;

	command = strsep(&rest, DELIM);
	if (strcmp("moves", command)) {
		fprintf(out, "info Expected 'moves' after position, not '%s'.\n",
		        command);
		return 1;
	}

	while (rest) {
		movestr = strsep(&rest, DELIM);
		if (*movestr) {
			errnum = chi_parse_move(&tate.position, &move, movestr);
			if (errnum) {
				fprintf(out, "info Invalid move '%s'.\n", movestr);
				return 1;
			}

			errnum = chi_check_legality(&tate.position, move);
			if (errnum) {
				fprintf(out, "info Illegal move '%s'.\n", movestr);
				return 1;
			}

			errnum = chi_apply_move(&tate.position, move);
			if (errnum) {
				fprintf(out, "info Cannot apply move '%s'.\n", movestr);
				return 1;
			}
		}
	}

	return 1;
}

int
uci_handle_go(UCIEngineOptions *options, char *args, FILE *out)
{
	char *bestmove = NULL;
	char *pondermove = NULL;
	unsigned int bufsize;
	int errnum;
	char *token;
	char *argptr = args;
	unsigned long perft_depth = 0;
	char *endptr;

	while ((token = next_token(&argptr)) != NULL) {
		if (strcmp("perft", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: perft DEPTH.\n");
				return 1;
			}
			perft_depth = strtoul (token, &endptr, 10);
			if (perft_depth == 0 && endptr == token) {
				fprintf (out, "info error: illegal perft depth: %s.\n",
				         token);
				return 1;
			}
			chi_pos position;
			chi_copy_pos(&position, &tate.position);
			(void) perft(&position, perft_depth, out);
			return 1;
		} else {
			fprintf(out, "info unknown or unsupported go option '%s'.\n",
			        token);
		}
	}

	think();

	if (tate.bestmove_found) {
		errnum = chi_coordinate_notation(
			tate.bestmove, chi_on_move(&tate.position), &bestmove, &bufsize);
		if (!errnum && tate.pondermove_found) {
			errnum = chi_coordinate_notation(
				tate.pondermove, !chi_on_move(&tate.position), &pondermove,
				&bufsize);
		}

		if (errnum) {
			fprintf(out, "bestmove 0000\n");
		} else if (pondermove) {
			fprintf(out, "bestmove %s pondermove %s\n", bestmove, pondermove);
		} else {
			fprintf(out, "bestmove %s\n", bestmove);
		}

		if (bestmove) free(bestmove);
		if (pondermove) free(pondermove);
	} else {
		fprintf(out, "bestmove 0000\n");
	}

	return 1;
}
