/* print_pv.c - print the principal variation
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

#include <libchi.h>

#include "brain.h"
#include "time_ctrl.h"

void
print_pv (tree, score, whisper, ply)
     TREE* tree;
     int score;
     int whisper;
     int ply;
{
    int i;
    long int elapsed = rdifftime (rtime (), start_time);
    chi_pos tmp_pos = tree->pos;
    static char* buf = NULL;
    static unsigned int bufsize = 0;

    tree->pv_printed = tree->iteration_depth;

    if (!whisper)
	fprintf (stdout, "%3d %5d %7ld %8ld  ",
		 tree->iteration_depth, chi_value2centipawns (score), 
		 elapsed, tree->nodes);
    
    if (chi_on_move (&tmp_pos) != chi_white)
	fprintf (stdout, " %d. ... ", 1 + tmp_pos.half_moves / 2);

    for (i = 0; i < tree->pv[0].length && i >= ply; ++i) {
	int errnum;
	chi_print_move (&tmp_pos, tree->pv[0].moves[i], &buf, &bufsize, 0);
	errnum = chi_apply_move (&tmp_pos, tree->pv[0].moves[i]);
	
	if (chi_on_move (&tmp_pos) != chi_white)
	    fprintf (stdout, " %d.", 1 + tmp_pos.half_moves / 2);
	
	fprintf (stdout, " %s", buf);

	if (errnum)
	    fprintf (stdout, " [illegal: %s]", chi_strerror (errnum));
    }

    if (i < tree->iteration_depth) {
	/* Must be a hash move.  */
	bitv64 signature = chi_zk_signature (zk_handle, &tmp_pos);
	chi_move hash_move = best_tt_move (tree, signature);

	if (hash_move) {
	    chi_print_move (&tmp_pos, hash_move, &buf, &bufsize, 0);
	    fprintf (stdout, " <%s>", buf);
	} else {
	    fprintf (stdout, " <HT>");
	}
    }

    fprintf (stdout, "\n");

#if 0
    tmp_pos = tree->pos;
    for (i = 0; i < tree->iteration_depth; ++i) {
	int j;
	for (j = 0; j < 1 + tree->pv[i + 1].length; ++j) {
	    fprintf (stdout, " %s-%s%s", 
		     chi_shift2label (chi_move_from (tree->pv[i].moves[j])),
		     chi_shift2label (chi_move_to (tree->pv[i].moves[j])),
		     i == ply ? "*" : " ");

	}
	fprintf (stdout, "\n");
    }
#endif
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
