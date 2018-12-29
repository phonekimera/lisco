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
    bitv64 signature;
    chi_move hashed_move;

    tree->pv_printed = tree->iteration_depth;

    if (!whisper)
	fprintf (stdout, "%3d %5d %7ld %8ld  ",
		 tree->iteration_depth, chi_value2centipawns (score), 
		 elapsed, tree->nodes);
    
    if (chi_on_move (&tmp_pos) != chi_white)
	fprintf (stdout, " %d. ...", 1 + tmp_pos.half_moves / 2);

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

    signature = chi_zk_signature (zk_handle, &tmp_pos);
    while (0 != (hashed_move = best_tt_move (&tmp_pos, signature))) {
	int left_char;
	int right_char;
	int alpha = +INF;
	int beta = -INF;

	chi_print_move (&tmp_pos, hashed_move, &buf, &bufsize, 0);

	switch (probe_tt (&tmp_pos, signature, 0, &alpha, &beta)) {
	    case HASH_ALPHA:
		left_char = right_char = '<';
		break;
	    case HASH_BETA:
		left_char = right_char = '>';
		break;
	    case HASH_EXACT:
		left_char = '<';
		right_char = '>';
		break;
	    default:
		left_char = right_char = '?';
		break;
	}

	if (chi_apply_move (&tmp_pos, hashed_move))
	    break;
	if (chi_on_move (&tmp_pos) != chi_white)
	    fprintf (stdout, " %d.", 1 + tmp_pos.half_moves / 2);
	
	fprintf (stdout, " %c", left_char);

	fprintf (stdout, "%s%c", buf, right_char);
	
	signature = chi_zk_signature (zk_handle, &tmp_pos);
    }

    fprintf (stdout, "\n");
}

void
print_fail_high (tree, score, whisper)
     TREE* tree;
     int score;
     int whisper;
{
    long int elapsed = rdifftime (rtime (), start_time);
    static char* buf = NULL;
    static unsigned int bufsize = 0;

    if (!whisper)
	fprintf (stdout, "%3d %5d %7ld %8ld  ",
		 tree->iteration_depth, chi_value2centipawns (score), 
		 elapsed, tree->nodes);
    
    if (chi_on_move (&tree->pos) != chi_white)
	fprintf (stdout, " %d. ... ", 1 + tree->pos.half_moves / 2);
    else
	fprintf (stdout, " %d. ", 1 + tree->pos.half_moves / 2);
    
    chi_print_move (&tree->pos, tree->pv[0].moves[0], &buf, &bufsize, 0);
    fprintf (stdout, "%s!!\n", buf);
}

void
print_fail_low (tree, score, whisper)
     TREE* tree;
     int score;
     int whisper;
{
    long int elapsed = rdifftime (rtime (), start_time);
    static char* buf = NULL;
    static unsigned int bufsize = 0;

    if (!whisper)
	fprintf (stdout, "%3d %5d %7ld %8ld  ",
		 tree->iteration_depth, chi_value2centipawns (score), 
		 elapsed, tree->nodes);
    
    if (chi_on_move (&tree->pos) != chi_white)
	fprintf (stdout, " %d. ... ", 1 + tree->pos.half_moves / 2);
    else
	fprintf (stdout, " %d. ", 1 + tree->pos.half_moves / 2);
    
    chi_print_move (&tree->pos, tree->pv[0].moves[0], &buf, &bufsize, 0);
    fprintf (stdout, "%s??\n", buf);
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
