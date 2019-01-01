/* epd.c - EPD test suite stuff.
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <libchi.h>

#include "xalloc.h"
#include "error.h"

#include "tate.h"
#include "brain.h"

int
handle_epd (epdstr, max_time, epd)
     const char* epdstr;
     unsigned long max_time;
     chi_epd_pos* epd;
{
    int errnum;
    int retval = 0;
    static char* buf = NULL;
    static unsigned int bufsize = 0;

    errnum = chi_parse_epd (epd, epdstr);
    epd->fixed_time = max_time;

    if (errnum) {
	fprintf (stdout, "Error (%s): %s\n",
		 chi_strerror (errnum), epdstr);
	return EVENT_CONTINUE;
    }

    current = epd->pos;

    mate_announce = 0;
    game_over = 0;
    force = 0;
    engine_color = chi_on_move (&current);
    game_hist_ply = 0;
    game_hist[0].pos = current;
    game_hist[0].signature = chi_zk_signature (zk_handle, &current);

    current_score = engine_color == chi_white ? 
	chi_material (&current) * 100 : 
	chi_material (&current) * -100;

    chi_print_move (&epd->pos, epd->solution, &buf, &bufsize, 1);
    fprintf (stdout, "    Solving test '%s', solution: %s.\n", 
	     epd->id, buf);
    clear_tt_hashs ();
    clear_qtt_hashs ();
    retval = handle_go (epd);

    fprintf (stdout, "    Results for test '%s'.\n", epd->id);
    fprintf (stdout, "        Solved: %s.\n", 
	     epd->solution == epd->suggestion ? "yes" : "no");
    if (epd->depth_stable_solution) {
	fprintf (stdout,
		 "        Solution last found after %ld.%02ld s at depth %d.\n",
		 epd->cs_stable_solution / 100, epd->cs_stable_solution % 100,
		 epd->depth_stable_solution);
    }
    if (epd->depth_refuted_solution) {
	fprintf (stdout,
		 "        Solution last refuted after %ld.%02ld s at depth %d.\n",
		 epd->cs_refuted_solution / 100, epd->cs_refuted_solution % 100,
		 epd->depth_refuted_solution);
    }

    return retval;
}

int
handle_epdfile (filename, max_time)
     const char* filename;
     unsigned long max_time;
{
    static char* buf = NULL;
    static size_t bufsize = 0;
    chi_epd_pos** epds = NULL;
    int epds_allocated = 0;
    int num_epds = 0;
    FILE* f;

    f = fopen (filename, "r");
    if (!f) {
	error (EXIT_SUCCESS, errno, "Error (%s)", filename);
	return EVENT_CONTINUE;
    }

    while (-1 != getline (&buf, &bufsize, f)) {
	char* epd_str = buf;
	chi_epd_pos epd;
	int retval;
	int errnum;

	while (*epd_str &&
	       (*epd_str == ' ' || *epd_str == '\t'))
	    ++epd_str;
	if (!*epd_str)
	    continue;

	errnum = chi_parse_epd (&epd, buf);
	if (errnum) {
	    fprintf (stdout, "Error (%s): %s\n",
		     chi_strerror (errnum), buf);
	    continue;
	}
	chi_free_epd (&epd);

	retval = handle_epd (buf, max_time, &epd);
	if (retval != EVENT_CONTINUE)
	    break;

	while (num_epds >= epds_allocated) {
	    epds_allocated += 8;
	    epds = xrealloc (epds, epds_allocated * sizeof &epd);
	}

	epds[num_epds] = (chi_epd_pos*) xmalloc (sizeof epd);
	*(epds[num_epds++]) = epd;
    }
    
    fclose (f);

    if (!num_epds) {
	fprintf (stdout, "Error: file contains no valid EPD positions.\n");
    } else {
	unsigned long total_time = 0;
	int solved_epds = 0;
	int i;
	int l;

	fprintf (stdout, "\n      Results for testsuite from file '%s':\n",
		 filename);
	
	fprintf (stdout, "\
    +--------------------+--------------------+---+-----------------------+\n\
    | Solution           | Refuted            | R | Name                  |\n\
    +-------+------------+-------+------------+---+-----------------------+\n");

	for (i = 0; i < num_epds; ++i) {
	    chi_epd_pos* epd = epds[i];
	    int solved = 0;

	    if (epd->solution == epd->suggestion) {
		++solved_epds;
		total_time += epd->cs_stable_solution;
		solved = 1;
	    } else {
		total_time += max_time;
	    }

	    fprintf (stdout,
		     "    | %5d | %7lu.%02lu | %5d | %7lu.%02lu | %c | %-22s|\n",
		     epd->depth_stable_solution,
		     epd->cs_stable_solution / 100, 
		     epd->cs_stable_solution % 100,
		     epd->depth_refuted_solution,
		     epd->cs_refuted_solution / 100, 
		     epd->cs_refuted_solution % 100,
		     solved ? 'X' : '-',
		     epd->id);
	}

	fprintf (stdout, "\
    +-------+------------+-------+------------+---+-----------------------+\n");
	l = fprintf (stdout, 
		     "    | Solved %d/%d (time %ld.%02ld/%ld.%02ld s).",
		     solved_epds, num_epds, 
		     total_time / 100, total_time % 100,
		     (num_epds * max_time) / 100, (num_epds * max_time) % 100);
	for (; l < 74; ++l)
	    fputc (' ', stdout);
	fputs ("|\n", stdout);
	fprintf (stdout, "\
    +---------------------------------------------------------------------+\n");
    }

    if (epds) {
	int i;
	
	for (i = 0; i < num_epds; ++i) {
	    if (epds[i]) {
		chi_free_epd (epds[i]);
		free (epds[i]);
	    }
	}
	free (epds);
    }

    return EVENT_CONTINUE;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
