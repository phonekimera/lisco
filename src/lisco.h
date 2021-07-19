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

#ifndef LISCO_H
# define LISCO_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#include "rtime.h"
#include "uci-engine.h"

#define LISCO_DEFAULT_TT_SIZE 16

#define MATE -10000
#define INF ((-(MATE)) << 1)
#define MAX_PLY 512

typedef struct Lisco {
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
} Lisco;

typedef struct Line {
	chi_move moves[MAX_PLY];
	unsigned int num_moves;
} Line;

typedef struct Tree {
	bitv64 signatures[MAX_PLY + 1];

	chi_pos position;
	chi_move bestmove;
	int score;
	int depth;
	unsigned long long nodes;
	unsigned long long evals;

	Line line;

	rtime_t start_time;
	unsigned long long int nodes_to_tc;
	long long int fixed_time;
	int move_now;

	chi_move hash_move[MAX_PLY];

	unsigned long long tt_probes;
	unsigned long long tt_hits;
	unsigned long long ev_hits;
	unsigned long long lazy_evals;
} Tree;

typedef struct MoveSelector {
	chi_move moves[CHI_MAX_MOVES];
	size_t num_moves;
	size_t selected;
} MoveSelector;

#ifdef __cplusplus
extern "C" {
#endif

extern Lisco lisco;

// Main transposition table.

/* Create a new transposition table of approximatel SIZE bytes and destroy
 * an old one.
 */
extern void tt_init(size_t size);

/* Clear all entries in the transposition table.  */
extern void tt_clear(void);

/* Completely destroy the transposition table.  */
extern void tt_destroy(void);

/* Initialize a move selector from a search tree TREE.  An optional
 * BESTMOVE is always returned first.
 */
void move_selector_init(MoveSelector *self, const Tree *tree, chi_move bestmove);

/* Get the next move from the pool or 0 if there are no more moves.  */
chi_move move_selector_next(MoveSelector *self);

/* Evaluate the position in a search tree. Returns positive results for an
 * advantage for the side on move.
 */
extern int evaluate(Tree *tree, int ply, int alpha, int beta);

extern void init_ev_hash(size_t memuse);
extern void clear_ev_hash(void);
extern int probe_ev (chi_pos *pos, bitv64 signature, int *score);
extern void store_ev_entry (chi_pos *pos, bitv64 signature, int score);

#ifdef __cplusplus
extern }
#endif

#endif
