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

#include <stdio.h>

#include <check.h>

#include "libchi.h"
#include "../src/lisco.h"

START_TEST(test_movetime)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.movetime = 120000;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.fixed_time, 120000);
	ck_assert(tree.nodes_to_tc > 0);
}
END_TEST

START_TEST(test_nodes)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.nodes = 2304;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.fixed_time, 0);
	ck_assert_uint_eq(tree.nodes_to_tc, 2304);
}
END_TEST

START_TEST(test_depth)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.depth = 3;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.max_depth, 3);
}
END_TEST

START_TEST(test_mate)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.mate = 2;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.max_depth, 3);
}
END_TEST

START_TEST(test_sudden_death)
{
	SearchParams params;
	Tree tree;
	int errnum;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	chi_init_position(&tree.position);
	params.mytime = 60000;
	params.hertime = 60000;

	/* Instead of making exact assumptions about the time allocated for a
	 * move we should just make rough estimates and check tendencies.
	 */

	/* In the initial position we should should assume that we have the maximum
	 * number of moves to go which is 60.
	 */
	process_search_params(&tree, &params);
	ck_assert_uint_eq(tree.fixed_time, 1000);

	const char *fen;

	/* 13 pieces for each side, material imbalance is 0.  The material
	 * balance (weight 0.25) should still indicate 60 moves to go.
	 * The popcount has dropped by 3 of 15.  With a weight of 0.75
	 * that should give us now 0.8 * 0.75 * 60 = 36 moves to go.
	 * 60000 / 36 = 1666.667.
	 */
	fen = "r1bqk2r/ppppbppp/8/1N2R3/8/8/PPPP1PPP/R1BQ2K1 b kq - 0 9";
	/*
	     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | r |   | b | q | k |   |   | r | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 | p | p | p | p | b | p | p | p | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   |   |   |   |   |   |   | Black queen castle: yes.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   | N |   |   | R |   |   |   | Half moves: 17.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P | P | P | P |   | P | P | P |
   +---+---+---+---+---+---+---+---+
 1 | R |   | B | Q |   |   | K |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	errnum = chi_set_position(&tree.position, fen);
	ck_assert_int_eq(errnum, 0);
	process_search_params(&tree, &params);
	ck_assert_uint_eq(tree.fixed_time, 1111);

	/* 13 pieces for each side, material imbalance is 0.  The material
	 * balance (weight 0.25) should still indicate 60 moves to go.
	 * The popcount has dropped by 3 of 15.  With a weight of 0.75
	 * that should give us now 0.8 * 0.75 * 60 = 36 moves to go.
	 * 60000 / 36 = 1666.667.
	 */
	fen = "7k/8/8/8/3RR3/8/8/7K w - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   | k | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   | R | R |   |   |   | Material: +10.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	errnum = chi_set_position(&tree.position, fen);
	ck_assert_int_eq(errnum, 0);
	process_search_params(&tree, &params);
	ck_assert_uint_eq(tree.fixed_time, 3000);

}
END_TEST

Suite *
time_control_suite(void)
{
	Suite *suite;
	TCase *tc_process_search_params;
	TCase *tc_time_allocation;

	suite = suite_create("Time Control");

	tc_process_search_params = tcase_create("Process Parameters");
	tcase_add_test(tc_process_search_params, test_movetime);
	tcase_add_test(tc_process_search_params, test_nodes);
	tcase_add_test(tc_process_search_params, test_depth);
	tcase_add_test(tc_process_search_params, test_mate);
	suite_add_tcase(suite, tc_process_search_params);

	tc_time_allocation = tcase_create("Time Allocation");
	tcase_add_test(tc_time_allocation, test_sudden_death);
	suite_add_tcase(suite, tc_time_allocation);

	return suite;
}
