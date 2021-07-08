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

#ifndef _STATE_H
# define _STATE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#include "uci-engine.h"

typedef struct Tate {
	// The current UCI options and state.
	UCIEngineOptions uci;

	// The current position.
	chi_pos position;

	// The move found.
	chi_move bestmove;

	// Non-zero if a valid move had been found.
	int bestmove_found;

	// Move that the engine would like to ponder on.
	chi_move pondermove;

	// Non-zero if a valid move to ponder upon had been found.
	int pondermove_found;

	chi_zk_handle zk_handle;
} Tate;

#ifdef __cplusplus
extern "C" {
#endif

extern Tate tate;

#ifdef __cplusplus
}
#endif

#endif
