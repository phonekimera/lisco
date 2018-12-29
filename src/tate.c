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
#include "brain.h"
#include "time_ctrl.h"

static void greeting PARAMS ((void));
static void feature_requests PARAMS ((void));
static void handle_accepted PARAMS ((const char* feature));
static void handle_rejected PARAMS ((const char* feature));
static int handle_usermove PARAMS ((const char* movestr));
static int handle_legalmove PARAMS ((const char* movestr));
static int handle_hashmove PARAMS ((void));
static int handle_evaluate PARAMS ((const char* movestr));
static int handle_go PARAMS ((void));
static int handle_setboard PARAMS ((const char* fen));
static void display_offsets PARAMS ((void));
static void display_moves PARAMS ((void));
static RETSIGTYPE sigio_handler PARAMS ((int));
static RETSIGTYPE sigxboard_handler PARAMS ((int));
static void check_input PARAMS ((void));

#define WHITE_SPACE " \t\r\n\v\f"

chi_pos current;
chi_zk_handle zk_handle;

int protover = 2;

char* name = NULL;
int xboard = 0;
int force = 0;
int game_over = 0;
int thinking = 0;
int pondering = 0;
int allow_pondering = 1;
int ics = 0;
int computer = 0;
chi_color_t engine_color = chi_black;

int input_available = 0;
int event_pending   = 0;
int max_ply = MAX_PLY;

int mate_announce = 0;

int post = 1;

static int edit_mode = 0;

struct game_hist_entry* game_hist = NULL;
unsigned int game_hist_ply = 0;

static unsigned int game_hist_alloc = 0;

#define PROMPT (edit_mode ? \
		((chi_on_move (&current) == chi_white ? \
		  "tate[white]# " : "tate[black]# ")) : \
		((chi_on_move (&current) == chi_white ? \
		  "tate[white]> " : "tate[black]> ")))

int
main (argc, argv)
    int argc;
    char* argv[];
{
    int flags;
    int errnum;

    /* Standard input must be unbuffered, because we mix it with
       low-level I/O (which is of course a sin).  */
    setvbuf (stdin, NULL, _IONBF, 0);
    setvbuf (stdout, NULL, _IOLBF, 0);
    setvbuf (stderr, NULL, _IOLBF, 0);

    greeting ();

    chi_init_position (&current);
    errnum = chi_zk_init (&zk_handle);
    if (errnum)
	error (EXIT_FAILURE, 0, "Cannot initialize Zobrist key array: %s",
	       chi_strerror (errnum));

    game_hist_alloc = MAX_PLY;
    game_hist = xmalloc (game_hist_alloc * sizeof *game_hist);
    game_hist[0].pos = current;
    game_hist[0].signature = chi_zk_signature (zk_handle, &current);
    game_hist[0].castling_state = 0x33;

    /* Use minimum size.  */
    init_tt_hashs (0);

    if (signal (SIGIO, sigio_handler) != 0)
	error (EXIT_FAILURE, errno, "Cannot install SIGIO handler");

    fcntl (fileno (stdin), F_SETOWN, getpid ());

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
	check_input ();
	while (event_pending) {
	    int status = get_event ();
	    if (status & EVENT_TERMINATE)
		return EXIT_SUCCESS;
	}
	sleep (1);
    }

    return EXIT_SUCCESS;
}

