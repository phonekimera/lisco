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

#include <math.h>

#include <libchi.h>

#include "board.h"
#include "tate.h"

void
dump_board (pos)
     chi_pos* pos;
{
    int rank, file;

    display_board (pos);

    fprintf (stdout, "\n Pieces   Pawns    Knights  Bishops  Rooks    Queens   Kings    Pieces90\n");
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
	    
	    if (pos->w_bishops & ~pos->w_rooks & (1ULL << off))
		fputc ('B', stdout);
	    else if (pos->b_bishops & ~pos->b_rooks & (1ULL << off))
		fputc ('b', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_rooks & ~pos->w_bishops & (1ULL << off))
		fputc ('R', stdout);
	    else if (pos->b_rooks & ~pos->b_bishops & (1ULL << off))
		fputc ('r', stdout);
	    else
		fputc ('.', stdout);
	}

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_rooks & pos->w_bishops & (1ULL << off))
		fputc ('Q', stdout);
	    else if (pos->b_rooks & pos->b_bishops & (1ULL << off))
		fputc ('q', stdout);
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

	fputc (' ', stdout);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_pieces90 & (1ULL << off))
		fputc ('W', stdout);
	    else if (pos->b_pieces90 & (1ULL << off))
		fputc ('b', stdout);
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
		printf ("White queen castle: %s.", 
			chi_wq_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_6:
		printf ("Black queen castle: %s.", 
			chi_bq_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_5:
		printf ("Half moves: %d.", pos->half_moves);
		break;
	    case CHI_RANK_4:
		printf ("Material: %+d.", chi_material (pos));
		break;
	    case CHI_RANK_3:
		if (game_hist[game_hist_ply].castling_state & 0x4)
		    printf ("White has castled: yes (%x).", 
			    game_hist[game_hist_ply].castling_state);
		else
		    printf ("White has castled: no (%x).",
			    game_hist[game_hist_ply].castling_state);
		break;
	}

	fputs ("\n   +---+---+---+---+---+---+---+---+ ", stdout);

	switch (rank) {
	    case CHI_RANK_8:
		printf ("White king castle: %s.", 
			chi_wk_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_7:
		printf ("Black king castle: %s.", 
			chi_bk_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_6:
		printf ("Half move clock (50 moves): %d.", 
			pos->half_move_clock);
		break;
	    case CHI_RANK_5:
		printf ("Next move: %s.", 
			chi_on_move (pos) ? "black" : "white");
		break;
	    case CHI_RANK_4:
		printf ("Zobrist key: 0x%016llx.", 
			chi_zk_signature (zk_handle, pos));
		break;
	    case CHI_RANK_3:
		if (game_hist[game_hist_ply].castling_state & 0x40)
		    printf ("Black has castled: yes (%x).", 
			    game_hist[game_hist_ply].castling_state);
		else
		    printf ("Black has castled: no (%x).",
			    game_hist[game_hist_ply].castling_state);
		break;
	}

	fputc ('\n', stdout);
    }

    printf ("    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	printf (" %c  ", chi_file2char(file));
    }
    printf ("\n");
}

void
print_game ()
{
    unsigned int ply;
    static char* buf = NULL;
    unsigned int bufsize;
    unsigned int chars_in_line = 0;

    if (!game_hist_ply)
	return;

    if (chi_on_move (&game_hist[0].pos) != chi_white)
	chars_in_line += 
	    fprintf (stdout, "%d. ... ", 
		     1 + (game_hist[0].pos.half_moves >> 1));

    for (ply = 0; ply < game_hist_ply; ++ply) {
	unsigned int length;
	int errnum = chi_print_move (&game_hist[ply].pos, game_hist[ply].move,
			&buf, &bufsize, 1);

	if (errnum)
	    fprintf (stderr, "  Error: %s\n", chi_strerror (errnum));

	length = 1 + strlen (buf);

	if (chi_on_move (&game_hist[ply].pos) == chi_white) {
	    length += 2 + (log10 (game_hist[ply].pos.half_moves >> 1));

	    if (chars_in_line + length > 74) {
		chars_in_line = 0;
		fputc ('\n', stdout);
	    }

	    if (chars_in_line) {
		++length;
		fputc (' ', stdout);
	    }

	    fprintf (stdout, "%d.", 1 + (game_hist[ply].pos.half_moves >> 1));
	}

	if (chars_in_line + length > 75) {
	    --length;
	    chars_in_line = 0;
	    fprintf (stdout, "\n%s", buf);
	} else {
	    fprintf (stdout, " %s", buf);
	    chars_in_line += length;
	}
    }

    fputc ('\n', stdout);
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
