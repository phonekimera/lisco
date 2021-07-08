/* This file is part of the chess engine tate.
 *
 * Copyright (C) 2002-2021 cantanea EOOD.
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
#include <string.h>

#include "edit.h"

static chi_color_t edit_color = chi_white;

void
init_edit_mode (pos)
     chi_pos* pos;
{
    edit_color = chi_white;

    fputs (". Exit to main\n", stdout);
    fputs ("# Clear bord\n", stdout);
    fputs ("c Change sides\n", stdout);
    fputs ("display Display board\n", stdout);
    fputs ("quit    Terminate program\n", stdout);
}

int
check_position (pos, is_fen)
     chi_pos* pos;
     int is_fen;
{
    if (!is_fen) {
	/* Heuristic for castling status.  */
	chi_ep (pos) = 0;
	chi_wk_castle (pos) = chi_wq_castle (pos) =
	    chi_bk_castle (pos) = chi_bq_castle (pos) = 0;

	if (pos->w_kings & CHI_FILE_E & CHI_RANK_1) {
	    if (pos->w_rooks & ~pos->w_bishops & CHI_FILE_A & CHI_RANK_1)
		chi_wq_castle (pos) = 1;

	    if (pos->w_rooks & ~pos->w_bishops & CHI_FILE_A & CHI_RANK_1)
		chi_wk_castle (pos) = 1;
	}

	if (pos->b_kings & CHI_FILE_E & CHI_RANK_8) {
	    if (pos->b_rooks & ~pos->b_bishops & CHI_FILE_A & CHI_RANK_8)
		chi_bq_castle (pos) = 1;

	    if (pos->w_rooks & ~pos->b_bishops & CHI_FILE_A & CHI_RANK_1)
		chi_bk_castle (pos) = 1;
	}
    }

    chi_update_material (pos);

    return 0;
}

#define CHAR2PIECE(c) \
    (c) == 'Q' || (c) == 'q' ? queen : \
    (c) == 'R' || (c) == 'r' ? rook : \
    (c) == 'B' || (c) == 'b' ? bishop : \
    (c) == 'N' || (c) == 'n' ? knight : \
    (c) == 'P' || (c) == 'p' ? pawn : \
    (c) == 'K' || (c) == 'k' ? king : empty

void
handle_edit_command (pos, cmd)
     chi_pos* pos;
     const char* cmd;
{
    switch (cmd[0]) {
	case 'c':
	case 'C':
	    edit_color = !edit_color;
	    break;
	case '#':
	    chi_clear_position (pos);
	    break;
	case 'P':
	case 'p':
	case 'N':
	case 'n':
	case 'B':
	case 'b':
	case 'R':
	case 'r':
	case 'Q':
	case 'q':
	case 'K':
	case 'k':
	case 'X':
	case 'x':
	    if (((cmd[1] >= 'a' && cmd[1] <= 'h')
		 || (cmd[1] >= 'A' && cmd[1] <= 'H'))
		&& cmd[2] >= '1' && cmd[2] <= '8') {
		int file_char = cmd[1];
		chi_piece_t piece = CHAR2PIECE (cmd[0]);
		int file, rank;

		if (file_char <= 'H')
		    file_char += 'a' - 'A';
		file = file_char - 'a';
		rank = cmd[2] - '1';

		if (0 != chi_set_piece (pos, piece, edit_color, file, rank)) {
		    fprintf (stdout, "Error (illegal edit command): %s\n",
			     cmd);
		}
		break;
	    }
	    /* FALLTHRU.  */
	default:
	    fprintf (stdout, "Error (illegal edit command): %s\n",
		     cmd);
	    break;
    }
}

#if 0
/* FIXME: Belongs into libchi!  */
#define WHITE_SPACE " \t\r\n\v\f"

