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

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "basename.h"
#include "closeout.h"
#include "error.h"
#include "progname.h"

#include "log.h"
#include "tateplay-game.h"
#include "tateplay-loop.h"
#include "tateplay-time-control.h"
#include "util.h"

static void usage(int status);
static void version(void);
static void set_protocol(const char *name);
static void set_depth(const char *depth);
static void set_seconds_per_move(const char *depth);
static void set_delay(const char *delay);
static void set_time_control(const char *depth);
static void handle_sigchld(int signo);
static void reap_children(void);
//static void unplay_move(chi_move *move);

static Game *game;

static int opt_protocol_seen = 0;
static int opt_depth_seen = 0;
static int opt_seconds_per_move_seen = 0;
static int opt_time_control_seen = 0;
static int opt_delay_seen = 0;

static const struct option long_options[] = {
	{ "white", required_argument, NULL, 'w' },
	{ "black", required_argument, NULL, 'b' },
	{ "protocol", required_argument, NULL, 'p' },
	{ "depth", required_argument, NULL, 'd' },
	{ "seconds-per-move", required_argument, NULL, CHAR_MAX + 5 },
	{ "time-control", required_argument, NULL, 't' },
	{ "delay", required_argument, NULL, CHAR_MAX + 8 },
	{ "event", required_argument, NULL, 'e' },
	{ "site", required_argument, NULL, 's' },
	{ "round", required_argument, NULL, 'r' },
	{ "player-white", required_argument, NULL, CHAR_MAX + 1 },
	{ "player-black", required_argument, NULL, CHAR_MAX + 2 },
	{ "option-white", required_argument, NULL, CHAR_MAX + 3 },
	{ "option-black", required_argument, NULL, CHAR_MAX + 4 },
	{ "ponder-white", no_argument, NULL, CHAR_MAX + 6 },
	{ "ponder-black", no_argument, NULL, CHAR_MAX + 7 },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "verbose", no_argument, NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};

#ifdef DEBUG_XMALLOC
# include "xmalloc-debug.c"
#endif

int
main(int argc, char *argv[])
{
	int optchar;
	bool do_help = false;
	bool do_version = false;
	bool white_seen = false;
	bool black_seen = false;
	struct sigaction sa;
	bool status;

#ifdef DEBUG_XMALLOC
	init_xmalloc_debug();
#endif

	game = game_new();

	set_program_name(argv[0]);

	atexit(close_stdout);
	atexit(reap_children);

	sa.sa_handler = &handle_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, 0) == -1)
		error(EXIT_FAILURE, errno, "cannot install SIGCHLD handler");

	while ((optchar = getopt_long(argc, argv,
	                              "w:b:d:e:s:r:hp:Vv",
								  long_options, NULL)) != EOF) {
		switch (optchar) {
			case '\0':
				/* Long option.  */
				break;

			case 'w':
				white_seen = true;
				engine_add_argv(game->white, optarg);
				break;

			case 'b':
				black_seen = true;
				engine_add_argv(game->black, optarg);
				break;

			case 'p':
				set_protocol(optarg);
				break;

			case 'd':
				set_depth(optarg);
				break;

			case 't':
				set_time_control(optarg);
				break;

			case 'e':
				game_set_event(game, optarg);
				break;

			case 's':
				game_set_site(game, optarg);
				break;

			case 'r':
				game_set_round(game, optarg);
				break;

			case CHAR_MAX + 1:
				game_set_player_white(game, optarg);
				break;

			case CHAR_MAX + 2:
				game_set_player_black(game, optarg);
				break;
			
			case CHAR_MAX + 3:
				game_set_option_white(game, optarg);
				break;

			case CHAR_MAX + 4:
				game_set_option_black(game, optarg);
				break;

			case CHAR_MAX + 5:
				set_seconds_per_move(optarg);
				break;

                        case CHAR_MAX + 6:
				engine_turn_on_ponder(game->white);
				break;

                        case CHAR_MAX + 7:
				engine_turn_on_ponder(game->black);
                                break;

			case CHAR_MAX + 8:
				set_delay(optarg);
				break;

			case 'h':
				do_help = true;
				break;

			case 'V':
				do_version = true;
				break;

			case 'v':
				++verbose;
				break;

			default:
				usage(EXIT_FAILURE);
				/* NOTREACHED */
		}
	}

	if (do_version) version();
	if (do_help) usage(EXIT_SUCCESS);

	if (!(white_seen && black_seen)) {
		error(EXIT_SUCCESS, 0,
		      "The options '--white' and '--black' are mandatory");
		usage(EXIT_FAILURE);
	}

	status = tateplay_loop(game);

	game_print_pgn(game);

	log_realm("info", "terminating engines");
	game_destroy(game);

	return status ? EXIT_SUCCESS : EXIT_FAILURE;
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
Engine selection:\n");
		printf("\
  -w, --white=CMD_OR_ARG      white engine\n");
		printf("\
  -b, --black=CMD_OR_ARG      black engine\n");
		printf("\n");
		printf("\
Engine behavior and properties (first time for both engines, second time only\n\
for black):\n\
");
		printf("\
  -p, --protocol=PROTOCOL     one of 'uci' (default), 'xboard', or 'cecp'\n\
                              (an alias for 'xboard'\n");
        printf("\
  -d, --depth=PLIES           search at most to depth PLIES\n");
        printf("\
      --seconds-per-move=SEC  set time control to fixed SEC seconds per move\n");
        printf("\
  -t, --time-control=NUM MINUTES INCREMENT  set time control to NUM moves\n\
      --delay=MS              start sending commands to the engine after MS ms\n\
");
		printf("\n");
		printf("\
You can specify fractions of a minute for the time control, for example 0:30.\n\
The three tokens can be separate by anything but a digit, a plus sign ('+'),\n\
or a colon (':').  The default is '40 15:00 0'.\n\
");
		printf("\n");
		printf("\
General engine options\n");
		printf("\
      --ponder-white            turn on pondering for white engine\n\
      --ponder-black            turn on pondering for black engine\n\
      --option-white=KEY=VALUE  set option KEY for white engine to VALUE\n\
      --option-black=KEY=VALUE  set option KEY for black engine to VALUE\n");
		printf("\n");
		printf ("\
Game information:\n");
		printf ("\
  -e, --event=EVENT           the name of the event\n");
        printf ("\
  -s, --site=SITE             the name of the site\n");
        printf ("\
  -r, --round=ROUND           the round of the match\n");
        printf ("\
  -f, --fen=FEN               the starting position in Forsyth-Edwards\n\
                              Notation (FEN)\n");
		printf("\
Informative output:\n");
		printf ("\
  -h, --help                  display this help and exit\n");
		printf ("\
  -V, --version               output version information and exit\n");
		printf ("\n");
		printf("\
  -v, --verbose               increase verbosity level\n");
		printf ("\n");
		fputs ("Report bugs at <https://github.com/gflohr/tate/issues>.\n",
		       stdout);
	}

	exit(status);
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

