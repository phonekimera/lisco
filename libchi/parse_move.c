/* parse_move.c - Parse textual representation of a move.
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

#include <libchi.h>

static int parse_castling CHI_PARAMS ((chi_pos*, chi_move*, const char*));
static int parse_fq_move CHI_PARAMS ((chi_pos*, chi_move*, const char*));

int
chi_parse_move (pos, move, movestr)
     chi_pos* pos;
     chi_move* move;
     const char* movestr;
{
    int match = 0;

    if (!pos || !move || !movestr || !movestr[0])
	return CHI_ERR_YOUR_FAULT;

    move->packed = 0;

    /* Castling moves should be okay to parse.  */
    match = parse_castling (pos, move, movestr);
    if (match >= 0)
	return match;

    /* Fully qualified moves: (P/)?e2-e4.  */
    match = parse_fq_move (pos, move, movestr);
    if (match >= 0)
	return match;

    return CHI_ERR_PARSER;
}

static int
parse_castling (pos, move, movestr)
     chi_pos* pos;
     chi_move* move;
     const char* movestr;
{
    /* Castling moves are easy to detect.  */
    if (0 == strncasecmp ("o-o-o", movestr, 5)) {
	if (chi_to_move (pos) == chi_white) {
	    move->fields.from = chi_coords2shift (CHI_FILE_E, CHI_RANK_1);
	    move->fields.to = chi_coords2shift (CHI_FILE_C, CHI_RANK_1);
	} else {
	    move->fields.from = chi_coords2shift (CHI_FILE_E, CHI_RANK_8);
	    move->fields.to = chi_coords2shift (CHI_FILE_C, CHI_RANK_8);
	}
	
	return 0;
    }

    if (0 == strncasecmp ("o-o", movestr, 3)) {
	if (chi_to_move (pos) == chi_white) {
	    move->fields.from = chi_coords2shift (CHI_FILE_E, CHI_RANK_1);
	    move->fields.to = chi_coords2shift (CHI_FILE_G, CHI_RANK_1);
	} else {
	    move->fields.from = chi_coords2shift (CHI_FILE_E, CHI_RANK_8);
	    move->fields.to = chi_coords2shift (CHI_FILE_G, CHI_RANK_8);
	}

	return 0;
    }

    return -1;
}

static int
parse_fq_move (pos, move, movestr)
     chi_pos* pos;
     chi_move* move;
     const char* movestr;
{
    chi_piece_t piece = pawn;
    chi_piece_t promote = promote;
    int from_file = -1;
    int from_rank = -1;
    int to_file = -1;
    int to_rank = -1;
    char* ptr = (char*) movestr;  /* We will not touch the const, promised.  */

    switch (ptr[0]) {
	case 'P':
	    ++ptr;
	    break;
	case 'N':
	    ++ptr;
	    piece = knight;
	    break;
	case 'B':
	    /* FIXME: Might be B2-B4... */
	    ++ptr;
	    piece = bishop;
	    break;
	case 'R':
	    ++ptr;
	    piece = rook;
	    break;
	case 'Q':
	    ++ptr;
	    piece = queen;
		break;
	case 'K':
	    ++ptr;
	    piece = king;
	    break;
    }

    if (ptr != movestr) {
	/* Maybe an additional slash.  */
	if (ptr[0] == '/')
	    ++ptr;
    }

    switch (ptr[0]) {
	case '\0':
	    return -1;
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	    from_file = ptr[0] - 'a';
	    ++ptr;
	    break;
	default:
	    return -1;
    }

    switch (ptr[0]) {
	case '\0':
	    return -1;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	    from_rank = ptr[0] - '1';
	    ++ptr;
	    break;
	default:
	    return -1;
    }

    switch (ptr[0]) {
	case '-':
	case 'x':
	case ':':
	    ++ptr;  /* Discard.  */
	    break;
    }

    switch (ptr[0]) {
	case '\0':
	    return -1;
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	    to_file = ptr[0] - 'a';
	    ++ptr;
	    break;
	default:
	    return -1;
    }

    switch (ptr[0]) {
	case '\0':
	    return -1;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	    to_rank = ptr[0] - '1';
	    ++ptr;
	    break;
	default:
	    return -1;
    }

    if (ptr[0] == '=') {
	++ptr;
	
	switch (ptr[0]) {
	    case 'N':
	    case 'K':
		move->fields.promote = knight;
		break;
	case 'B':
	    move->fields.promote = bishop;
	    break;
	    case 'R':
		move->fields.promote = rook;
		break;
	    case 'Q':
		move->fields.promote = queen;
		break;
	    default: return -1;
	}
    }

    move->fields.from = chi_coords2shift (from_file, from_rank);
    move->fields.to = chi_coords2shift (to_file, to_rank);

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