static void
greeting ()
{
    fprintf (stdout, "\
%s %s (libchi %s)\n\
Copyright (C) 2002, Guido Flohr <guido@imperia.net>.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
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
    if (feof (stdin))
	error (EXIT_SUCCESS, errno, "stdin eof");
    if (feof (stdout))
	error (EXIT_SUCCESS, errno, "stdout eof");
    if (feof (stderr))
	error (EXIT_SUCCESS, errno, "stderr eof");
    if (ferror (stdin))
	error (EXIT_SUCCESS, errno, "stdin error");
    if (ferror (stdout))
	error (EXIT_SUCCESS, errno, "stdout error");
    if (ferror (stderr))
	error (EXIT_SUCCESS, errno, "stderr error");

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
	if (cmd [0] == '.' && cmd[1] == 0) {
	    edit_mode = check_position (&current, 0);
	    if (!edit_mode) {
		int castling_state = 0;

		game_hist_ply = 0;
		game_hist[0].pos = current;
		game_hist[0].signature = chi_zk_signature (zk_handle, 
							   &current);
		
		if (chi_wk_castle (&current))
		    castling_state |= 0x1;
		if (chi_wq_castle (&current))
		    castling_state |= 0x2;
		if (chi_bk_castle (&current))
		    castling_state |= 0x10;
		if (chi_bq_castle (&current))
		    castling_state |= 0x20;
		game_hist[0].castling_state = castling_state;
	    }
	} else if (strcasecmp (cmd, "display") == 0) {
	    /* Convenience.  */
	    display_board (&current);
	    fprintf (stdout, "\n");
	} else if (strcasecmp (cmd, "quit") == 0) {
	    /* Convenience, too.  */
	    return EVENT_TERMINATE;
	} else
	    handle_edit_command (&current, cmd);
    } else if (strcasecmp (cmd, "xboard") == 0) {
	xboard = 1;
	if (signal (SIGINT, sigxboard_handler) != 0)
	    error (EXIT_SUCCESS, errno, "Cannot install SIGINT handler");
	if (signal (SIGTERM, sigxboard_handler) != 0)
	    error (EXIT_SUCCESS, errno, "Cannot install SIGTERM handler");
    } else if (strcasecmp (cmd, "edit") == 0) {
	init_edit_mode (&current);
	edit_mode = 1;
    } else if (strcasecmp (cmd, "setboard") == 0) {
	/* This next line makes the unsafe assumption, that strtok
	   will only insert one single null-byte... */
	char* position = linebuf + 9;
	handle_setboard (position);
    } else if (strcasecmp (cmd, "force") == 0) {
	force = 1;
	// FIXME: Stop clocks.
	if (!force)
	    game_over = 0;
    } else if (strcasecmp (cmd, "go") == 0) {
	retval = handle_go ();
    } else if (cmd[0] == '?' && cmd[1] == 0) {
	retval = EVENT_MOVE_NOW;
    } else if (strcasecmp (cmd, "draw") == 0) {
	// FIXME: Keep global score and react.
    } else if (strcasecmp (cmd, "hint") == 0) {
	retval = EVENT_WANT_HINT;
    } else if (strcasecmp (cmd, "bk") == 0) {
	retval = EVENT_WANT_BOOK;
    } else if (strcasecmp (cmd, "undo") == 0) {
	fprintf (stdout, "Error (command not implemented): undo\n");
    } else if (strcasecmp (cmd, "remove") == 0) {
	fprintf (stdout, "Error (command not implemented): remove\n");
    } else if (strcasecmp (cmd, "hard") == 0) {
	// FIXME: Start pondering if not on move.
	retval = EVENT_PONDER;
    } else if (strcasecmp (cmd, "easy") == 0) {
	retval = EVENT_NO_PONDER;
    } else if (strcasecmp (cmd, "post") == 0) {
	post = 1;
    } else if (strcasecmp (cmd, "nopost") == 0) {
	post = 0;
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
	chi_on_move (&current) = chi_black;
	engine_color = chi_white;
	// FIXME: Stop clock, maybe stop thinking.
	retval = EVENT_STOP_THINKING;
    } else if (strcasecmp (cmd, "white") == 0) {
	game_over = 0;
	chi_on_move (&current) = chi_white;
	engine_color = chi_black;
	// FIXME: Stop clock, maybe stop thinking.
	retval = EVENT_STOP_THINKING;
    } else if (strcasecmp (cmd, "playother") == 0) {
	game_over = 0;
	// FIXME: Start pondering...
    } else if (strcasecmp (cmd, "time") == 0) {
	char* position = cmd + 5;
	char* end_ptr;
	long int parsed = strtol (position, &end_ptr, 10);
	
	if (parsed == 0 && end_ptr == position) {
	    fprintf (stdout, "error (illegal time specification): %s",
		     position);
	} else {
	    time_left = parsed;
	}
    } else if (strcasecmp (cmd, "otim") == 0) {
	char* position = cmd + 5;
	char* end_ptr;
	long int parsed = strtol (position, &end_ptr, 10);
	
	if (parsed == 0 && end_ptr == position) {
	    fprintf (stdout, "error (illegal time specification): %s",
		     position);
	} else {
	    opp_time = parsed;
	}
    } else if (strcasecmp (cmd, "level") == 0) {
	parse_level (cmd + 6);
    } else if (strcasecmp (cmd, "st") == 0) {
	char* fixed_str = cmd + 3;
	char* end_ptr;
	long int parsed = strtol (fixed_str, &end_ptr, 10);
	
	if (parsed == 0 && end_ptr == fixed_str) {
	    fprintf (stdout, "error (illegal search time): %s",
		     fixed_str);
	} else {
	    fixed_time = 100 * parsed;
	}
    } else if (strcasecmp (cmd, "sd") == 0) {
#if 0
	char* depth_str = cmd + 3;
	char* end_ptr;
	long int parsed = strtol (depth_str, &end_ptr, 10);

	if (parsed == 0 && end_ptr == depth_str) {
	    fprintf (stdout, "error (illegal search depth): %s",
		     depth_str);
	} else {
	    max_ply = parsed;
	}
#endif
    } else if (strcasecmp (cmd, "random") == 0) {
	/* Ignore.  */
    } else if (strcasecmp (cmd, "quit") == 0) {
	    return EVENT_TERMINATE;
    } else if (strcasecmp (cmd, "dump") == 0) {
	dump_board (&current);
    } else if (strcasecmp (cmd, "print") == 0) {
	print_game ();
    } else if (strcasecmp (cmd, "new") == 0) {
	if (xboard) {
	    fprintf (stdout, "tellics set 1 %s %s (libchi %s)\n",
		     PACKAGE, VERSION, CHI_VERSION);
	}

	mate_announce = 0;
	game_over = 0;
	force = 0;
	go_fast = 0;
	chi_init_position (&current);
	engine_color = chi_black;
	game_hist_ply = 0;
	game_hist[0].pos = current;
	game_hist[0].signature = chi_zk_signature (zk_handle, &current);
	game_hist[0].castling_state = 0x33;
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
    } else if (strcasecmp (cmd, "legalmove") == 0) {
	char* movestr = strtok (NULL, WHITE_SPACE);

	retval = handle_legalmove (movestr);
    } else if (strcasecmp (cmd, "evaluate") == 0) {
	char* movestr = strtok (NULL, WHITE_SPACE);

	retval = handle_evaluate (movestr);
    } else if (strcasecmp (cmd, "hashmove") == 0) {
	retval = handle_hashmove ();
    } else {
	retval = handle_usermove (cmd);
    }

    if (!xboard)
	fputs (PROMPT, stdout); 

    fflush (stdout);

    check_input ();

    return retval;
}

