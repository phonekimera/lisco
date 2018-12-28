/* tate.c - main program file for Tate.
 * Copyright (C) 2002 Guido Flohr (guido@imperia.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// FIXME: Check for availability in configure.in!
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <system.h>

#include <error.h>

#include <libchi.h>

#include "tate.h"
#include "edit.h"
#include "board.h"
#include "book.h"

static void greeting PARAMS ((void));
static void feature_requests PARAMS ((void));
static void handle_accepted PARAMS ((const char* feature));
static void handle_rejected PARAMS ((const char* feature));
static int handle_usermove PARAMS ((const char* movestr));
static void display_offsets PARAMS ((void));
static void display_moves PARAMS ((void));
static RETSIGTYPE sigio_handler PARAMS ((int));

int get_event PARAMS ((void));

#define WHITE_SPACE " \t\r\n\v\f"

chi_pos current;

int protover = 2;

char* name = NULL;
char* my_time = NULL;
char* opp_time = NULL;
int xboard = 0;
int force = 0;
int game_over = 0;
int thinking = 0;
int ics = 0;
int computer = 0;

int input_available = 0;
int event_pending   = 0;

static int edit_mode = 0;

#define PROMPT (chi_to_move (&current) == chi_white ? \
		"tate[white]> " : "tate[black]> ")

int
main (argc, argv)
    int argc;
    char* argv[];
{
    int flags;

    /* Standard input must be unbuffered, because we mix it with
       low-level I/O (which is of course a sin).  */
    setvbuf (stdin, NULL, _IONBF, 4096);
    setvbuf (stdout, NULL, _IOLBF, 4096);
    setvbuf (stderr, NULL, _IOLBF, 4096);

    greeting ();

    srandom (time (NULL));

    chi_init_position (&current);

    if (signal (SIGIO, sigio_handler) != 0)
	error (EXIT_FAILURE, errno, "Cannot install SIGIO handler");

    if (fcntl (fileno (stdin), F_SETOWN, getpid ()) != 0)
	error (EXIT_FAILURE, errno, "Cannot F_SETOWN stdin");

    flags = fcntl (fileno (stdin), F_GETFL);
    if (flags == -1)
	error (EXIT_FAILURE, errno, "Cannot get flags of standard input");

    flags |= O_ASYNC;
    if (fcntl (fileno (stdin), F_SETFL, flags) != 0)
	error (EXIT_FAILURE, errno, 
	       "Cannot set flags of standard input to asynchronous");

    fputs (PROMPT, stdout);
    fflush (stdout);

    while (1) {
	if (event_pending) {
	    int status = get_event ();
	    if (status & EVENT_TERMINATE)
		break;
	}
	sleep (1);
    }

    return EXIT_SUCCESS;
}

