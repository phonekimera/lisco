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
#include <errno.h>
#include <ctype.h>

#define TEST_UCI_ENGINE 1

#include "uci-engine.h"

#include "lisco.h"
#include "util.h"

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
uci_init(UCIEngineOptions *options, FILE *in, const char *inname,
         FILE *out, const char *outname)
{
	memset(options, 0, sizeof *options);

	options->option_threads = 1;
	options->in = in;
	options->inname = inname;
	options->out = out;
	options->outname = outname;
}

int
uci_main(UCIEngineOptions *options)
{
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen = 1;
	FILE *in = options->in;
	const char *inname = options->inname;
	FILE *out = options->out;

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
			case 'i':
				if(strcmp(command + 1, "sready") == 0) {
					go_on = uci_handle_isready(options, trim(trimmed), out);
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
		errnum = chi_extract_position(&lisco.position, rest, (const char **) &rest);
		if (errnum) {
			fprintf(out, "info Invalid FEN string (%d).\n", errnum);
			return 1;
		}
	} else if (strcmp("startpos", type) == 0) {
		chi_init_position(&lisco.position);
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
			errnum = chi_parse_move(&lisco.position, &move, movestr);
			if (errnum) {
				fprintf(out, "info Invalid move '%s'.\n", movestr);
				return 1;
			}

			errnum = chi_check_legality(&lisco.position, move);
			if (errnum) {
				fprintf(out, "info Illegal move '%s'.\n", movestr);
				return 1;
			}

			errnum = chi_apply_move(&lisco.position, move);
			if (errnum) {
				fprintf(out, "info Cannot apply move '%s'.\n", movestr);
				return 1;
			}
		}
	}

	return 1;
}