static void
feature_requests ()
{
    /* Send all features we would like to see.  */
    fprintf (stdout, "feature done=0\n");
    fprintf (stdout, "feature ping=1\n");
    fprintf (stdout, "feature setboard=1\n");
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
    fprintf (stdout, "Interface accepted feature %s\n", feature);
}

static void
handle_rejected (feature)
    const char* feature;
{
    fprintf (stdout, "Interface rejected feature %s\n", feature);
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

RETSIGTYPE
sigxboard_handler (signum)
     int signum;
{
    error (EXIT_FAILURE, 0, "engine received signal %d\n", signum);
    
#if RETSIGTYPE != void
    return 0;
#endif
}

static void
display_offsets ()
{
    int file;
    int rank;

    fputs ("\n     a   b   c   d   e   f   g   h", stdout);
    fputs ("   ", stdout);
    fputs ("    a   b   c   d   e   f   g   h\n", stdout);

    fputs ("   +---+---+---+---+---+---+---+---+", stdout);
    fputs ("   ", stdout);
    fputs ("+---+---+---+---+---+---+---+---+\n", stdout);

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	fputc (' ', stdout);
	fputc ('8' - 7 + rank, stdout);
	fputc (' ', stdout);

	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int offset = chi_coords2shift (file, rank);

	    fprintf (stdout, "| %02d", offset);
	}

	fputc ('|', stdout);
	fputs ("   ", stdout);

	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int offset = chi_coords2shift90 (file, rank);

	    fprintf (stdout, "| %02d", offset);
	}

	fputc ('|', stdout);
	fputc (' ', stdout);

	fputc ('8' - 7 + rank, stdout);
	fputs ("\n   +---+---+---+---+---+---+---+---+", stdout);
	fputs ("   ", stdout);
	fputs ("+---+---+---+---+---+---+---+---+\n", stdout);
    }

    fputs ("     a   b   c   d   e   f   g   h", stdout);
    fputs ("   ", stdout);
    fputs ("    a   b   c   d   e   f   g   h\n\n", stdout);
}

