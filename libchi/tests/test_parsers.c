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

#include <check.h>

#include "libchi.h"

START_TEST(test_parse_move_san_bug)
	chi_pos pos;
	chi_move move;
/*      a   b   c   d   e   f   g   h  
   +---+---+---+---+---+---+---+---+
 8 | r |   |   |   |   |   | k |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   | p |   |   | b |   | p | p | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 | p |   |   | p |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 1.
 5 |   |   | p | q | p | r |   |   | Half moves: 37.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: -1.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   | P |   | P | Q | N |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+ 
 2 | P |   |   |   |   | P | P | P | 
   +---+---+---+---+---+---+---+---+ 
 1 |   | R | R |   |   |   |   | K | 
   +---+---+---+---+---+---+---+---+ 
     a   b   c   d   e   f   g   h  
 */
	const char *fen = "r5k1/1p2b1pp/p2p4/2pqpr2/8/1P1PQN2/P4PPP/1RR4K b - - 1 19";
	int errnum = chi_set_position(&pos, fen);

	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move (&pos, &move, "Ra8f8");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(0, 7));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 7));
	/* Same move now in SAN.  */
#if 0
	errnum = chi_parse_move (&pos, &move, "Raf8");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(0, 7));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 7));
#endif
END_TEST

START_TEST(test_parse_move_san_pawn)
	chi_pos pos;
	chi_move move;
	int errnum;
	
	chi_init_position(&pos);

	errnum = chi_parse_move (&pos, &move, "e4");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 1));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(4, 3));
END_TEST

START_TEST(test_parse_move_san_piece)
	chi_pos pos;
	chi_move move;
	int errnum;
	
	chi_init_position(&pos);

	errnum = chi_parse_move (&pos, &move, "Nc3");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(1, 0));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(2, 2));
END_TEST

START_TEST(test_parse_move_san_piece_capture)
	chi_pos pos;
	chi_move move;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | r |   | b | q | k | b | n | r | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: yes.
 7 | p | p | p | p |   | p | p | p | White queen castle: yes.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   | n |   |   |   |   |   | Black queen castle: yes.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 2.
 5 |   |   |   |   | p |   |   |   | Half moves: 4.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   | P |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   | N |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P | P | P | P |   | P | P | P |
   +---+---+---+---+---+---+---+---+
 1 | R | N | B | Q | K | B |   | R |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3";
	int errnum = chi_set_position(&pos, fen);

	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move (&pos, &move, "Nxe5");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(5, 2));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(4, 4));
	errnum = chi_parse_move (&pos, &move, "N:e5");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(5, 2));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(4, 4));
	errnum = chi_parse_move (&pos, &move, "Ne5:");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(5, 2));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(4, 4));
END_TEST


START_TEST(test_parse_move_san_ambiguous_pawn_capture)
	chi_pos pos;
	chi_move move;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | r | n | b | q | k | b | n | r | En passant possible on file d.
   +---+---+---+---+---+---+---+---+ White king castle: yes.
 7 | p | p | p |   |   | p | p | p | White queen castle: yes.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   |   |   |   |   |   |   | Black queen castle: yes.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   | p | p |   |   |   | Half moves: 4.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   | P |   | P |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P | P |   | P |   | P | P | P |
   +---+---+---+---+---+---+---+---+
 1 | R | N | B | Q | K | B | N | R |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "rnbqkbnr/ppp2ppp/8/3pp3/2P1P3/8/PP1P1PPP/RNBQKBNR w KQkq d6 0 3";
	int errnum = chi_set_position(&pos, fen);

	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move (&pos, &move, "cxd");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(2, 3));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(3, 4));
	errnum = chi_parse_move (&pos, &move, "cd:");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(2, 3));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(3, 4));
	errnum = chi_parse_move (&pos, &move, "c:d");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(2, 3));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(3, 4));

/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   | k |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   | n |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   | P | b |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   | K | P |   |   |   | Material: -4.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	fen = "4k3/8/5n2/4Pb2/3KP3/8/8/8 w - - 0 1";

	errnum = chi_set_position(&pos, fen);

	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move (&pos, &move, "exf6");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 4));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 5));
	errnum = chi_parse_move (&pos, &move, "e:f6");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 4));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 5));
	errnum = chi_parse_move (&pos, &move, "ef6:");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 4));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 5));
	errnum = chi_parse_move (&pos, &move, "exf5");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 3));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 4));
	errnum = chi_parse_move (&pos, &move, "e:f5");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 3));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 4));
	errnum = chi_parse_move (&pos, &move, "ef5:");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(4, 3));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 4));

	/* Test that move is completely specified.  */
	errnum = chi_parse_move (&pos, &move, "exf");
	ck_assert_int_eq(errnum, CHI_ERR_AMBIGUOUS_MOVE);
	errnum = chi_parse_move (&pos, &move, "e:f");
	ck_assert_int_eq(errnum, CHI_ERR_AMBIGUOUS_MOVE);
	errnum = chi_parse_move (&pos, &move, "ef:");
	ck_assert_int_eq(errnum, CHI_ERR_AMBIGUOUS_MOVE);

END_TEST

Suite *
parsers_suite(void)
{
	Suite *suite;
	TCase *tc_bugs;
    TCase *tc_san;
	
	suite = suite_create("Parsers");

	tc_bugs = tcase_create("Bugs");
	tcase_add_test(tc_bugs, test_parse_move_san_bug);
	suite_add_tcase(suite, tc_bugs);

	tc_san = tcase_create("SAN");
	tcase_add_test(tc_san, test_parse_move_san_pawn);
	tcase_add_test(tc_san, test_parse_move_san_piece);
	tcase_add_test(tc_san, test_parse_move_san_piece_capture);
	tcase_add_test(tc_san, test_parse_move_san_ambiguous_pawn_capture);
	suite_add_tcase(suite, tc_san);

	return suite;
}