void
setboard (pos, position)
     chi_pos* pos;
     char* position;
{
    char* piece_str = strtok (position, WHITE_SPACE);
    char* active_color_str;
    char* castling_str;
    char* en_passant_str;
    int en_passant;
#if 0
    char* half_move_str;
    char* full_move_str;
#endif
    chi_pos new_pos;
    int rank = eight;
    int file = a;
    int i = 0;

    error (EXIT_FAILURE, 0, "not yet implemented");

    if (piece_str == NULL) {
	fprintf (stderr, "Error (syntax error): %s\n", position);
	return;
    }

    memset (&new_chi_pos, 0, sizeof new_chi_pos);

    while (piece_str[i]) {
	int offset = rank * 8 + file;
	bitv mask = 1ULL << offset;

	switch (piece_str[i]) {
	    case 'P':
		new_chi_pos.wp |= mask;
		new_chi_pos.w_pieces = mask;
		new_chi_pos.squares[offset] = pawn;
		break;
	    case 'p':
		new_chi_pos.bp |= mask;
		new_chi_pos.b_pieces = mask;
		new_chi_pos.squares[offset] = pawn;
		break;
	    case 'N':
		new_chi_pos.wn |= mask;
		new_chi_pos.w_pieces = mask;
		new_chi_pos.squares[offset] = knight;
		break;
	    case 'n':
		new_chi_pos.bn |= mask;
		new_chi_pos.b_pieces = mask;
		new_chi_pos.squares[offset] = knight;
		break;
	    case 'B':
		new_chi_pos.wb |= mask;
		new_chi_pos.w_pieces = mask;
		new_chi_pos.squares[offset] = bishop;
		break;
	    case 'b':
		new_chi_pos.bb |= mask;
		new_chi_pos.b_pieces = mask;
		new_chi_pos.squares[offset] = bishop;
		break;
	    case 'R':
		new_chi_pos.wr |= mask;
		new_chi_pos.w_pieces = mask;
		new_chi_pos.squares[offset] = rook;
		break;
	    case 'r':
		new_chi_pos.br |= mask;
		new_chi_pos.b_pieces = mask;
		new_chi_pos.squares[offset] = rook;
		break;
	    case 'Q':
		new_chi_pos.wb |= mask;
		new_chi_pos.wr |= mask;
		new_chi_pos.w_pieces = mask;
		new_chi_pos.squares[offset] = queen;
		break;
	    case 'q':
		new_chi_pos.bb |= mask;
		new_chi_pos.br |= mask;
		new_chi_pos.b_pieces = mask;
		new_chi_pos.squares[offset] = queen;
		break;
	    case 'K':
		new_chi_pos.wk |= mask;
		new_chi_pos.w_pieces = mask;
		new_chi_pos.squares[offset] = king;
		break;
	    case 'k':
		new_chi_pos.bk |= mask;
		new_chi_pos.b_pieces = mask;
		new_chi_pos.squares[offset] = king;
		break;
	    case '/':
		--rank;
		file = a - 1;
		if (rank < one) {
		    fprintf (stdout, "Error (more than eight ranks): %s\n",
			     position);
		    return;
		}
		break;
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
		file += piece_str[i] - '1';
		if (file > h + 1) {
		    fprintf (stdout, "Error (more than eight files: %s\n",
			     position);
		    return;
		}
		break;
	    default:
		fprintf (stdout, 
			 "\
Error (illegal position): illegal character at position %d: %s\n", 
			 i, position);
		return;
	}
	++i;
	++file;
    }

    active_color_str = strtok (NULL, WHITE_SPACE);
    switch (active_color_str[0]) {
	case '\000':
	    fprintf (stdout, "Error (no active color)\n");
	    return;
	case 'w':
	case 'W':
	    new_chi_pos.to_move = white;
	    break;
	case 'b':
	case 'B':
	    new_chi_pos.to_move = black;
	    break;
	default:
	    fprintf (stdout, "Error (invalid active color): %s\n",
		     active_color_str);
    }

    castling_str = strtok (NULL, WHITE_SPACE);
    i = 0;
    while (castling_str[i]) {
	switch (castling_str[i]) {
	    case 'K':
		new_chi_pos.wk_castle = 1;
		break;
	    case 'k':
		new_chi_pos.bk_castle = 1;
		break;
	    case 'Q':
		new_chi_pos.wq_castle = 1;
		break;
	    case 'q':
		new_chi_pos.bq_castle = 1;
		break;
	}
	break;
    }

    en_passant_str = strtok (NULL, WHITE_SPACE);

    if (en_passant_str 
	&& en_passant_str[0] >= 'a' && en_passant_str[0] <= 'h') {
	en_passant = en_passant_str[0] - 'a';
	if (en_passant_str[1] == '3' && new_chi_pos.to_move == white)
	    new_chi_pos.en_passant = three * 8 + en_passant;
	else if (en_passant_str[1] == '6' && new_chi_pos.to_move == black)
	    new_chi_pos.en_passant = six * 8 + en_passant;
    }

    // FIXME: Evaluate rest of FEN.
    if (0 == check_position (&new_chi_pos, 1))
	*pos = new_chi_pos;
}

#endif
