/* This file is part of the chess engine tate.
 *
 * Copyright (C) 2002-2019 cantanea EOOD.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <libchi.h>

#include "board.h"
#include "tate.h"

void
dump_board (FILE *stream, chi_pos *pos)
{
    int rank, file;

    display_board (stream, pos);

    fprintf (stream, "\n Pieces   Pawns    Knights  Bishops  Rooks    Queens   Kings    Pieces90\n");
    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	
	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);

	    if (pos->w_pieces & (1ULL << off))
		fputc ('W', stream);
	    else if (pos->b_pieces & (1ULL << off))
		fputc ('b', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_pawns & (1ULL << off))
		fputc ('P', stream);
	    else if (pos->b_pawns & (1ULL << off))
		fputc ('p', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_knights & (1ULL << off))
		fputc ('N', stream);
	    else if (pos->b_knights & (1ULL << off))
		fputc ('n', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_bishops & ~pos->w_rooks & (1ULL << off))
		fputc ('B', stream);
	    else if (pos->b_bishops & ~pos->b_rooks & (1ULL << off))
		fputc ('b', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_rooks & ~pos->w_bishops & (1ULL << off))
		fputc ('R', stream);
	    else if (pos->b_rooks & ~pos->b_bishops & (1ULL << off))
		fputc ('r', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_rooks & pos->w_bishops & (1ULL << off))
		fputc ('Q', stream);
	    else if (pos->b_rooks & pos->b_bishops & (1ULL << off))
		fputc ('q', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_kings & (1ULL << off))
		fputc ('K', stream);
	    else if (pos->b_kings & (1ULL << off))
		fputc ('k', stream);
	    else
		fputc ('.', stream);
	}

	fputc (' ', stream);
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    bitv64 off = chi_coords2shift (file, rank);
	    
	    if (pos->w_pieces90 & (1ULL << off))
		fputc ('W', stream);
	    else if (pos->b_pieces90 & (1ULL << off))
		fputc ('b', stream);
	    else
		fputc ('.', stream);
	}
	
	fputs ("\n", stream);
    }

    fputs ("\n", stream);
}

void
display_board (FILE * stream, chi_pos *pos)
{
    static char*  buf = NULL;
    static unsigned int bufsize;
    int file;
    int rank;

    fprintf(stream, "    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	fprintf(stream, " %c  ", chi_file2char(file));
    }
    fprintf(stream, "\n   +---+---+---+---+---+---+---+---+\n");

    chi_dump_pieces (pos, &buf, &bufsize);

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	fprintf(stream, " %c ", chi_rank2char (rank));
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int shift = chi_coords2shift (file, rank);
	    fprintf(stream, "| %c ", buf[shift]);
	}
	fprintf(stream, "| ");

	switch (rank) {
	    case CHI_RANK_8:
		if (chi_ep (pos)) {
		    fprintf(stream, "En passant possible on file %c.", 
			    chi_ep_file (pos) + 'a');
		} else {
		    fprintf(stream, "En passant not possible.");
		}
		break;
	    case CHI_RANK_7:
		fprintf(stream, "White queen castle: %s.", 
			chi_wq_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_6:
		fprintf(stream, "Black queen castle: %s.", 
			chi_bq_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_5:
		fprintf(stream, "Half moves: %d.", pos->half_moves);
		break;
	    case CHI_RANK_4:
		fprintf(stream, "Material: %+d.", chi_material (pos));
		break;
	    case CHI_RANK_3:
		    fprintf(stream, "White has castled: %s.",
			    chi_w_castled (pos) ? "yes" : "no");
		break;
	}

	fputs ("\n   +---+---+---+---+---+---+---+---+ ", stdout);

	switch (rank) {
	    case CHI_RANK_8:
		fprintf(stream, "White king castle: %s.", 
			chi_wk_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_7:
		fprintf(stream, "Black king castle: %s.", 
			chi_bk_castle (pos) ? "yes" : "no");
		break;
	    case CHI_RANK_6:
		fprintf(stream, "Half move clock (50 moves): %d.", 
			pos->half_move_clock);
		break;
	    case CHI_RANK_5:
		fprintf(stream, "Next move: %s.", 
			chi_on_move (pos) ? "black" : "white");
		break;
	    case CHI_RANK_4:
		fprintf(stream, "Zobrist key: 0x%016llx.", 
			chi_zk_signature (zk_handle, pos));
		break;
	    case CHI_RANK_3:
		fprintf(stream, "Black has castled: %s.",
			chi_b_castled (pos) ? "yes" : "no");
		break;
	}

	fputc ('\n', stdout);
    }

    fprintf(stream, "    ");
    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	fprintf(stream, " %c  ", chi_file2char(file));
    }
    fprintf(stream, "\n");
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