static void
display_moves ()
{
    chi_move moves[CHI_MAX_MOVES];
    chi_move* mv;
    chi_move* mv_ptr;
    static char* buf = NULL;
    static unsigned int bufsize;

    mv = chi_legal_moves (&current, moves);

    for (mv_ptr = moves; mv_ptr < mv; ++mv_ptr) {
	chi_print_move (&current, *mv_ptr, &buf, &bufsize, 0);
	fprintf (stdout, "  Possible move: %s ", buf);

	fprintf (stdout, "[%08x:", *mv_ptr);
	fprintf (stdout, "%d:", chi_move_material (*mv_ptr));
	fprintf (stdout, "%s:", chi_move_is_ep (*mv_ptr) ? "ep" : "-");
	
	switch (chi_move_promote (*mv_ptr)) {
	    case knight:
		fprintf (stdout, "=N:");
		break;
	    case bishop:
		fprintf (stdout, "=B:");
		break;
	    case rook:
		fprintf (stdout, "=R:");
		break;
	    case queen:
		fprintf (stdout, "=Q:");
		break;
	    case empty:
		fprintf (stdout, "-:");
		break;
	    default:
		fprintf (stdout, "=?%d:", chi_move_promote (*mv_ptr));
		break;
	}
	
	switch (chi_move_victim (*mv_ptr)) {
	    case pawn:
		fprintf (stdout, "xP:");
		break;
	    case knight:
		fprintf (stdout, "xN:");
		break;
	case bishop:
	    fprintf (stdout, "xB:");
	    break;
	    case rook:
		fprintf (stdout, "xR:");
		break;
	    case queen:
		fprintf (stdout, "xQ:");
		break;
	    case empty:
		fprintf (stdout, "-:");
		break;
	    default:
		fprintf (stdout, "x?%d:", chi_move_victim (*mv_ptr));
		break;
	}
	
	switch (chi_move_attacker (*mv_ptr)) {
	    case pawn:
		fprintf (stdout, "P");
		break;
	    case knight:
		fprintf (stdout, "N");
		break;
	    case bishop:
		fprintf (stdout, "B");
		break;
	    case rook:
		fprintf (stdout, "R");
		break;
	    case queen:
		fprintf (stdout, "Q");
		break;
	    case king:
		fprintf (stdout, "K");
		break;
	}
	
	fprintf (stdout, "]\n");
	
    }
}

