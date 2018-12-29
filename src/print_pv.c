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
    static bitv64* seen = NULL;
    static unsigned int seen_size = 0;
    chi_move hashed_move;
    int printed = 0;

    tree->pv_printed = tree->iteration_sdepth;

    if (!whisper)
	printed += fprintf (stdout, "%3d %5d %7ld %10ld ",
			    tree->iteration_sdepth, 
			    chi_value2centipawns (score), 
			    elapsed, tree->nodes);
    
    if (chi_on_move (&tmp_pos) != chi_white)
	printed += fprintf (stdout, " %d. ...", 1 + tmp_pos.half_moves / 2);

    signature = chi_zk_signature (zk_handle, &tmp_pos);
    for (i = 0; i < tree->pv[0].length && i >= ply; ++i) {
	int errnum;

	/* Long lines confuse xboard.  */
	if (printed >= 4000)
	    break;

	while (i >= seen_size) {
	    seen_size += 5;
	    seen = xrealloc (seen, seen_size * sizeof signature);
	}

	seen[i] = signature;

	errnum = chi_print_move (&tmp_pos, tree->pv[0].moves[i], 
				 &buf, &bufsize, 1);
	errnum |= chi_apply_move (&tmp_pos, tree->pv[0].moves[i]);
	
	if (chi_on_move (&tmp_pos) != chi_white)
	    printed += fprintf (stdout, " %d.", 1 + tmp_pos.half_moves / 2);
	
	printed += fprintf (stdout, " %s", buf);

	if (errnum)
	    printed += fprintf (stdout, " [illegal: %s]", 
				chi_strerror (errnum));
	signature = chi_zk_signature (zk_handle, &tmp_pos);
    }

    signature = chi_zk_signature (zk_handle, &tmp_pos);
    while (0 != (hashed_move = best_tt_move (&tmp_pos, signature))) {
	int left_char;
	int right_char;
	int alpha = +INF;
	int beta = -INF;
	int j;

	if (printed >= 4000)
	    break;

	if (seen_size < 5) {
	    seen_size += 5;
	    seen = xrealloc (seen, seen_size * sizeof signature);
	}

	if (chi_print_move (&tmp_pos, hashed_move, &buf, &bufsize, 1))
	    break;

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
	    printed += fprintf (stdout, " %d.", 1 + tmp_pos.half_moves / 2);
	
	printed += fprintf (stdout, " %c%s%c", left_char, buf, right_char);
	
	signature = chi_zk_signature (zk_handle, &tmp_pos);
	seen[i++] = signature;

	for (j = 0; j < i - 1; ++j) {
	    if (seen[j] == signature)
		break;
	}

	if (j < i - 1 && seen[j] == signature) {
	    printed += fprintf (stdout, " ...");
	    break;
	}
    }

    if (printed >= 4000)
	fprintf (stdout, " {cut}");

    if (tree->epd) {
	chi_epd_pos* epd = tree->epd;
	int solved_before = epd->solution == epd->suggestion;
	int solved_after = epd->solution == tree->best_move;
	
#if 0
	fprintf (stderr, "Solution: %s-%s\n", 
		 chi_shift2label (chi_move_from (epd->solution)),
		 chi_shift2label (chi_move_to (epd->solution)));
	fprintf (stderr, "Best move: %s-%s\n", 
		 chi_shift2label (chi_move_from (tree->best_move)),
		 chi_shift2label (chi_move_to (tree->best_move)));
#endif

	if (solved_before != solved_after) {
	    long int elapsed = rdifftime (rtime (), start_time);

	    if (solved_after) {
		epd->cs_stable_solution = elapsed;
		epd->depth_stable_solution = tree->iteration_sdepth;
		fprintf (stdout, " :-)");
	    } else {
		epd->cs_refuted_solution = elapsed;
		epd->depth_refuted_solution = tree->iteration_sdepth;
		fprintf (stdout, " :-(");
	    }
	}
	epd->suggestion = tree->best_move;
    }

    fprintf (stdout, "\n");
}

void dump_pv (tree)
     TREE* tree;
{
    int i;

    fprintf (stderr, "Current variation: ");
    for (i = 0; i < tree->cv.length; ++i) {
	fprintf (stderr, " ");
	my_print_move (tree->cv.moves[i]);
    }
    fprintf (stderr, "\n");

    fprintf (stderr, "Complete PV:\n");
    for (i = 0; tree->pv[i].length; ++i) {
	int j;
	fprintf (stderr, "%02d ", i);

	for (j = 0; j < tree->pv[i].length; ++j) {
	    fprintf (stderr, "|");
	    my_print_move (tree->pv[i].moves[j]);
	}
	
	fprintf (stderr, "\n");
    }
    fflush (stderr);
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
	fprintf (stdout, "%3d %5d %7ld %10ld ",
		 tree->iteration_sdepth, chi_value2centipawns (score), 
		 elapsed, tree->nodes);
    
    if (chi_on_move (&tree->pos) != chi_white)
	fprintf (stdout, " %d. ... ", 1 + tree->pos.half_moves / 2);
    else
	fprintf (stdout, " %d. ", 1 + tree->pos.half_moves / 2);
    
    chi_print_move (&tree->pos, tree->pv[0].moves[0], &buf, &bufsize, 1);
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
	fprintf (stdout, "%3d %5d %7ld %10ld ",
		 tree->iteration_sdepth, chi_value2centipawns (score), 
		 elapsed, tree->nodes);
    
    if (chi_on_move (&tree->pos) != chi_white)
	fprintf (stdout, " %d. ... ", 1 + tree->pos.half_moves / 2);
    else
	fprintf (stdout, " %d. ", 1 + tree->pos.half_moves / 2);
    
    chi_print_move (&tree->pos, tree->pv[0].moves[0], &buf, &bufsize, 1);
    fprintf (stdout, "%s??\n", buf);
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
