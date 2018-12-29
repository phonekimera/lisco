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

#define MIN_MTT_SIZE (sizeof (TT_Entry) * 2000000)

// FIXME: 19 bytes is much too much!
typedef struct tt_entry {
    bitv64 signature;
    chi_move best;
    short int score;
    unsigned short int depth;
    unsigned char flags;
} TT_Entry;

/* The main transposition table.  */
TT_Entry* mtt = NULL;

unsigned long int mtt_size = 0;

void
init_tt_hashs (memuse)
     unsigned long int memuse;
{
    if (memuse < MIN_MTT_SIZE)
	memuse = MIN_MTT_SIZE;

    mtt_size = chi_closest_prime (memuse / sizeof *mtt);
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
probe_tt (signature, depth, alpha, beta)
     bitv64 signature;
     int depth;
     int* alpha;
     int* beta;
{
    TT_Entry* hit = mtt + (signature % ((bitv64) mtt_size));

    if (hit->signature == signature) {
	if (hit->depth >= depth) {
	    if (hit->flags == HASH_EXACT) {
		*alpha = hit->score;
		return HASH_EXACT;
	    } else if (hit->flags == HASH_ALPHA &&
		       hit->score <= *alpha) {
		*alpha = hit->score;
		return HASH_ALPHA;
	    } else if (hit->flags == HASH_BETA &&
		       hit->score >= *beta) {
		*beta = hit->score;
		return HASH_BETA;
	    }
	}
    }
    return HASH_UNKNOWN;
}

chi_move
best_tt_move (signature)
     bitv64 signature;
{
    TT_Entry* hit = mtt + (signature % ((bitv64) mtt_size));

    if (hit->signature == signature) {
	return hit->best;
    }

    return 0;
}

void
store_tt_entry (signature, move, depth, score, flags)
     bitv64 signature;
     chi_move move;
     int depth;
     int score;
     int flags;
{
    TT_Entry* old_entry = mtt + (signature % ((bitv64) mtt_size));

    /* Collision or not yet seen.  */
    if (!old_entry->signature || old_entry->signature != signature) {
	old_entry->signature = signature;
	old_entry->depth = depth;
	old_entry->score = score;
	old_entry->flags = flags;
	old_entry->best = move;
	return;
    }

    if (!old_entry->best)
	old_entry->best = move;

    if (old_entry->depth > depth)
	return;

    old_entry->signature = signature;
    old_entry->depth = depth;
    old_entry->score = score;
    old_entry->flags = flags;
    if (move)
	old_entry->best = move;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
