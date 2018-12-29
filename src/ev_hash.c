/* ev_hash.c - evaluation cache.
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

#define MIN_EV_SIZE (sizeof (EV_Entry) * 100000)

typedef struct ev_entry {
    bitv64 signature;
    short int score;
} EV_Entry;

EV_Entry* ev = NULL;

static unsigned long int ev_size = 0;
static unsigned long int half_ev_size = 0;

void
init_ev_hashs (memuse)
     unsigned long int memuse;
{
    if (memuse < MIN_EV_SIZE)
	memuse = MIN_EV_SIZE;

    half_ev_size = chi_closest_prime ((memuse / sizeof *ev) / 2);

    ev_size = half_ev_size << 1;
    ev = xrealloc (ev, ev_size * sizeof *ev);
    fprintf (stdout, 
	     "\
Allocated %lu bytes (%lu entries) for evaluation cache.\n",
	     ev_size * sizeof *ev, ev_size);

    clear_ev_hashs ();
}

void
clear_ev_hashs ()
{
    fprintf (stdout, "Clearing evaluation cache.\n");
    memset (ev, 0, ev_size * sizeof *ev);
}

int
probe_ev (pos, signature, score)
     chi_pos* pos;
     bitv64 signature;
     int* score;
{
    size_t offset;
    EV_Entry* hit;
    int wtm = chi_on_move (pos) == chi_white;

    if (wtm)
	offset = signature % ((bitv64) half_ev_size);
    else
	offset = half_ev_size + signature % ((bitv64) half_ev_size);

    hit = ev + offset;

    if (hit->signature == signature) {
	*score = hit->score;
	return 1;
    }

    return 0;
}

void
store_ev_entry (pos, signature, score)
     chi_pos* pos;
     bitv64 signature;
     int score;
{
    size_t offset;
    EV_Entry* hit;
    int wtm = chi_on_move (pos) == chi_white;

    if (wtm)
	offset = signature % ((bitv64) half_ev_size);
    else
	offset = half_ev_size + signature % ((bitv64) half_ev_size);

    hit = ev + offset;

    hit->signature = signature;
    hit->score = score;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
