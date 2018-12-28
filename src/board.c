/* board.c - Functions for board manipulation.
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

#include "board.h"

void
dump_board (pos)
     chi_pos* pos;
{
    int rank, file;

    display_board (pos);

    fprintf (stdout, "\n Pieces   Pawns    Knights  Bishops  Queens   Kings\n");
    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	
	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);

	    if (pos->w_pieces & (1ULL << off))
		fputc ('W', stdout);
	    else if (pos->b_pieces & (1ULL << off))
		fputc ('b', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_pawns & (1ULL << off))
		fputc ('P', stdout);
	    else if (pos->b_pawns & (1ULL << off))
		fputc ('p', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_knights & (1ULL << off))
		fputc ('N', stdout);
	    else if (pos->b_knights & (1ULL << off))
		fputc ('n', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_bishops & (1ULL << off))
		fputc ('B', stdout);
	    else if (pos->b_bishops & (1ULL << off))
		fputc ('b', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_rooks & (1ULL << off))
		fputc ('R', stdout);
	    else if (pos->b_rooks & (1ULL << off))
		fputc ('r', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_kings & (1ULL << off))
		fputc ('K', stdout);
	    else if (pos->b_kings & (1ULL << off))
		fputc ('k', stdout);
	    else
		fputc ('.', stdout);
	}

	fputs ("\n", stdout);
    }

    fputs ("\n", stdout);
}

void
display_board (pos)
     chi_pos* pos;
{
    static char*  buf = NULL;
    static unsigned int bufsize;
    int file;
    int rank;

    printf ("    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	printf (" %c  ", chi_file2char(file));
    }
    printf ("\n   +---+---+---+---+---+---+---+---+\n");

    chi_dump_pieces (pos, &buf, &bufsize);

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	printf (" %c ", chi_rank2char (rank));
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int shift = chi_coords2shift (file, rank);
	    printf ("| %c ", buf[shift]);
	}
	printf ("| ");

	switch (rank) {
	    case CHI_RANK_8:
		if (chi_ep (pos)) {
		    printf ("En passant possible on file %c.", 
			    chi_ep_file (pos) + 'a');
		} else {
		    printf ("En passant not possible.");
		}
		break;
	    case CHI_RANK_7:
		printf ("White king castle: %s.", 
			chi_wk_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_6:
		printf ("White queen castle: %s.", 
			chi_wq_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_5:
		printf ("Black king castle: %s.", 
			chi_bk_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_4:
		printf ("Black queen castle: %s.", 
			chi_bq_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_3:
		printf ("Half move clock (50 moves): %d.", 
			pos->half_move_clock);
		break;
	    case CHI_RANK_2:
		printf ("Half moves: %d.", pos->half_moves);
		break;
	    case CHI_RANK_1:
		printf ("Next move: %s.", 
			chi_to_move (pos) ? "black" : "white");
		break;
	}

	printf ("\n   +---+---+---+---+---+---+---+---+\n");
    }

    printf ("    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	printf (" %c  ", chi_file2char(file));
    }
    printf ("\n");
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
