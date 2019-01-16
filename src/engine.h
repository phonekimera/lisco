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

typedef enum {
	uci = 0,
#define uci uci
	xboard = 1,
#define xboard xboard
} EngineProtocol;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Engine {
	/* The command-line options to start the engine.  */
	char **argv;
	size_t _argv_size;

	EngineProtocol protocol;
} Engine;

extern Engine *engine_new();
extern void engine_destroy(Engine *engine);

/* Add to the engine's argument vector.  */
extern void engine_add_argv(Engine *self, const char *arg);

/* Set the engine protocol.  */
extern void engine_set_protocol(Engine *self, EngineProtocol protocol);

#ifdef __cplusplus
}
#endif

#endif
