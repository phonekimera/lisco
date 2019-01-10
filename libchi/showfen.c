/*
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libchi.h"
#include "error.h"
#include "getprogname.h"
#include "xalloc.h"

/*
 * FIXME! This is almost an exact copy of that function in src/board.c.  We
 * only omit the zobrist key here.
 */
static void
display_board(chi_pos *pos)
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

	chi_dump_pieces(pos, &buf, &bufsize);

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
				printf ("White has castled: %s.",
				chi_w_castled (pos) ? "yes" : "no");
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
				printf ("Black has castled: %s.",
				chi_b_castled (pos) ? "yes" : "no");
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

int
main (int argc, char* argv[])
{
	size_t fen_length = 0;
	size_t arg_length;
	int i;
	char *fen;
	char *ptr;
	int errnum;
	chi_pos pos;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s FEN_POSITION\n\n", getprogname());
		fprintf(stderr, "There is no need to quote FEN_POSITION.\n");
		return EXIT_FAILURE;
	}

	for (i = 1; i < argc; ++i) {
		fen_length += strlen(argv[i]) + 1;
	}

	fen = ptr = xmalloc(fen_length);

	*ptr = '\0';
	for (i = 1; i < argc; ++i) {
		arg_length = strlen(argv[i]);
		strcpy(ptr, argv[i]);
		ptr += arg_length;
		if (i + 1 < argc) {
			*ptr++ = ' ';
		}
	}

	errnum = chi_set_position(&pos, fen);
	if (errnum)
		error(EXIT_FAILURE, 0, "%s: %s", fen, chi_strerror(errnum));

	display_board(&pos);

	return EXIT_SUCCESS;
}
