/* qtt_hash.c - quiescence transposition tables.
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

#include <stdio.h>
#include <string.h>

#include <libchi.h>

#include "brain.h"

#define MIN_QTT_SIZE (sizeof (QTT_Entry) * 500000)

typedef struct qtt_entry {
    bitv64 signature;
    short int score;
    unsigned char flags;
} QTT_Entry;

/* The transposition table.  It is organized as follows: The first
   half is reserved for positions with white on move, the second half
   for positions with black on move.  For each position, the first third
   contains a partial hash table that only gets updated when a new 
   entry has at least the same depth as the old entry, the second and 
   third third, will always be overwriqtten.  */
QTT_Entry* qtt = NULL;

static unsigned long int qtt_size = 0;
static unsigned long int half_qtt_size = 0;

void
init_qtt_hashs (memuse)
     unsigned long int memuse;
{
    if (memuse < MIN_QTT_SIZE)
	memuse = MIN_QTT_SIZE;

    half_qtt_size = chi_closest_prime ((memuse / sizeof *qtt) / 6);

    qtt_size = half_qtt_size << 1;
    qtt = xrealloc (qtt, 6 * half_qtt_size * sizeof *qtt);
    fprintf (stdout, 
	     "\
Allocated %lu bytes (%lu entries) for quiescence transposition table.\n",
	     6 * half_qtt_size * sizeof *qtt, 6 * half_qtt_size);

    clear_qtt_hashs ();
}

void
clear_qtt_hashs ()
{
    fprintf (stdout, "Clearing quiescence transposition table.\n");
    memset (qtt, 0, (qtt_size << 1) * sizeof *qtt);
}

unsigned int
probe_qtt (pos, signature, alpha, beta)
     chi_pos* pos;
     bitv64 signature;
     int* alpha;
     int* beta;
{
    size_t offset;
    QTT_Entry* hit;
    int wtm = chi_on_move (pos) == chi_white;

    if (wtm)
	offset = signature % ((bitv64) half_qtt_size);
    else
	offset = qtt_size + half_qtt_size + 
	    signature % ((bitv64) half_qtt_size);

    hit = qtt + offset;

    if (hit->signature && hit->signature != signature) {
	if (wtm)
	    offset = half_qtt_size + signature % ((bitv64) qtt_size);
	else
	    offset = (qtt_size << 1) + signature % ((bitv64) qtt_size);
	hit = qtt + offset;
    }

#if DEBUG_BRAIN
    fprintf (stderr, "QTTProbe (on move: %s, ",
	     wtm ? "white" : "black");
    fprintf (stderr, "alpha: %d, beta: %d, signature: 0x%016llx)\n", 
	     *alpha, *beta, signature);
#endif

    if (hit->signature == signature) {
	unsigned int flags = hit->flags;
	int score = hit->score;

	if (flags == HASH_EXACT) {
#if DEBUG_BRAIN
	    fprintf (stderr, 
		     "QTT hit (score: %d, flags: HASH_EXACT)\n", score);
#endif
	    
	    *alpha = score;
	    return HASH_EXACT;
	} else if (flags == HASH_ALPHA && score >= *beta) {
#if DEBUG_BRAIN
	    fprintf (stderr, 
		     "QTT hit (score: %d, flags: HASH_ALPHA)\n", score);
#endif
	    *alpha = score;
	    return HASH_ALPHA;
	} else if (flags == HASH_BETA && score <= *alpha) {
#if DEBUG_BRAIN
	    fprintf (stderr, 
		     "QTT hit (score: %d, flags: HASH_BETA)\n", score);
#endif
	    *beta = score;
	    return HASH_BETA;
#if DEBUG_BRAIN
	} else if (flags == HASH_ALPHA) {
	    fprintf (stderr, "HASH_ALPHA qtt score %d too high)\n", score);
	} else if (flags == HASH_BETA) {
	    fprintf (stderr, "HASH_BETA qtt score %d too low)\n", score);
#endif
	}

	if (flags == HASH_BETA && hit->score < *beta)
	    *beta = hit->score;
	else if (flags == HASH_ALPHA && hit->score > *alpha)
	    *alpha = hit->score;
    }

#if DEBUG_BRAIN
    fprintf (stderr, "QTTProbe failed\n");
#endif

    return HASH_UNKNOWN;
}

void
store_qtt_entry (pos, signature, score, flags)
     chi_pos* pos;
     bitv64 signature;
     int score;
     unsigned int flags;
{
    size_t offset;
    QTT_Entry* hit;
    int wtm = chi_on_move (pos) == chi_white;

    if (wtm)
	offset = signature % ((bitv64) half_qtt_size);
    else
	offset = qtt_size + half_qtt_size + signature % ((bitv64) half_qtt_size);

    hit = qtt + offset;

#if DEBUG_BRAIN
    fprintf (stderr, "QTTStore score %d for %s (%s), signature: 0x%016llx\n", 
	     score, wtm ? "white" : "black",
	     flags == HASH_EXACT ? 
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
	unsigned int old_flags = hit->flags;
	size_t offset_always;
	QTT_Entry* always;

	if (wtm)
	    offset_always = half_qtt_size + signature % ((bitv64) qtt_size);
	else
	    offset_always = (qtt_size << 1) + signature % ((bitv64) qtt_size);

	always = qtt + offset_always;

	if (old_flags <= flags)
	    move_entry = 1;

	if (move_entry) {
	    *always = *hit;
	    hit->signature = signature;
	    hit->score = score;
	    hit->flags = flags;
	} else {
	    always->signature = signature;
	    always->score = score;
	    hit->flags = flags;
	}
    } else {
	/* No conflict.  */
	hit->signature = signature;
	hit->score = score;
	hit->flags = flags;
    }
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
