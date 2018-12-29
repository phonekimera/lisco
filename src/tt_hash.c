/* tt_hash.c - transposition tables.
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

#include <system.h>

#include <string.h>

#include <libchi.h>

#include "brain.h"

#define MIN_MTT_SIZE (sizeof (TT_Entry) * 5000000)

// FIXME: 19 bytes is much too much!
typedef struct tt_entry {
    /* Used: 64, needed: 64.  */
    bitv64 signature;
    /* Used: 32, needed: 27.  */
    chi_move best;
    /* Used 16, needed: 15.  */
    short int score;
    /* Used 16, needed 9 (8 for two tables).  */
    unsigned short int depth;
} TT_Entry;

/* The main transposition table.  */
TT_Entry* mtt = NULL;

unsigned long int mtt_size = 0;
unsigned long int half_mtt_size = 0;

void
init_tt_hashs (memuse)
     unsigned long int memuse;
{
    if (memuse < MIN_MTT_SIZE)
	memuse = MIN_MTT_SIZE;

    half_mtt_size = chi_closest_prime ((memuse / sizeof *mtt) >> 1);
    mtt_size = half_mtt_size << 1;
    mtt = xrealloc (mtt, mtt_size * sizeof *mtt);
    fprintf (stdout, 
	     "\
Allocated %lu bytes (%lu entries) for main transposition table.\n",
	     mtt_size * sizeof *mtt, mtt_size);

    clear_tt_hashs ();
}

void
clear_tt_hashs ()
{
    fprintf (stdout, "Clearing hash tables.\n");
    memset (mtt, 0, mtt_size * sizeof *mtt);
}

int
probe_tt (pos, signature, depth, alpha, beta)
     chi_pos* pos;
     bitv64 signature;
     int depth;
     int* alpha;
     int* beta;
{
    size_t offset = chi_on_move (pos) == chi_white ? 0 : half_mtt_size;
    TT_Entry* hit = mtt + offset + (signature % ((bitv64) half_mtt_size));

#if DEBUG_BRAIN
    fprintf (stderr, "TTProbe at depth %d (on move: %s, ",
	     depth, chi_on_move (pos) == chi_white ? "white" : "black");
    fprintf (stderr, "alpha: %d, beta: %d, signature: 0x%016llx)\n", 
	     *alpha, *beta, signature);
#endif

    if (hit->signature == signature) {
	unsigned int flags = (hit->best & 0xc0000000) >> 30;

	if (hit->depth >= depth) {
	    if (flags == HASH_EXACT) {
#if DEBUG_BRAIN
		fprintf (stderr, 
			 "Hit for depth %d (score: %d, flags: HASH_EXACT, ",
			 hit->depth, hit->score);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, ")\n");
#endif

		*alpha = hit->score;
		return HASH_EXACT;
	    } else if (flags == HASH_ALPHA &&
		       hit->score <= *alpha) {
#if DEBUG_BRAIN
		fprintf (stderr, 
			 "Hit for depth %d (score: %d, flags: HASH_ALPHA, ",
			 hit->depth, hit->score);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, ")\n");
#endif
		*alpha = hit->score;
		return HASH_ALPHA;
	    } else if (flags == HASH_BETA &&
		       hit->score >= *beta) {
#if DEBUG_BRAIN
		fprintf (stderr, 
			 "Hit for depth %d (score: %d, flags: HASH_BETA, ",
			 hit->depth, hit->score);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, ")\n");
#endif
		*beta = hit->score;
		return HASH_BETA;
#if DEBUG_BRAIN
	    } else if (flags == HASH_ALPHA) {
		fprintf (stderr, "HASH_ALPHA score %d too high at depth %d, ",
			 hit->score, depth);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, "\n");
	    } else if (flags == HASH_BETA) {
		fprintf (stderr, "HASH_BETA score %d too low at depth %d, ",
			 hit->score, depth);
		my_print_move (hit->best & 0x3fffffff);
		fprintf (stderr, "\n");
#endif
	    }
	}
    }

#if DEBUG_BRAIN
    fprintf (stderr, "TTProbe failed\n");
#endif

    return HASH_UNKNOWN;
}

chi_move
best_tt_move (pos, signature)
     chi_pos* pos;
     bitv64 signature;
{
    size_t offset = chi_on_move (pos) == chi_white ? 0 : half_mtt_size;
    TT_Entry* hit = mtt + offset + (signature % ((bitv64) half_mtt_size));

    if (hit->signature == signature) {
	return hit->best & 0x3fffffff;
    }

    return 0;
}

int
store_tt_entry (pos, signature, move, depth, score, flags)
     chi_pos* pos;
     bitv64 signature;
     chi_move move;
     int depth;
     int score;
     int flags;
{
    size_t offset = chi_on_move (pos) == chi_white ? 0 : half_mtt_size;
    TT_Entry* old_entry = mtt + offset + 
	(signature % ((bitv64) half_mtt_size));

    int retval = old_entry->signature && old_entry->signature != signature ? 
	1 : 0;

#if DEBUG_BRAIN
    fprintf (stderr, "TTStore score %d for %s with move ", 
	     score, chi_on_move (pos) == chi_white ? "white" : "black");
    my_print_move (move);
    fprintf (stderr, " at depth %d (%s), signature: %016llx\n", 
	     depth, flags == HASH_EXACT ? 
	     "HASH_EXACT" : flags == HASH_ALPHA ? "HASH_ALPHA" :
	     flags == HASH_BETA ? "HASH_BETA" : "unknown flag",
	     signature);
#endif

#if 0
    if (chi_move_from (move) == 12 && chi_move_to (move) == 21) {
	unsigned int old_flags = (old_entry->best & 0xc0000000) >> 30;
	unsigned int old_move = (old_entry->best & ~0xc0000000);

	fprintf (stderr, "Storing score %d with move ", score);
	my_print_move (move);
	fprintf (stderr, " at depth %d (%s)\n", depth, flags == HASH_EXACT ? 
		 "HASH_EXACT" : flags == HASH_ALPHA ? "HASH_ALPHA" :
		 flags == HASH_BETA ? "HASH_BETA" : "unknown flag");
	fprintf (stderr, "Signature: 0x%016llx\n", signature);
	fprintf (stderr, "Old signature: 0x%016llx\n", old_entry->signature);
	fprintf (stderr, "Old score: %d\n", old_entry->score);
	fprintf (stderr, "Old depth: %d\n", old_entry->depth);
	fprintf (stderr, "Old flags: %s\n", old_flags == HASH_EXACT ? 
		 "HASH_EXACT" : old_flags == HASH_ALPHA ? "HASH_ALPHA" :
		 old_flags == HASH_BETA ? "HASH_BETA" : "unknown flag");
	fprintf (stderr, "Old move: ");
	my_print_move (old_entry->best);
	fprintf (stderr, "\n");
	fflush (stderr);
	dump_board (pos);
    }
#endif

    /* Collision or not yet seen.  */
    if (!old_entry->signature || old_entry->signature != signature) {
	old_entry->signature = signature;
	old_entry->depth = depth;
	old_entry->score = score;
	old_entry->best = move | (flags << 30);
	return retval;
    }

    if (old_entry->depth > depth)
	return retval;

    old_entry->signature = signature;
    old_entry->depth = depth;
    old_entry->score = score;
    old_entry->best = move | (flags << 30);

    return retval;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