static int
handle_usermove (movestr)
     const char* movestr;
{
    chi_move move;
    int errnum;
    int castling_state;

    if (game_over) {
	fprintf (stdout, "Error (game over): %s\n", movestr);
	return EVENT_CONTINUE;
    }

    if (thinking) {
	fprintf (stdout, "Error (it's not your move): %s\n",
		 movestr);
	return EVENT_CONTINUE;
    }

    if (!movestr)
	return EVENT_CONTINUE;

    errnum = chi_parse_move (&current, &move, movestr);
	
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    errnum = chi_check_legality (&current, move);
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    errnum = chi_apply_move (&current, move);

    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    if (game_hist_ply + 1 >= game_hist_alloc) {
	game_hist = xrealloc (game_hist, game_hist_ply + MAX_PLY);
	game_hist_ply += MAX_PLY;
    }

    game_hist[game_hist_ply++].move = move;
    game_hist[game_hist_ply].signature = chi_zk_signature (zk_handle, 
							   &current);
    castling_state = game_hist[game_hist_ply - 1].castling_state;
    if (!chi_wk_castle (&current))
	castling_state &= ~0x1;
    if (!chi_wq_castle (&current))
	castling_state &= ~0x2;

    if (!chi_bk_castle (&current))
	castling_state &= ~0x10;
    if (!chi_bq_castle (&current))
	castling_state &= ~0x20;

    if ((0xfff & move) == (3 | (1 << 6))) {
	castling_state &= 0x70;
	castling_state |= 0x4;
    } else if ((0xfff & move) == (3 | (5 << 6))) {
	castling_state &= 0x70;
	castling_state |= 0x4;
    } else if ((0xfff & move) == (59 | (57 << 6))) {
	castling_state &= 0x7;
	castling_state |= 0x40;
    } else if ((0xfff & move) == (59 | (61 << 6))) {
	castling_state &= 0x7;
	castling_state |= 0x40;
    }
    game_hist[game_hist_ply].castling_state = castling_state;

    if (1) {
	bitv64 new_signature = game_hist[game_hist_ply].signature;
	bitv64 old_signature = game_hist[game_hist_ply - 1].signature;
	chi_color_t color = chi_on_move (&game_hist[game_hist_ply - 1].pos);
	
	if (new_signature != chi_zk_update_signature (zk_handle, 
						      old_signature, 
						      move, color)) {
	    fprintf (stderr, "new sig: %016llx\n", new_signature);
	    fprintf (stderr, "old sig: %016llx\n", old_signature);
	    fprintf (stderr, "upd sig: %016llx\n",
		     chi_zk_update_signature (zk_handle, old_signature,
					      move, color));
	    fprintf (stderr, "xpd sig: %016llx\n",
	         chi_zk_update_signature (zk_handle, old_signature,
	    			      move, !color));
	    error (EXIT_FAILURE, 0, "usermove: signatures mismatch");
	}						    
    }

    game_hist[game_hist_ply].pos = current;

    if (force)
	return EVENT_CONTINUE;

    engine_color = !engine_color;
    thinking = 1;

    return handle_go ();
}

static int
handle_legalmove (movestr)
     const char* movestr;
{
    chi_move move;
    int errnum;
    chi_pos dummy_pos = current;

    if (game_over) {
	fprintf (stdout, "Error (game over): %s\n", movestr);
	return EVENT_CONTINUE;
    }

    if (!movestr)
	return EVENT_CONTINUE;

    errnum = chi_parse_move (&dummy_pos, &move, movestr);	
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    errnum = chi_illegal_move (&dummy_pos, move, 1);
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    fprintf (stdout, "  Move %s is legal\n", movestr);
    
    return EVENT_CONTINUE;
}

static int
handle_evaluate (movestr)
     const char* movestr;
{
    chi_move move;
    int errnum;
    chi_pos dummy_pos = current;

    if (game_over) {
	fprintf (stdout, "Error (game over): %s\n", movestr);
	return EVENT_CONTINUE;
    }

    if (!movestr)
	return EVENT_CONTINUE;

    errnum = chi_parse_move (&current, &move, movestr);	
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    errnum = chi_illegal_move (&dummy_pos, move, 1);
    if (errnum != 0) {
	fprintf (stdout, "Illegal move (%s): %s\n",
		 chi_strerror (errnum), movestr);
	return EVENT_CONTINUE;
    }

    evaluate_move (move);
    
    return EVENT_CONTINUE;
}

