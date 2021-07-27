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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>

#include "lisco.h"

static void allocate_time(Tree *tree, SearchParams *params);
static double moves_to_go(chi_pos *position);

int
process_search_params(Tree *tree, SearchParams *params)
{
	if (params->mate) {
		params->depth = 2 * params->mate - 1;
	}

	if (params->depth) {
		tree->max_depth = params->depth;
	} else {
		/* If no other hint given, use 30 seconds per move.  */
		tree->fixed_time = 30000;
	}

	/* Initial value for calibration.  */
	tree->nodes_to_tc = 10000;

	if (params->movetime) {
		tree->fixed_time = params->movetime;
	} else if (params->nodes) {
		tree->nodes_to_tc = params->nodes;
		tree->fixed_time = 0;
	} else if (params->mytime) {
		allocate_time(tree, params);
	}

	/* This is ugly. :( */
	if (params->searchmoves.num_moves) {
		tree->searchmoves.moves = params->searchmoves.moves;
		tree->searchmoves.num_moves = params->searchmoves.num_moves;
	}

	return 1;
}

static void
allocate_time(Tree *tree, SearchParams *params)
{
	// First get a rough estimate of the moves to go.
	float mtg = moves_to_go(&tree->position);

	if (params->movestogo < mtg) {
		mtg = params->movestogo;
	}

	float time_left = params->mytime + params->movestogo * params->myinc;

	// FIXME! This should not be fixed_time but have a better name.
	// FIXME! Depending on the volatility of the position, there should be
	// a time cushion that can be used if the evaluation changes a lot between
	// iterations.
	tree->fixed_time = (int) floor(0.5 + time_left / mtg);
}

/* Estimate how many moves until the end of the game.  We never assume less
 * than MIN_MOVES_REMAINING and never more than MAX_MOVES_REMAINING.  Inbetween
 * we interpolate linearily based on the number of pieces of the side behind
 * by material.
 */
static double
moves_to_go(chi_pos *position)
{
	// FIXME! These parameters should be configurable and their defaults
	// should be tuned!
	unsigned min_moves_remaining = 20;
	unsigned max_moves_remaining = 60;

	// We make two very simple assumptions.  The popcount of the weaker
	// party decreases in the course of the game from 20 to 1.  That
	// allows us a linear interpolation for the number of moves to go.
	// On the other hand, the material imbalance may change from 0
	// to 9 queens (81 for our purposes).  But an imbalance of 10
	// (one queen plus a pawn) should guaranty a trivial win for the side
	// to move and we can limit the material imbalance to that.
	//
	// And then we simply give each a result a weight with the two results
	// summing up to 1.0.
	double popcount_weight = 0.75;
	double material_weight = (1 - popcount_weight);
	int moves_range = max_moves_remaining - min_moves_remaining;

	int wpopcount = chi_popcount(position->w_pieces);
	int bpopcount = chi_popcount(position->b_pieces);
	int popcount = wpopcount < bpopcount ? wpopcount : bpopcount;

	// Popcount slope and constant offset.
	double mpc = moves_range / (20 - 1);
	double cpc = min_moves_remaining - mpc;

	// Material imbalance slope and constant offset.
	double mmc = moves_range / 10 - 0;
	double cmc = min_moves_remaining;

	// FIXME! Since this is only done once per ply, a full evaluation of
	// the position should be done instead of just looking at the material
	// balance.
	float mtg = popcount_weight * (mpc * popcount + cpc)
		+ material_weight * (mmc * chi_material(position) + cmc);

	return mtg;
}