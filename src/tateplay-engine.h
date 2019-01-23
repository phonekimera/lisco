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

#ifndef _ENGINE_H
# define _ENGINE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "stdbool.h"

typedef enum EngineProtocol {
	uci = 0,
#define uci uci
	xboard = 1,
#define xboard xboard
} EngineProtocol;

typedef enum EngineState {
	initial,
	started,
	initialize_protocol,
	acknowledged_protocol
} EngineState;

#ifdef __cplusplus
extern "C" {
#endif

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

	EngineProtocol protocol;
	EngineState state;

	char *outbuf;
	size_t outbuf_size;
	size_t outbuf_length;
	char *inbuf;
	size_t inbuf_size;
	size_t inbuf_length;

	struct timeval waiting_since;
	suseconds_t max_waiting_time;
} Engine;

extern Engine *engine_new();
extern void engine_destroy(Engine *engine);

/* Add to the engine's argument vector.  */
extern void engine_add_argv(Engine *self, const char *arg);

/* Set the engine protocol.  */
extern void engine_set_protocol(Engine *self, EngineProtocol protocol);

/* Start the engine.  */
extern bool engine_start(Engine *self);

/* Initialize the protocol.  */
extern void engine_init_protocol(Engine *self);

/* Read from the engine's standard output and parse it.  */
extern bool engine_read_stdout(Engine *self);

/* Read from the engine's standard error and log it.  */
extern bool engine_read_stderr(Engine *self);

/* Write commands to the engine's standard input.  */
extern bool engine_write_stdin(Engine *self);

#ifdef __cplusplus
}
#endif

#endif
