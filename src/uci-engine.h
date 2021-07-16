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

#ifndef _UCI_ENGINE_H
# define _UCI_ENGINE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#define UCI_ENGINE_MAX_THREADS 512

typedef struct UCIEngineOptions {
	int debug;
	int option_threads;
	FILE *in;
	const char *inname;
	FILE *out;
	const char *outname;
} UCIEngineOptions;

#ifdef __cplusplus
extern "C" {
#endif

extern void uci_init(UCIEngineOptions* options,
		FILE *in, const char *inname,
		FILE *out, const char *outname);
extern int uci_main(UCIEngineOptions *options);

#ifdef TEST_UCI_ENGINE
extern int uci_handle_quit(UCIEngineOptions *options);
extern int uci_handle_uci(UCIEngineOptions *options, char *args, FILE *out);
extern int uci_handle_debug(UCIEngineOptions *options, char *args, FILE *out);
extern int uci_handle_position(UCIEngineOptions *options, char *args, FILE *out);
extern int uci_handle_go(UCIEngineOptions *options, char *args, FILE *out);
#endif

#ifdef __cplusplus
}
#endif

#endif
