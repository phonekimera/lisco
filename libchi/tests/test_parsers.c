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
	tcase_add_test(tc_bugs, test_parse_move_san_pawn);
    suite_add_tcase(suite, tc_san);

	return suite;
}
