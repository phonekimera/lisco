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

#include <string.h>

#include <libchi.h>

#include "xalloc.h"

#include "lisco.h"

#define DEBUG_TT 1

#define HASH_UNKNOWN ((unsigned int) 0)
#define HASH_ALPHA   ((unsigned int) 1)
#define HASH_BETA    ((unsigned int) 2)
#define HASH_EXACT   ((unsigned int) 3)

#define MIN_TT_SIZE (sizeof (TT_Entry) * 1000000)

typedef struct tt_entry {
	bitv64 signature;
	// FIXME! This wastes a lot of space! For the move we just need the
	// bare information. The rest can be restored laster.  And for the depth
	// we need a lot less space.
	chi_move best;
	short int score;
	unsigned short int depth;
} TT_Entry;

/* The transposition table.  It is organized as follows: The first
   half is reserved for positions with white on move, the second half
   for positions with black on move.  For each position, the first third
   contains a partial hash table that only gets updated when a new 
   entry has at least the same depth as the old entry, the second and 
   third third, will always be overwritten.  */
TT_Entry* tt = NULL;

static unsigned long int tt_size = 0;
static unsigned long int half_tt_size = 0;

void
init_tt_hashs (unsigned long int memuse)
{
	if (memuse < MIN_TT_SIZE)
		memuse = MIN_TT_SIZE;

	half_tt_size = chi_closest_prime ((memuse / sizeof *tt) / 6);

	tt_size = half_tt_size << 1;
	tt = xrealloc (tt, 6 * half_tt_size * sizeof *tt);

	clear_tt_hashs ();
}

void
clear_tt_hashs ()
{
	memset (tt, 0, (tt_size << 1) * sizeof *tt);
}

unsigned int
probe_tt (chi_pos *pos, bitv64 signature, int depth, int *alpha, int *beta)
{
	size_t offset;
	TT_Entry* hit;
	int wtm = chi_on_move (pos) == chi_white;

	if (wtm)
		offset = signature % ((bitv64) half_tt_size);
	else
		offset = tt_size + half_tt_size + signature % ((bitv64) half_tt_size);

	hit = tt + offset;

	if (hit->signature && hit->signature != signature) {
		if (wtm)
			offset = half_tt_size + signature % ((bitv64) tt_size);
		else
			offset = (tt_size << 1) + signature % ((bitv64) tt_size);
		hit = tt + offset;
	}

#if DEBUG_TT
	fprintf (stderr, "TTProbe at depth %d (on move: %s, ",
			depth, wtm ? "white" : "black");
	fprintf (stderr, "alpha: %d, beta: %d, signature: %llu)\n", 
			*alpha, *beta, signature);
#endif

	if (hit->signature == signature) {
		unsigned int flags = hit->best >> 30;

	if (hit->depth >= depth) {
		if (flags == HASH_EXACT) {
#if DEBUG_TT
		fprintf (stderr, 
			 "Hit for depth %d (score: %d, flags: HASH_EXACT, ",
			 hit->depth, hit->score);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, ")\n");
#endif
			*alpha = hit->score;
			return HASH_EXACT;
	} else if (flags == HASH_ALPHA && hit->score >= *beta) {
#if DEBUG_TT
		fprintf (stderr, 
			 "Hit for depth %d (score: %d, flags: HASH_ALPHA, ",
			 hit->depth, hit->score);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, ")\n");
#endif
		*alpha = hit->score;
		return HASH_ALPHA;
	} else if (flags == HASH_BETA && hit->score <= *alpha) {
#if DEBUG_TT
		fprintf (stderr, 
			 "Hit for depth %d (score: %d, flags: HASH_BETA, ",
			 hit->depth, hit->score);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, ")\n");
#endif
		*beta = hit->score;
		return HASH_BETA;
	} else if (flags == HASH_ALPHA) {
#if DEBUG_TT
		fprintf (stderr, "HASH_ALPHA score %d too high at depth %d, ",
			 hit->score, depth);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, "\n");
#endif
	} else if (flags == HASH_BETA) {
#if DEBUG_TT
		fprintf (stderr, "HASH_BETA score %d too low at depth %d, ",
			 hit->score, depth);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, "\n");
#endif
	}

		if (flags == HASH_BETA && hit->score < *beta)
			*beta = hit->score;
		else if (flags == HASH_ALPHA && hit->score > *alpha)
			*alpha = hit->score;
		}
	}

#if DEBUG_TT
	fprintf (stderr, "TTProbe failed\n");
#endif

	return HASH_UNKNOWN;
}

chi_move
best_tt_move (chi_pos *pos, bitv64 signature)
{
	size_t offset;
	TT_Entry* hit;
	int wtm = chi_on_move (pos) == chi_white;

	if (wtm)
		offset = signature % ((bitv64) half_tt_size);
	else
		offset = tt_size + half_tt_size + signature % ((bitv64) half_tt_size);

	hit = tt + offset;

	if (hit->signature && hit->signature != signature) {
		if (wtm)
			offset = half_tt_size + signature % ((bitv64) tt_size);
		else
			offset = (tt_size << 1) + signature % ((bitv64) tt_size);
		hit = tt + offset;
	}

	if (hit->signature == signature)
		return hit->best & 0x3fffffff;

	return 0;
}

void
store_tt_entry (
		chi_pos *pos, bitv64 signature, chi_move move, unsigned short int depth,
		short int score, unsigned int flags)
{
	size_t offset;
	TT_Entry* hit;
	int wtm = chi_on_move (pos) == chi_white;

	if (wtm)
		offset = signature % ((bitv64) half_tt_size);
	else
		offset = tt_size + half_tt_size + signature % ((bitv64) half_tt_size);

	hit = tt + offset;

#if DEBUG_BRAIN
    fprintf (stderr, "TTStore score %d for %s with move ", 
	     score, wtm ? "white" : "black");
    my_print_move (move);
    fprintf (stderr, " at depth %d (%s), signature: 0x%016llx\n", 
	     depth, flags == HASH_EXACT ? 
	     "HASH_EXACT" : flags == HASH_ALPHA ? "HASH_ALPHA" :
	     flags == HASH_BETA ? "HASH_BETA" : "unknown flag",
	     signature);
#endif

	if (hit->signature) {
		/* We have a conflict that is resolved as follows: If the new
		entry has a higher depth, it replaces the old entry, if it
		has the same depth, HASH_EXACT is preferred over HASH_BETA, and
		HASH_BETA is preferred over HASH_ALPHA.  */
		int move_entry = 0;
		unsigned int old_flags = 0x3 & (hit->best >> 30);
		size_t offset_always;
		TT_Entry* always;

		if (wtm)
			offset_always = half_tt_size + signature % ((bitv64) tt_size);
		else
			offset_always = (tt_size << 1) + signature % ((bitv64) tt_size);

		always = tt + offset_always;

		if (hit->depth < depth)
			move_entry = 1;
		else if ((hit->depth == depth) && (old_flags <= flags))
			move_entry = 1;

		if (move_entry) {
			*always = *hit;
			hit->signature = signature;
			hit->depth = depth;
			hit->score = score;
			hit->best = move | (flags << 30);
		} else {
			always->signature = signature;
			always->depth = depth;
			always->score = score;
			always->best = move | (flags << 30);
		}
	} else {
		/* No conflict.  */
		hit->signature = signature;
		hit->depth = depth;
		hit->score = score;
		hit->best = move | (flags << 30);
	}
}
