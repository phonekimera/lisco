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

static void fill_move CHI_PARAMS ((chi_pos*, chi_move*));

int
chi_parse_move (pos, move, movestr)
     chi_pos* pos;
     chi_move* move;
     const char* movestr;
{
    int match = 0;

    if (!pos || !move || !movestr || !movestr[0])
	return CHI_ERR_YOUR_FAULT;

    *move = 0;

    /* Castling moves should be okay to parse.  */
    match = parse_castling (pos, move, movestr);

    /* Fully qualified moves: (P/)?e2-e4.  */
    if (match != 0)
	match = parse_fq_move (pos, move, movestr);

    if (match != 0)
	return CHI_ERR_PARSER;

    fill_move (pos, move);

    return 0;
}

static int
parse_castling (pos, move, movestr)
     chi_pos* pos;
     chi_move* move;
     const char* movestr;
{
    /* Castling moves are easy to detect.  */
    if (0 == strncasecmp ("o-o-o", movestr, 5)
	|| 0 == strncmp ("0-0-0", movestr, 5)) {
	if (chi_on_move (pos) == chi_white) {
	    chi_move_set_from (*move, chi_coords2shift (CHI_FILE_E, CHI_RANK_1));
	    chi_move_set_to (*move, chi_coords2shift (CHI_FILE_C, CHI_RANK_1));
	} else {
	    chi_move_set_from (*move, chi_coords2shift (CHI_FILE_E, CHI_RANK_8));
	    chi_move_set_to (*move, chi_coords2shift (CHI_FILE_C, CHI_RANK_8));
	}
	
	return 0;
    }

    if (0 == strncasecmp ("o-o", movestr, 3)
	|| 0 == strncmp ("0-0", movestr, 3)) {
	if (chi_on_move (pos) == chi_white) {
	    chi_move_set_from (*move, chi_coords2shift (CHI_FILE_E, CHI_RANK_1));
	    chi_move_set_to (*move, chi_coords2shift (CHI_FILE_G, CHI_RANK_1));
	} else {
	    chi_move_set_from (*move, chi_coords2shift (CHI_FILE_E, CHI_RANK_8));
	    chi_move_set_to (*move, chi_coords2shift (CHI_FILE_G, CHI_RANK_8));
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

    if (ptr[0] == '=')
	++ptr;
	
    switch (ptr[0]) {
	case 'N':
	case 'K':
	case 'n':
	case 'k':
	    chi_move_set_promote (*move, knight);
	    break;
	case 'B':
	case 'b':
	    chi_move_set_promote (*move, bishop);
	    break;
	case 'R':
	case 'r':
	    chi_move_set_promote (*move, rook);
	    break;
	case 'Q':
	case 'q':
	    chi_move_set_promote (*move, queen);
	    break;
    }

    chi_move_set_from (*move, chi_coords2shift (from_file, from_rank));
    chi_move_set_to (*move, chi_coords2shift (to_file, to_rank));

    return 0;
}

static void
fill_move (pos, move)
     chi_pos* pos;
     chi_move* move;
{
    chi_piece_t attacker = empty;
    chi_piece_t victim = empty;
    chi_piece_t promote = chi_move_promote (*move);
    int is_ep = 0;
    int material = 0;
    int from = chi_move_from (*move);
    int to = chi_move_to (*move);
    bitv64 from_mask = ((bitv64) 1) << from;
    bitv64 to_mask = ((bitv64) 1) << to;

    if (chi_on_move (pos) == chi_white) {
	if (pos->w_pawns & from_mask) {
	    attacker = pawn;
	    if (chi_ep (pos) &&
		(to_mask & CHI_6_MASK) &&
		(!(to_mask & pos->b_pieces)) &&
		(to - from) & 1) {
		is_ep = 1;
		victim = pawn;
		material = 1;
	    } else if (promote) {
		switch (promote) {
		    case queen:
			material = 8;
			break;
		    case rook:
			material = 4;
			break;
		    default:
			material = 2;
			break;
		}
	    }
	} else if (pos->w_knights & from_mask) {
	    attacker = knight;
	} else if (pos->w_bishops & from_mask) {
	    if (pos->w_rooks & from_mask)
		attacker = queen;
	    else 
		attacker = bishop;  
	} else if (pos->w_rooks & from_mask) {
	    attacker = rook;
	} else if (pos->w_kings & from_mask) {
	    attacker = king;
	}

	if (pos->b_pieces & to_mask) {
	    if (pos->b_pawns & to_mask) {
		material += 1;
		victim = pawn;
	    } else if (pos->b_knights & to_mask) {
		material += 3;
		victim = knight;
	    } else if (pos->b_bishops & to_mask) {
		if (pos->b_rooks & to_mask) {
		    material += 9;
		    victim = queen;
		} else {
		    material += 3;
		    victim = bishop;
		}
	    } else if (pos->b_rooks & to_mask) {
		material += 5;
		victim = rook;
	    }
	}
    } else {
	if (pos->b_pawns & from_mask) {
	    attacker = pawn;
	    if (chi_ep (pos) &&
		(to_mask & CHI_3_MASK) &&
		(!(to_mask & pos->w_pieces)) &&
		(from - to) & 1) {
		is_ep = 1;
		victim = pawn;
		material = 1;
	    } else if (promote) {
		switch (promote) {
		    case queen:
			material = 8;
			break;
		    case rook:
			material = 4;
			break;
		    default:
			material = 2;
			break;
		}
	    }
	} else if (pos->b_knights & from_mask) {
	    attacker = knight;
	} else if (pos->b_bishops & from_mask) {
	    if (pos->b_rooks & from_mask)
		attacker = queen;
	    else 
		attacker = bishop;  
	} else if (pos->b_rooks & from_mask) {
	    attacker = rook;
	} else if (pos->b_kings & from_mask) {
	    attacker = king;
	}

	if (pos->w_pieces & to_mask) {
	    if (pos->w_pawns & to_mask) {
		material += 1;
		victim = pawn;
	    } else if (pos->w_knights & to_mask) {
		material += 3;
		victim = knight;
	    } else if (pos->w_bishops & to_mask) {
		if (pos->w_rooks & to_mask) {
		    material += 9;
		    victim = queen;
		} else {
		    material += 3;
		    victim = bishop;
		}
	    } else if (pos->w_rooks & to_mask) {
		material += 5;
		victim = rook;
	    }
	}
    }

    *move |= (material << 22) | (is_ep << 21) | 
	(victim << 15) | (attacker << 12);
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
