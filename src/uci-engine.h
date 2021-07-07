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

#ifndef UCI_ENGINE_H
# define UCI_ENGINE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

typedef struct UCIEngineOptions {
} UCIEngineOptions;

extern void uci_init(UCIEngineOptions* options);
extern int uci_main(UCIEngineOptions *options,
                    FILE *in, const char *inname,
                    FILE *out, const char *outname);

#ifdef TEST_UCI_ENGINE
# define uci_engine_extern extern
uci_engine_extern uci_engine_extern int handle_quit(const char *arguments);
#else
# define uci_engine_extern static
#endif

#endif
