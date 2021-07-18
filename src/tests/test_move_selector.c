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

START_TEST(test_basic)
{
	const char *fen = "1B1bR3/1K1pn2P/4kN2/6Qp/7p/7P/8/8 w - - 0 1";
	/*
	    a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   | B |   | b | R |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   | K |   | p | n |   |   | P | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   | k | N |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   | Q | p | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   | p | Material: +12.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   | P | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	Tree tree;
	int errnum;
	chi_move bestmove;

	errnum = chi_set_position(&tree.position, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&tree.position, &bestmove, "Bd6");
	ck_assert_int_eq(errnum, 0);

	MoveSelector selector;
	move_selector_init(&selector, &tree, bestmove);
	ck_assert_int_eq(selector.num_moves, 45);
	ck_assert_int_eq(selector.selected, 0);

	chi_move move;

	move = move_selector_next(&selector);
	ck_assert_uint_eq(move, bestmove);

	char *buf = NULL;
	unsigned int bufsize = 0;

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "h8=Q");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "h8=R");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "Rxd8");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "Rxe7+");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "h8=B");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "h8=N");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "Nxd7");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "Nxh5");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "Qxh5");

	move = move_selector_next(&selector);
	errnum = chi_print_move(&tree.position, move, &buf, &bufsize, 1);
	ck_assert_int_eq(errnum, 0);
	ck_assert_str_eq(buf, "Qxh4");

	free(buf);

	size_t i = 0;
	while (move_selector_next(&selector))
		++i;
	ck_assert_int_eq(i, 34);
}
END_TEST

Suite *
move_selector_suite(void)
{
	Suite *suite;
	TCase *tc_basic;

	suite = suite_create("Move Selector");

	tc_basic = tcase_create("Basic");
	tcase_add_test(tc_basic, test_basic);
	suite_add_tcase(suite, tc_basic);

	return suite;
}