static int
handle_hashmove ()
{
    chi_move move;
    bitv64 signature;
    int alpha = +INF;
    int beta = -INF;
    int probe;

    if (game_over) {
	fprintf (stdout, "Error (game over): hashmove\n");
	return EVENT_CONTINUE;
    }

    signature = chi_zk_signature (zk_handle, &current);
    move = best_tt_move (&current, signature);
    probe = probe_tt (&current, signature, 0, &alpha, &beta);

    if (move) {
	fprintf (stdout, "  Hashed best move: %s%c%s",
		 chi_shift2label (chi_move_from (move)),
		 chi_move_victim (move) ? 'x' : '-',
		 chi_shift2label (chi_move_to (move)));
	if (chi_move_promote (move)) {
	    switch (chi_move_promote (move)) {
		case knight:
		    fprintf (stdout, "=N");
		    break;
		case bishop:
		    fprintf (stdout, "=B");
		    break;
		case rook:
		    fprintf (stdout, "=R");
		    break;
		case queen:
		    fprintf (stdout, "=Q");
		    break;
		default:
		    fprintf (stdout, "=?");
		    break;
	    }
	}
	fputc ('\n', stdout);
    } else {
	fprintf (stdout, "  No best move!\n");
    }

    switch (probe) {
	case HASH_EXACT:
	    fprintf (stdout, "  HASH_EXACT: %d\n", alpha);
	    break;
	case HASH_ALPHA:
	    fprintf (stdout, "  HASH_ALPHA: %d\n", alpha);
	    break;
	case HASH_BETA:
	    fprintf (stdout, "  HASH_BETA: %d\n", beta);
	    break;
	default:
	    fprintf (stdout, "  HASH_UNKNOWN\n");
	    break;
    }

    return EVENT_CONTINUE;
}

static int
handle_go ()
{
    chi_move move;
    int errnum;
    static char* buf = NULL;
    static unsigned int bufsize;
    int result;
    int castling_state;

    force = 0;

    if (game_over)
	return EVENT_GAME_OVER;

    engine_color = chi_on_move (&current);

    result = think (&move);
    if (result & EVENT_GAME_OVER)
	return result;
    
    errnum = chi_print_move (&current, move, &buf, &bufsize, 0);
    if (errnum != 0) {
	print_game (); fflush (stdout);
	display_moves (); fflush (stdout);
	dump_board (&current);
	fprintf (stderr, "engine move: ");

	fprintf (stderr, "[%08x:", move);
	fprintf (stderr, "%d:", chi_move_material (move));
	fprintf (stderr, "%s:", chi_move_is_ep (move) ? "ep" : "-");
	
	switch (chi_move_promote (move)) {
	    case knight:
		fprintf (stderr, "=N:");
		break;
	    case bishop:
		fprintf (stderr, "=B:");
		break;
	    case rook:
		fprintf (stderr, "=R:");
		break;
	    case queen:
		fprintf (stderr, "=Q:");
		break;
	    case empty:
		fprintf (stderr, "-:");
		break;
	    default:
		fprintf (stderr, "=?%d:", chi_move_promote (move));
		break;
	}
	
	switch (chi_move_victim (move)) {
	    case pawn:
		fprintf (stderr, "xP:");
		break;
	    case knight:
		fprintf (stderr, "xN:");
		break;
	    case bishop:
		fprintf (stderr, "xB:");
		break;
	    case rook:
		fprintf (stderr, "xR:");
		break;
	    case queen:
		fprintf (stderr, "xQ:");
		break;
	    case empty:
		fprintf (stderr, "-:");
		break;
	    default:
		fprintf (stderr, "x?%d:", chi_move_victim (move));
		break;
	}
	
	switch (chi_move_attacker (move)) {
	    case pawn:
		fprintf (stderr, "P");
		break;
	    case knight:
		fprintf (stderr, "N");
		break;
	    case bishop:
		fprintf (stderr, "B");
		break;
	    case rook:
		fprintf (stderr, "R");
		break;
	    case queen:
		fprintf (stderr, "Q");
		break;
	    case king:
		fprintf (stderr, "K");
		break;
	}
	
	fprintf (stderr, "]\n");
	fprintf (stderr, "Passed extended chi_illegal_move: %s\n",
		 chi_illegal_move (&current, move, 1) ? "no" : "yes");
	error (EXIT_FAILURE, 0, "engine made illegal move (%s-%s).\n",
	       chi_shift2label (chi_move_from (move)), 
	       chi_shift2label (chi_move_to (move)));
    }

    fprintf (stdout, "move %s\n", buf);

    errnum = chi_apply_move (&current, move);
    if (errnum != 0)
	error (EXIT_FAILURE, 0, "engine made illegal move.\n");

    if (game_hist_ply + 1 >= game_hist_alloc) {
	game_hist = xrealloc (game_hist, game_hist_ply + MAX_PLY);
	game_hist_ply += MAX_PLY;
    }

    if (result == EVENT_GAME_OVER)
	return EVENT_GAME_OVER;

    game_hist[game_hist_ply++].move = move;
    game_hist[game_hist_ply].signature = chi_zk_signature (zk_handle, 
							   &current);
    game_hist[game_hist_ply].pos = current;
    castling_state = game_hist[game_hist_ply - 1].castling_state;
    if (!chi_wk_castle (&current))
	castling_state &= ~0x1;
    if (!chi_wq_castle (&current))
	castling_state &= ~0x2;

    if (!chi_bk_castle (&current))
	castling_state &= ~0x10;
    if (!chi_bq_castle (&current))
	castling_state &= ~0x20;

    if ((0xfff & move) == (3 | (1 << 6))) {
	castling_state &= 0x70;
	castling_state |= 0x4;
    } else if ((0xfff & move) == (3 | (5 << 6))) {
	castling_state &= 0x70;
	castling_state |= 0x4;
    } else if ((0xfff & move) == (59 | (57 << 6))) {
	castling_state &= 0x7;
	castling_state |= 0x40;
    } else if ((0xfff & move) == (59 | (61 << 6))) {
	castling_state &= 0x7;
	castling_state |= 0x40;
    }
    game_hist[game_hist_ply].castling_state = castling_state;

    if (1) {
	bitv64 new_signature = game_hist[game_hist_ply].signature;
	bitv64 old_signature = game_hist[game_hist_ply - 1].signature;
	chi_color_t color = !chi_on_move (&current);
	
	if (new_signature != chi_zk_update_signature (zk_handle, 
						      old_signature, 
						      move, color)) {
	    print_game ();
	    fflush (stdout);
	    fprintf (stderr, "new sig: %016llx\n", new_signature);
	    fprintf (stderr, "old sig: %016llx\n", old_signature);
	    fprintf (stderr, "upd sig: %016llx\n",
		     chi_zk_update_signature (zk_handle, old_signature,
					      move, color));
	    fprintf (stderr, "xpd sig: %016llx\n",
	         chi_zk_update_signature (zk_handle, old_signature,
	    			      move, !color));

	    error (EXIT_FAILURE, 0, "go: signatures mismatch");
	}
    }

    engine_color = !engine_color;
    thinking = 0;

    // FIXME: Start pondering.

    return EVENT_CONTINUE;
}

