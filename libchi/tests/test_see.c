/* This file is part of the chess engine lisco.
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

#include "libchi.h"

#include <check.h>

/* Example from
 * http://mediocrechess.blogspot.com/2007/03/guide-static-exchange-evaluation-see.html
 */
#include <stdio.h>
START_TEST(test_obvious_attackers_lone_pawn)
{
	const char *fen_white = "7k/p7/1p6/8/8/1Q6/8/7K w - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   | k | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 | p |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   | p |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +7.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   | Q |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	chi_pos position;
	int errnum;
	bitv64 white_attackers, black_attackers;
	bitv64 expect_white_attackers, expect_black_attackers;

	errnum = chi_set_position(&position, fen_white);
	ck_assert_int_eq(errnum, 0);

	chi_move move;

	errnum = chi_parse_move(&position, &move, "Qxb6");
	ck_assert_int_eq(errnum, 0);

	expect_white_attackers = 0ULL;
	expect_black_attackers = CHI_A7_MASK;
	chi_obvious_attackers(&position, move, &white_attackers, &black_attackers);
	ck_assert_int_eq(white_attackers, expect_white_attackers);
	ck_assert_int_eq(black_attackers, expect_black_attackers);
}
END_TEST

Suite *
see_suite(void)
{
	Suite *suite;
	TCase *tc_obvious_attackers;

	suite = suite_create("Static Exchange Evaluation");

	tc_obvious_attackers = tcase_create("Obvious Attackers");
	tcase_add_test(tc_obvious_attackers, test_obvious_attackers_lone_pawn);
	suite_add_tcase(suite, tc_obvious_attackers);

	return suite;
}
