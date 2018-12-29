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
probe_tt (tree, signature, depth, alpha, beta)
     TREE* tree;
     bitv64 signature;
     int depth;
     int* alpha;
     int* beta;
{
    size_t offset = chi_on_move (&tree->pos) == chi_white ? 0 : half_mtt_size;
    TT_Entry* hit = mtt + offset + (signature % ((bitv64) half_mtt_size));

    if (hit->signature == signature) {
	unsigned int flags = (hit->best & 0xc0000000) >> 30;
	if (hit->depth >= depth) {
	    if (flags == HASH_EXACT) {
		*alpha = hit->score;
		return HASH_EXACT;
	    } else if (flags == HASH_ALPHA &&
		       hit->score <= *alpha) {
		*alpha = hit->score;
		return HASH_ALPHA;
	    } else if (flags == HASH_BETA &&
		       hit->score >= *beta) {
		*beta = hit->score;
		return HASH_BETA;
	    }
	}
    }
    return HASH_UNKNOWN;
}

chi_move
best_tt_move (tree, signature)
     TREE* tree;
     bitv64 signature;
{
    size_t offset = chi_on_move (&tree->pos) == chi_white ? 0 : half_mtt_size;
    TT_Entry* hit = mtt + offset + (signature % ((bitv64) half_mtt_size));

    if (hit->signature == signature) {
	return hit->best & 0x3fffffff;
    }

    return 0;
}

int
store_tt_entry (tree, signature, move, depth, score, flags)
     TREE* tree;
     bitv64 signature;
     chi_move move;
     int depth;
     int score;
     int flags;
{
    size_t offset = chi_on_move (&tree->pos) == chi_white ? 0 : half_mtt_size;
    TT_Entry* old_entry = mtt + offset + 
	(signature % ((bitv64) half_mtt_size));

    int retval = old_entry->signature && old_entry->signature != signature ? 
	1 : 0;

#if DEBUG_BRAIN
    fprintf (stderr, "Storing score %d with move ", score);
    my_print_move (move);
    fprintf (stderr, " at depth %d (%s)\n", depth, flags == HASH_EXACT ? 
	"HASH_EXACT" : flags == HASH_ALPHA ? "HASH_ALPHA" :
	flags == HASH_BETA ? "HASH_BETA" : "unknown flag");
#endif

    /* Collision or not yet seen.  */
    if (!old_entry->signature || old_entry->signature != signature) {
	old_entry->signature = signature;
	old_entry->depth = depth;
	old_entry->score = score;
	old_entry->best = move | (flags << 30);
	return retval;
    }

    if (!old_entry->best) {
	old_entry->best &= 0xcfffffff;
	old_entry->best |= move;
    }

    if (old_entry->depth > depth)
	return retval;

    old_entry->signature = signature;
    old_entry->depth = depth;
    old_entry->score = score;
    if (move) {
	old_entry->best = move | (flags << 30);
    } else {
	old_entry->best &= (old_entry->best & 0xcfffffff) | (flags << 30);	
    }

    return retval;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