static int
handle_setboard (fen)
     const char* fen;
{
    chi_pos new_pos;
    int errnum = chi_set_position (&new_pos, fen);

    if (errnum) {
	fprintf (stdout, "Error (%s): %s\n",
		 chi_strerror (errnum), fen);
	return EVENT_CONTINUE;
    }

    current = new_pos;

    mate_announce = 0;
    game_over = 0;
    force = 0;
    go_fast = 0;
    engine_color = !chi_on_move (&current);
    game_hist_ply = 0;
    game_hist[0].pos = current;
    game_hist[0].signature = chi_zk_signature (zk_handle, &current);

    game_hist[0].castling_state = 0;
    if (chi_wk_castle (&current))
	game_hist[0].castling_state |= 0x1;
    if (chi_wq_castle (&current))
	game_hist[0].castling_state |= 0x2;
    if (chi_bk_castle (&current))
	game_hist[0].castling_state |= 0x10;
    if (chi_bq_castle (&current))
	game_hist[0].castling_state |= 0x20;

    return EVENT_CONTINUE;
}

static void
check_input ()
{
    fd_set mask;
    struct timeval timeout;
    int result;

    while (1) {
	memset (&timeout, 0, sizeof timeout);
	FD_ZERO (&mask);
	FD_SET (fileno (stdin), &mask);
	
	result = select (1, &mask, NULL, NULL, &timeout);
#ifdef EINTR
	if (result == -1 && errno == EINTR)
	    continue;
#endif
	if (result == -1)
	    error (EXIT_FAILURE, errno, "select failed");
	if (result != 0) {
	    event_pending = 1;
	    raise (SIGIO);
	}
	break;
    }
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
