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

#include <stddef.h>
#include <string.h>
#include <strings.h>

#include <libchi.h>

static int parse_castling(chi_pos*, chi_move*, const char*);
static int parse_fq_move(chi_pos*, chi_move*, const char*);
static int parse_san_move(chi_pos*, chi_move*, const char*);

static void fill_move(chi_pos*, chi_move*);

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

    /* Fully qualified moves: (P/)?e2-e4.  */
    if (match != 0)
	match = parse_san_move (pos, move, movestr);

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

static int
parse_san_move (pos, move, movestr)
     chi_pos* pos;
     chi_move* move;
     const char* movestr;
{
    chi_piece_t piece = pawn;
    chi_piece_t promote = empty;
    int from_file = -1;
    int from_rank = -1;
    int to_file = -1;
    int to_rank = -1;
    int is_capture = 0;
    int is_check = 0;
    char* ptr = (char*) movestr;  /* We will not touch the const, promised.  */
    chi_move legalmoves[CHI_MAX_MOVES];
    chi_move* move_end;
    chi_move* mv;
    chi_move* match = NULL;
    int check_matched = 0;
    int capture_matched = 0;
    int enough = 0;

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
    }

    if (from_file >= 0) {
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
	}
    }

    if (ptr[0] == 'x' || ptr[0] == ':') {
	is_capture = 1;
	++ptr;
    } else if (ptr[0] == '-') {
	++ptr;
    }

    if (from_file >= 0 && from_rank >= 0) {
	to_file = from_file;
	to_rank = from_rank;
	from_file = from_rank = -1;
    }

    if (to_file < 0 || to_rank < 0) {
	switch (ptr[0]) {
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
		if (piece != pawn)
		    return -1;
		break;
	}

	switch (ptr[0]) {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
		to_rank = ptr[0] - '1';
		++ptr;
		break;	
	    default:
		if (piece != pawn)
		    return -1;
		break;
	}
    }

    if (ptr[0] == ':') {
	++ptr;
	is_capture = 1;
    }

    if (piece == pawn && from_file >= 0 && 
	to_file >= 0 && from_file != to_file)
	is_capture = 1;

    if (ptr[0] == '=') {
	++ptr;
	
	switch (ptr[0]) {
	    case 'N':
	    case 'K':
	    case 'n':
	    case 'k':
		promote = knight;
		break;
	    case 'B':
	    case 'b':
		promote = knight;
		break;
	    case 'R':
	    case 'r':
		promote = knight;
		break;
	    case 'Q':
	    case 'q':
		promote = queen;
		break;
	}
    }

    if (ptr[0] == '+' || ptr[0] == '#') {
	is_check = 1;
	++ptr;
    }

#if 0
    fprintf (stderr, "  Piece: %d\n", piece);
    fprintf (stderr, "  From: (%d|%d)\n", from_file, from_rank);
    fprintf (stderr, "  To: (%d|%d)\n", to_file, to_rank);
    fprintf (stderr, "  Capture: %s\n", is_capture ? "yes" : "no");
    fprintf (stderr, "  Promote: %d\n", promote);
    fprintf (stderr, "  Check: %s\n", is_check ? "yes" : "no");
#endif

    if (from_file >= 0)
	++enough;
    if (to_file >= 0)
	++enough;
    if (from_rank >= 0)
	++enough;
    if (to_rank >= 0)
	++enough;
    if (enough < 2)
	return -1;

    move_end = chi_legal_moves (pos, legalmoves);

    for (mv = legalmoves; mv < move_end; ++mv) {
	int move_from = chi_move_from (*mv);
	int move_to = chi_move_to (*mv);
	int move_from_file = 7 - move_from % 8;
	int move_from_rank = move_from / 8;
	int move_to_file = 7 - move_to % 8;
	int move_to_rank = move_to / 8;
	chi_piece_t move_promote = chi_move_promote (*mv);
	chi_piece_t move_attacker = chi_move_attacker (*mv);
	chi_pos tmp_pos = *pos;
	chi_piece_t move_victim = chi_move_victim (*mv);
	int capture_matches;
	int check_matches;
	int move_is_check;

	/* Minimum requirements.  */
	if ((from_file >= 0) && (from_file != move_from_file))
	    continue;
	if ((to_file >= 0) && (to_file != move_to_file))
	    continue;
	if ((from_rank >= 0) && (from_rank != move_from_rank))
	    continue;
	if ((to_rank >= 0) && (to_rank != move_to_rank))
	    continue;
	if (promote != move_promote)
	    continue;
	if (piece != move_attacker)
	    continue;

	if (move_victim && is_capture)
	    capture_matches = 1;
	else if ((move_victim == empty) && !is_capture)
	    capture_matches = 1;
	else
	    capture_matches = 0;

	chi_apply_move (&tmp_pos, *mv);
	move_is_check = chi_check_check (&tmp_pos);

	if (move_is_check == is_check)
	    check_matches = 1;
	else
	    check_matches = 0;

	if (match) {
	    /* Possible ambiguity.  */
	    if (capture_matched && check_matched &&
		(!capture_matches || !check_matches))
		continue;
	}

	capture_matched = capture_matches;
	check_matched = check_matches;
	match = mv;
    }

    if (!match)
	return -1;

    *move = *match;

    return 0;
}

static void
fill_move (pos, move)
     chi_pos* pos;
     chi_move* move;
{
    chi_piece_t attacker = ~empty & 0x7;
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
	    attacker = ~pawn & 0x7;
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
	    attacker = ~knight & 0x7;
	} else if (pos->w_bishops & from_mask) {
	    if (pos->w_rooks & from_mask)
		attacker = ~queen & 0x7;
	    else 
		attacker = ~bishop & 0x7;  
	} else if (pos->w_rooks & from_mask) {
	    attacker = ~rook & 0x7;
	} else if (pos->w_kings & from_mask) {
	    attacker = ~king & 0x7;
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
	    attacker = ~pawn & 0x7;
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
	    attacker = ~knight & 0x7;
	} else if (pos->b_bishops & from_mask) {
	    if (pos->b_rooks & from_mask)
		attacker = ~queen & 0x7;
	    else 
		attacker = ~bishop & 0x7;  
	} else if (pos->b_rooks & from_mask) {
	    attacker = ~rook & 0x7;
	} else if (pos->b_kings & from_mask) {
	    attacker = ~king & 0x7;
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

    *move |= (material << 22) | (is_ep << 12) | 
	(victim << 16) | (attacker << 13);
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