static void
greeting ()
{
    fprintf (stdout, "\
%s %s (libchi %s)\n
Copyright (C) 2002, Guido Flohr <guido@imperia.net>.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Written by Guido Flohr.\n\n", PACKAGE, VERSION, CHI_VERSION);

    fprintf (stdout, "Type 'help' for help.\n\n");
}

int
get_event ()
{
    int retval = EVENT_CONTINUE;
    static size_t bufsize = 0;
    static char* linebuf = NULL;    
    char* cmd;

    event_pending = 0;
    if (input_available) {
	if (-1 == getline (&linebuf, &bufsize, stdin)) {
	    if (feof (stdin))
		return EVENT_TERMINATE;
	    error (EXIT_FAILURE, errno, "Error reading from stdin");
	}
    } else {
	return retval;
    }
    
    cmd = strtok (linebuf, WHITE_SPACE);
    if (cmd == NULL)
	return retval;

    if (edit_mode) {
	retval = EVENT_STOP_THINKING;
	if (cmd [0] == '.' && cmd[1] == 0)
	    edit_mode = check_position (&current, 0);
	else if (strcasecmp (cmd, "display") == 0) {
	    /* Convenience.  */
	    display_board (&current);
	    fprintf (stdout, "\n");
	} else if (strcasecmp (cmd, "quit") == 0) {
	    /* Convenience, too.  */
	    return EVENT_TERMINATE;
	} else
	    handle_edit_command (&current, cmd);

	return retval;
    }

    if (strcasecmp (cmd, "xboard") == 0) {
	xboard = 1;
    } else if (strcasecmp (cmd, "edit") == 0) {
	init_edit_mode (&current);
	edit_mode = 1;
    } else if (strcasecmp (cmd, "setboard") == 0) {
#if 0
	/* This next line makes the unsafe assumption, that strtok
	   will only insert one single null-byte... */
	char* position = linebuf + 9;
	setboard (&current, position);
#else
	fprintf (stdout, "Error (currently not implemented): %s\n",
		 linebuf);
#endif
    } else if (strcasecmp (cmd, "force") == 0) {
	force = 1;
	// FIXME: Stop clocks.
	if (!force)
	    game_over = 0;
    } else if (strcasecmp (cmd, "go") == 0) {
	force = 0;
#if 0
	// FIXME: Check return value!
	go (&current);
#else
	fprintf (stdout, "Error (currently not implemented): %s\n",
		 linebuf);
#endif
    } else if (cmd[0] == '?' && cmd[1] == 0) {
	retval = EVENT_MOVE_NOW;
    } else if (strcasecmp (cmd, "draw") == 0) {
	fprintf (stdout, "offer draw\n");  /* Always happy.  */
    } else if (strcasecmp (cmd, "hint") == 0) {
	retval = EVENT_WANT_HINT;
    } else if (strcasecmp (cmd, "bk") == 0) {
	retval = EVENT_WANT_BOOK;
    } else if (strcasecmp (cmd, "undo") == 0) {
	fprintf (stdout, "Error (command not implemented): undo\n");
    } else if (strcasecmp (cmd, "remove") == 0) {
	fprintf (stdout, "Error (command not implemented): remove\n");
    } else if (strcasecmp (cmd, "hard") == 0) {
	retval = EVENT_PONDER;
    } else if (strcasecmp (cmd, "easy") == 0) {
	retval = EVENT_NO_PONDER;
    } else if (strcasecmp (cmd, "post") == 0) {
	retval = EVENT_POST;
    } else if (strcasecmp (cmd, "nopost") == 0) {
	retval = EVENT_UNPOST;
    } else if (strcasecmp (cmd, "analyze") == 0) {
	/* Ignore.  */
    } else if (strcasecmp (cmd, "name") == 0) {
	char* position = cmd + 5;

	if (name)
	    free (name);
	while (*position == ' ' || *position == '\t')
	    ++position;

	name = xstrdup (position);
    } else if (strcasecmp (cmd, "rating") == 0) {
	/* Ignore.  */
    } else if (strcasecmp (cmd, "ics") == 0) {
	ics = 1;
    } else if (strcasecmp (cmd, "computer") == 0) {
	    computer = 1;
    } else if (strcasecmp (cmd, "pause") == 0) {
	/* Ignore.  */
    } else if (strcasecmp (cmd, "resume") == 0) {
	/* Ignore.  */
    } else if (strcasecmp (cmd, "black") == 0) {
	game_over = 0;
	chi_to_move (&current) = chi_white;
	// FIXME: Stop clock, maybe stop thinking.
	return EVENT_STOP_THINKING;
    } else if (strcasecmp (cmd, "white") == 0) {
	game_over = 0;
	chi_to_move (&current) = chi_black;
	// FIXME: Stop clock, maybe stop thinking.
	return EVENT_STOP_THINKING;
    } else if (strcasecmp (cmd, "playother") == 0) {
	game_over = 0;
	// FIXME: Start pondering...
    } else if (strcasecmp (cmd, "time") == 0) {
	char* position = cmd + 5;

	if (my_time)
	    free (my_time);
	while (*position == ' ' || *position == '\t')
	    ++position;

	my_time = xstrdup (position);
	// FIXME: Adjust clock.
    } else if (strcasecmp (cmd, "otim") == 0) {
	char* position = cmd + 5;

	if (opp_time)
	    free (opp_time);
	while (*position == ' ' || *position == '\t')
	    ++position;

	opp_time = xstrdup (position);
	// FIXME: Adjust clock.
    } else if (strcasecmp (cmd, "level") == 0) {
	// FIXME: Set time controls.
    } else if (strcasecmp (cmd, "st") == 0) {
	// FIXME: Set time controls.
    } else if (strcasecmp (cmd, "sd") == 0) {
	// FIXME: Set depth.
    } else if (strcasecmp (cmd, "random") == 0) {
	/* Ignore.  */
    } else if (strcasecmp (cmd, "quit") == 0) {
	    return EVENT_TERMINATE;
    } else if (strcasecmp (cmd, "dump") == 0) {
	dump_board (&current);
    } else if (strcasecmp (cmd, "new") == 0) {
	game_over = 0;
	chi_init_position (&current);
	fprintf (stdout, "\n");
    } else if (strcasecmp (cmd, "display") == 0) {
	display_board (&current);
	fprintf (stdout, "\n");
    } else if (strcasecmp (cmd, "offsets") == 0) {
	display_offsets ();
	fprintf (stdout, "\n");
    } else if (strcasecmp (cmd, "moves") == 0) {
	display_moves ();
	fprintf (stdout, "\n");
    } else if (strcasecmp (cmd, "variant") == 0) {
	char* variant = strtok (NULL, WHITE_SPACE);
	if (variant != NULL && strcasecmp (variant, "normal") != 0) {
	    fprintf (stdout, "Error (variant not supported): %s\n", 
		     variant);
	}
    } else if (strcasecmp (cmd, "result") == 0) {
	char* result = strtok (NULL, WHITE_SPACE);
	if (result == NULL)
	    fprintf (stdout, "Error (usage error): result RESULT {COMMENT}\n");
	else if (strcmp (result, "1-0") != 0
		 && strcmp (result, "0-1") != 0
		 && strcmp (result, "1/2-1/2") != 0
		 && strcmp (result, "*") != 0)
	    fprintf (stdout, "Error (unrecognized result): %s\n",
		     result);
	else {
	    game_over = 1;
	}
	retval = EVENT_GAME_OVER;
    } else if (strcasecmp (cmd, "ping") == 0) {
	char* number = strtok (NULL, WHITE_SPACE);

	/* FIXME: Do not reply if thinking until thinking is done! */
	    if (number == NULL)
		fprintf (stdout, "Error (usage error): ping NUMBER\n");
	    else
		fprintf (stdout, "pong %s\n", number);
    } else if (strcasecmp (cmd, "accepted") == 0) {
	char* what = strtok (NULL, WHITE_SPACE);

	if (what == NULL)
	    fprintf (stdout, "Error (usage error): accepted FEATURE\n");
	else
	    handle_accepted (what);
	fprintf (stdout, "\n");
    } else if (strcasecmp (cmd, "rejected") == 0) {
	char* what = strtok (NULL, WHITE_SPACE);

	if (what == NULL) {
	    fprintf (stdout, "Error (usage error): rejected FEATURE");
	} else
	    handle_rejected (what);
	fprintf (stdout, "\n");
	
    } else if (strcasecmp (cmd, "protover") == 0) {
	char* protover_str = strtok (NULL, WHITE_SPACE);
	
	if (protover_str == NULL) {
	    fprintf (stdout, "Error (usage error): protover VERSION");
	} else if (strcmp (protover_str, "1") == 0) {
	    protover = 1;  /* Umh.  */
	} else if (strcmp (protover_str, "2") != 0) {
	    fprintf (stdout, "Error (unsupported protocol version): %s\n",
		     protover_str);
	}
	feature_requests ();
	fprintf (stdout, "\n");
    } else if (strcasecmp (cmd, "book") == 0) {
	create_book ("book.gdbm", 60, 3, 100);
    } else if (strcasecmp (cmd, "usermove") == 0) {
	char* movestr = strtok (NULL, WHITE_SPACE);
	
	retval = handle_usermove (movestr);
    } else {
	retval = handle_usermove (cmd);
    }

    if (!xboard)
	fputs (PROMPT, stdout); 

    fflush (stdout);

    return retval;
}

static void
feature_requests ()
{
    /* Send all features we would like to see.  */
    fprintf (stdout, "feature done=0\n");
    fprintf (stdout, "feature ping=1\n");
//    fprintf (stdout, "feature setboard=1\n");
    fprintf (stdout, "feature playother=1\n");
    fprintf (stdout, "feature san=1\n");
    fprintf (stdout, "feature san=0\n");
    fprintf (stdout, "feature usermove=1\n");
    fprintf (stdout, "feature time=1\n");
    fprintf (stdout, "feature draw=1\n");
    fprintf (stdout, "feature sigint=0\n");
    fprintf (stdout, "feature sigterm=0\n");
    fprintf (stdout, "feature reuse=1\n");
    fprintf (stdout, "feature analyze=0\n");
    fprintf (stdout, "feature myname=\"%s v%s\"\n", PACKAGE, VERSION);
    fprintf (stdout, "feature variants=\"normal\"\n");
    fprintf (stdout, "feature colors=0\n");
    fprintf (stdout, "feature ics=1\n");
    fprintf (stdout, "feature name=1\n");
//    fprintf (stdout, "feature pause=1\n");
    fprintf (stdout, "feature done=1\n");

}

static void
handle_accepted (feature)
    const char* feature;
{
    fprintf (stderr, "Interface accepted feature %s\n", feature);
}

static void
handle_rejected (feature)
    const char* feature;
{
    fprintf (stderr, "Interface rejected feature %s\n", feature);
}

RETSIGTYPE
sigio_handler (signum)
     int signum;
{
    input_available = event_pending = 1;

#if RETSIGTYPE != void
    return 0;
#endif
}

static void
display_offsets ()
{
    int file;
    int rank;

    fputs ("\n     a   b   c   d   e   f   g   h\n", stdout);
    fputs ("   +---+---+---+---+---+---+---+---+\n", stdout);

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	fputc (' ', stdout);
	fputc ('8' - 7 + rank, stdout);
	fputc (' ', stdout);

	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int direction;
	    int e4 = CHI_RANK_4 * 8 + CHI_FILE_E;
	    int index = rank * 8 + file;
	    int offset = index >= e4 ? index - e4 : e4 - index;

	    if (index < e4)
		direction = '>';
	    else if (index > e4)
		direction = '<';
	    else 
		direction = '=';

	    fprintf (stdout, "|%c%02d", direction, offset);
	}

	fputc ('|', stdout);
	fputc (' ', stdout);
	fputc ('8' - 7 + rank, stdout);
	fputs ("\n   +---+---+---+---+---+---+---+---+\n", stdout);
    }

    fputs ("     a   b   c   d   e   f   g   h\n\n", stdout);
}

static void
display_moves ()
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* mv;
    chi_move* mv_ptr;
    char* buf = NULL;
    unsigned int bufsize;

    mv = chi_generate_captures (&current, moves);
    mv = chi_generate_non_captures (&current, mv);

    for (mv_ptr = moves; mv_ptr < mv; ++mv_ptr) {
	chi_print_move (&current, *mv_ptr, &buf, &bufsize, 0);
	fprintf (stdout, "  Possible move: %s\n", buf);
    }

    if (buf)
	free (buf);
}

static int
handle_usermove (movestr)
     const char* movestr;
{
    chi_move move;
    int errnum;

    // FIXME: Return with an error if the engine is on move.

    if (!movestr)
	return EVENT_CONTINUE;

    errnum = chi_parse_move (&current, &move, movestr);
	
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
    } else {
	char* buf = NULL;
	unsigned int bufsize;

	int result = chi_print_move (&current, move, &buf, &bufsize, 0);
	
	if (result) {
	    fprintf (stderr, "Oops: %s\n", chi_strerror (result));
	} else {
	    fprintf (stdout, "  Got move: %s\n", buf);
	}

	if (buf)
	    free (buf);
    }

    return EVENT_CONTINUE;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
