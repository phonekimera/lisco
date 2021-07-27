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

#include <check.h>

#include "lisco.h"

START_TEST(test_fail_high)
{
	/*
	    a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   | q |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   | k |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: -9.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   | K |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	const char *fen = "3q4/3k4/8/8/8/2K5/8/8 b - - 0 1";
	Tree tree;
	int errnum;

	memset(&tree, 0, sizeof tree);
	errnum = chi_set_position(&tree.position, fen);
	ck_assert_int_eq(errnum, 0);

	int value = quiesce(&tree, 5, -50, +50);
	ck_assert_int_eq(value, 50);
}

Suite *
quiescence_suite(void)
{
	Suite *suite;
	TCase *tc_basic;

	suite = suite_create("Quiescence Search");

	tc_basic = tcase_create("Basic");
	tcase_add_test(tc_basic, test_fail_high);
	suite_add_tcase(suite, tc_basic);

	return suite;
}
