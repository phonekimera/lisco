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

#ifndef _ENGINE_H
# define _ENGINE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "stdbool.h"

#include "tateplay-option.h"
#include "tateplay-time-control.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum EngineProtocol {
	uci = 0,
#define uci uci
	xboard = 1,
#define xboard xboard
} EngineProtocol;

typedef enum EngineState {
	initial,
	started,
	negotiating,
	acknowledged,
	configuring,
	ready,
	thinking,
	pondering,
	finished
} EngineState;

typedef struct UserOption {
	char *name;
	char *value;
} UserOption;

struct Game;

typedef struct Engine {
	/* The command-line options to start the engine.  */
	char **argv;
	size_t _argv_size;

	/* The engine's nick name.  */
	char *nick;

	/* The running engine's process id.  */
	pid_t pid;

	/* The running engine's standard file descriptors.  */
	int in;
	int out;
	int err;

	/* The game that the engine is taking place in.  */
	struct Game *game;

	EngineProtocol protocol;
	EngineState state;

	long delay;
	struct timeval start_output;

	char *outbuf;
	size_t outbuf_size;
	size_t outbuf_length;

	char *inbuf;
	size_t inbuf_size;
	size_t inbuf_length;

	void (*out_callback)(struct Engine *self);

	struct timeval ready;

	/* Common negotiable features.  */
	
	/* Send moves as SAN instead of
	 * coordinate notation.  Actually only possible for xboard.
	 */
	chi_bool san;

	/* Number of cpus to use.  O means maximum.  */
	size_t num_cpus;

	/* Memory usage (hash size).  Defaults to 1 GB.  */
	size_t mem_usage;

	/* Maximum search depth in plies.  0 means, search to infinite depth.  */
	unsigned long depth;

	/* Time control.  */
	TimeControl tc;

    /* Turn on pondering.  */
    chi_bool ponder;

	/* Options set by the user via CLI --option-COLOR.  */
	UserOption *user_options;
	size_t num_user_options;

	/* Negotiatable xboard features.  */
	chi_bool xboard_usermove;
	chi_bool xboard_time;
	chi_bool xboard_memory;
	chi_bool xboard_smp;
	chi_bool xboard_colors;
	/* FIXME! EGT formats! */

	/* Engine options.  */
	Option **options;
	size_t num_options;
} Engine;

extern Engine *engine_new(struct Game *game);
extern void engine_destroy(Engine *engine);

/* Add to the engine's argument vector.  */
extern void engine_add_argv(Engine *self, const char *arg);

/* Add an option set by the user via CLI for the configuration phase.  */
extern void engine_set_option(Engine *self, char *name, char *value);

/* Set the engine protocol.  */
extern void engine_set_protocol(Engine *self, EngineProtocol protocol);

/* Start the engine.  */
extern bool engine_start(Engine *self);

/* Initialize the protocol.  */
extern void engine_negotiate(Engine *self);

/* Read from the engine's standard output and parse it.  */
extern bool engine_read_stdout(Engine *self);

/* Read from the engine's standard error and log it.  */
extern bool engine_read_stderr(Engine *self);

/* Write commands to the engine's standard input.  */
extern bool engine_write_stdin(Engine *self);

/* Send configuration commands to the engine.  */
extern bool engine_configure(Engine *self);

/* Find the post move for pos, after move has been made.  If move is 0,
 * pos is the starting position.
 */
extern void engine_think(Engine *self, chi_pos *pos, chi_move move);

/* Try to turn on ponder aka hard mode.  */
extern void engine_turn_on_ponder(Engine *self);

/* Ponder on position if pondering is enabled.  */
extern void engine_ponder(Engine *self, chi_pos *pos);

/* Stop the clocks engine.  Starting the clock is done, after a move has been
 * successfully sent.  Returns false, if the engine's flag had fallen.
 */
extern chi_bool engine_stop_clock(Engine *self);

#ifdef __cplusplus
}
#endif

#endif