static int is_coordinate_notation(char *movestr) {
	movestr[0] = tolower(movestr[0]);
	if (movestr[0] < 'a' || movestr[0] > 'h') {
		return 0;
	}

	if (movestr[1] < '1' || movestr[1] > '8') {
		return 0;
	}

	movestr[2] = tolower(movestr[2]);
	if (movestr[2] < 'a' || movestr[2] > 'h') {
		return 0;
	}

	if (movestr[3] < '1' || movestr[3] > '8') {
		return 0;
	}

	if (movestr[4] == 0) {
		return 1;
	}

	movestr[4] = tolower(movestr[4]);
	if (movestr[4] != 'q' && movestr[4] != 'r'
	    && movestr[4] != 'b' && movestr[4] != 'n') {
		return 0;
	}

	return !movestr[5];
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

	Tree tree;
	SearchParams params;

	memset(&tree, 0, sizeof tree);
	memset(&params, 0, sizeof params);

	move_list_init(&params.searchmoves);

	while ((token = next_token(&argptr)) != NULL) {
		if (strcmp("searchmoves", token) == 0) {
			// Protect against crashes caused by giving searchmoves
			// multiple times.
			move_list_destroy(&params.searchmoves);
			move_list_init(&params.searchmoves);

			while ((token = next_token(&argptr))) {
				// Do a very basic check on the move syntax so that we can
				// distinguish a move from other commands that are following.
				// One of the many flaws in the UCI protocol ...
				if (!is_coordinate_notation(token)) {
					break;
				}

				chi_move move;
				errnum = chi_parse_move(&lisco.position, &move, token);
				if (errnum) {
					fprintf(out, "info illegal move '%s': %s\n",
					        token, chi_strerror(errnum));
					move_list_destroy(&params.searchmoves);
					return 1;
				}
				move_list_add(&params.searchmoves, move);
			}

			// An emtpy move list is ignored.
			if (!params.searchmoves.num_moves) {
				fprintf(out, "info 'searchmoves' without moves is ignored.\n");
			}
		} else if (strcmp("ponder", token) == 0) {
			fprintf(out, "info error: go option 'ponder' is not yet supported.\n");
			return 1;
		} else if (strcmp("wtime", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: wtime MS.\n");
				return 1;
			}
			unsigned long wtime = strtoul(token, &endptr, 10);
			if (wtime == 0 || endptr == token) {
				fprintf(out, "info error: illegal wtime: %s.\n",
				        token);
				return 1;
			}
			if (chi_on_move(&lisco.position) == chi_white) {
				params.mytime = wtime;
			} else {
				params.hertime = wtime;
			}
		} else if (strcmp("btime", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: btime MS.\n");
				return 1;
			}
			unsigned long btime = strtoul(token, &endptr, 10);
			if (btime == 0 || endptr == token) {
				fprintf(out, "info error: illegal btime: %s.\n",
				        token);
				return 1;
			}
			if (chi_on_move(&lisco.position) == chi_black) {
				params.mytime = btime;
			} else {
				params.hertime = btime;
			}
		} else if (strcmp("winc", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: winc S.\n");
				return 1;
			}
			unsigned long winc = strtoul(token, &endptr, 10);
			if (winc == 0 || endptr == token) {
				fprintf(out, "info error: illegal winc: %s.\n",
				        token);
				return 1;
			}
			if (chi_on_move(&lisco.position) == chi_white) {
				params.myinc = winc;
			} else {
				params.herinc = winc;
			}
		} else if (strcmp("binc", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: binc S.\n");
				return 1;
			}
			unsigned long binc = strtoul(token, &endptr, 10);
			if (binc == 0 || endptr == token) {
				fprintf(out, "info error: illegal binc: %s.\n",
				        token);
				return 1;
			}
			if (chi_on_move(&lisco.position) == chi_black) {
				params.myinc = binc;
			} else {
				params.herinc = binc;
			}
		} else if (strcmp("movestogo", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: depth PLIES.\n");
				return 1;
			}
			params.movestogo = strtoul(token, &endptr, 10);
			if (endptr == token) {
				fprintf(out, "info error: illegal movestogo: %s.\n",
				         token);
				return 1;
			}
		} else if (strcmp("depth", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: depth PLIES.\n");
				return 1;
			}
			params.depth = strtoul(token, &endptr, 10);
			if (params.depth == 0 || endptr == token) {
				fprintf(out, "info error: illegal depth: %s.\n",
				         token);
				return 1;
			}
		} else if (strcmp("nodes", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: nodes NODES.\n");
				return 1;
			}
			params.nodes = strtoul(token, &endptr, 10);
			if (params.nodes == 0 || endptr == token) {
				fprintf(out, "info error: illegal nodes: %s.\n",
				        token);
				return 1;
			}
		} else if (strcmp("mate", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: mate MOVES.\n");
				return 1;
			}
			params.mate = strtoul(token, &endptr, 10);
			if (params.mate == 0 || endptr == token) {
				fprintf(out, "info error: illegal mate depth: %s.\n",
				        token);
				return 1;
			}
		} else if (strcmp("movetime", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: movetime MS.\n");
				return 1;
			}
			params.movetime = strtoul(token, &endptr, 10);
			if (params.movetime == 0 || endptr == token) {
				fprintf(out, "info error: illegal movetime: %s.\n",
				        token);
				return 1;
			}
		} else if (strcmp("infinite", token) == 0) {
			fprintf(out, "info error: 'infinite' is not yet supported.\n");
			return 1;
		} else if (strcmp("perft", token) == 0) {
			token = next_token(&argptr);
			if (!token) {
				fprintf(out, "info usage: perft DEPTH.\n");
				return 1;
			}
			perft_depth = strtoul(token, &endptr, 10);
			if (perft_depth == 0 && endptr == token) {
				fprintf(out, "info error: illegal perft depth: %s.\n",
				         token);
				return 1;
			}
			chi_pos position;
			chi_copy_pos(&position, &lisco.position);
			(void) perft(&position, perft_depth, NULL, out);
			return 1;
		} else {
			fprintf(out, "info unknown or unsupported go option '%s'.\n",
			        token);
		}

		if (!process_search_params(&tree, &params)) {
			fprintf(out, "info cannot understand search parameters.\n");
			return 1;
		}
	}


	think(&tree);
	move_list_destroy(&tree.searchmoves);

	if (lisco.bestmove_found) {
		errnum = chi_coordinate_notation(
			lisco.bestmove, chi_on_move(&lisco.position), &bestmove, &bufsize);
		if (!errnum && lisco.pondermove_found) {
			errnum = chi_coordinate_notation(
				lisco.pondermove, !chi_on_move(&lisco.position), &pondermove,
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

int
uci_handle_isready(UCIEngineOptions *options, char *args, FILE *out)
{
	fprintf(out, "readyok\n");

	return 1;
}
