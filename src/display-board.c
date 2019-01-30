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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libchi.h"
#include "error.h"
#include "getprogname.h"

#include "display-board.h"
#include "xmalloca-debug.h"

/*
 * FIXME! This is almost an exact copy of that function in src/board.c.  We
 * only omit the zobrist key here.
 */
void
display_board(FILE *stream, chi_pos *pos)
{
	static char*  buf = NULL;
	static unsigned int bufsize;
	int file;
	int rank;

	fprintf(stream, "    ");
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
		if (file == CHI_FILE_H)
			fprintf(stream, " %c", chi_file2char(file));
		else
			fprintf(stream, " %c  ", chi_file2char(file));
	}
	fprintf(stream, "\n   +---+---+---+---+---+---+---+---+\n");

	chi_dump_pieces(pos, &buf, &bufsize);

	for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
		fprintf(stream, " %c ", chi_rank2char (rank));
		for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
			int shift = chi_coords2shift (file, rank);
#if CHI_USE_UTF_8
			fprintf(stream, "| %s ", chi_char2figurine(buf[shift]));
#else
			fprintf(stream, "| %c ", buf[shift]);
#endif
		}
		fprintf(stream, "|");

		switch (rank) {
			case CHI_RANK_8:
				if (chi_ep (pos)) {
					fprintf(stream, " En passant possible on file %c.", 
					chi_ep_file (pos) + 'a');
				} else {
					fprintf(stream, " En passant not possible.");
				}
				break;
			case CHI_RANK_7:
				fprintf(stream, " White queen castle: %s.", 
				chi_wq_castle (pos) ? "yes" : "no");
				break;
			case CHI_RANK_6:
				fprintf(stream, " Black queen castle: %s.", 
				chi_bq_castle (pos) ? "yes" : "no");
				break;
			case CHI_RANK_5:
				fprintf(stream, " Half moves: %d.", pos->half_moves);
				break;
			case CHI_RANK_4:
				fprintf(stream, " Material: %+d.", chi_material (pos));
				break;
			case CHI_RANK_3:
				fprintf(stream, " White has castled: %s.",
				chi_w_castled (pos) ? "yes" : "no");
				break;
		}

		fputs ("\n   +---+---+---+---+---+---+---+---+", stream);

		switch (rank) {
			case CHI_RANK_8:
				fprintf(stream, " White king castle: %s.", 
				chi_wk_castle (pos) ? "yes" : "no");
				break;
			case CHI_RANK_7:
				fprintf(stream, " Black king castle: %s.", 
				chi_bk_castle (pos) ? "yes" : "no");
				break;
			case CHI_RANK_6:
				fprintf(stream, " Half move clock (50 moves): %d.", 
				pos->half_move_clock);
				break;
	    	case CHI_RANK_5:
				fprintf(stream, " Next move: %s.", 
				chi_on_move (pos) ? "black" : "white");
				break;
			case CHI_RANK_4:
				fprintf(stream, " Black has castled: %s.",
				chi_b_castled (pos) ? "yes" : "no");
			break;
		}

		fputc ('\n', stream);
	}

	fprintf(stream, "    ");
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
		if (file == CHI_FILE_H)
			fprintf(stream, " %c", chi_file2char(file));
		else
			fprintf(stream, " %c  ", chi_file2char(file));
	}

	fprintf(stream, "\n");
}