static void
handle_sigchld(int signo)
{
	int saved_errno = errno;
	pid_t pid = 1;

	while (1) {
	pid = waitpid((pid_t) -1, 0, WNOHANG);
		if (pid <= 0) break;
		if (game->white && game->white->pid == pid) {
			child_exited = 1;
			log_realm(game->white->nick, "engine exited unexpectedly.");
		} else if (game->black && game->black->pid == pid) {
			child_exited = 1;
			log_realm(game->black->nick, "engine exited unexpectedly.");
		}
	}

	errno = saved_errno;
}

static void
reap_children(void)
{
	while (waitpid((pid_t) -1, 0, WNOHANG) > 0) {}
}

static void
set_protocol(const char *proto_name)
{
	EngineProtocol protocol;

	if (0 == strcasecmp("uci", proto_name)) {
		protocol = uci;
	} else if (0 == strcasecmp("xboard", proto_name)) {
		protocol = xboard;
	} else if (0 == strcasecmp("cecp", proto_name)) {
		protocol = xboard;
	} else {
		error(EXIT_SUCCESS, 0, "unsupported protocol '%s'.", proto_name);
		usage(EXIT_FAILURE);
	}

	switch(++opt_protocol_seen) {
		case 1:
			engine_set_protocol(game->white, protocol);
			/* FALLTHRU */
		case 2:
			engine_set_protocol(game->black, protocol);
			break;
		default:
			error(EXIT_SUCCESS, 0,
			      "option --protocol can be given at most twice.");
			usage(EXIT_FAILURE);
	}
}

static void
set_depth(const char *_depth)
{
	long depth;

	if (!parse_integer(&depth, _depth)) {
		error(EXIT_SUCCESS, errno, "invalid depth \"%s\"", _depth);
		usage(EXIT_FAILURE);
	}

	if (depth <= 0) {
		error(EXIT_SUCCESS, 0, "depth must be a positive number");
		usage(EXIT_FAILURE);
	}

	switch(++opt_depth_seen) {
		case 1:
			game->white->depth = depth;
			/* FALLTHRU */
		case 2:
			game->black->depth = depth;
			break;
		default:
			error(EXIT_SUCCESS, 0,
			      "option --depth can be given at most twice.");
			usage(EXIT_FAILURE);
	}
}

static void
set_seconds_per_move(const char *spec)
{
	TimeControl tc;

	if (opt_time_control_seen) {
		error(EXIT_SUCCESS, 0,
		      "the options '--seconds-per-move' and '--time-control' are \
mutually exclusive");
		usage(EXIT_FAILURE);
	}

	if (!time_control_init_st(&tc, spec)) {
		error(EXIT_SUCCESS, 0, "invalid seconds per move '%s'", spec);
		usage(EXIT_FAILURE);
	}

	switch(++opt_seconds_per_move_seen) {
		case 1:
			game->white->tc = tc;
			/* FALLTHRU */
		case 2:
			game->black->tc = tc;
			break;
		default:
			error(EXIT_SUCCESS, 0,
			      "option --seconds-per-move can be given at most twice.");
			usage(EXIT_FAILURE);
	}
}

static void
set_time_control(const char *spec)
{
	TimeControl tc;

	if (opt_seconds_per_move_seen) {
		error(EXIT_SUCCESS, 0,
		      "the options '--seconds-per-move' and '--time-control' are \
mutually exclusive");
		usage(EXIT_FAILURE);
	}

	if (!time_control_init_level(&tc, spec)) {
		error(EXIT_SUCCESS, 0, "invalid time control '%s'", spec);
		usage(EXIT_FAILURE);
	}

	switch(++opt_time_control_seen) {
		case 1:
			game->white->tc = tc;
			/* FALLTHRU */
		case 2:
			game->black->tc = tc;
			break;
		default:
			error(EXIT_SUCCESS, 0,
			      "option --seconds-per-move can be given at most twice.");
			usage(EXIT_FAILURE);
	}
}

static void
set_delay(const char *delay)
{
	long ms;

	if (!parse_integer(&ms, delay) || delay < 0) {
		error(EXIT_FAILURE, errno, "invalid delay '%s'", delay);
	}

	switch(++opt_delay_seen) {
		case 1:
			game->white->delay = ms;
			/* FALLTHRU */
		case 2:
			game->black->delay = ms;
			break;
		default:
			error(EXIT_SUCCESS, 0,
			      "option --seconds-per-move can be given at most twice.");
			usage(EXIT_FAILURE);
	}
}
