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

#include "libchi.h"

START_TEST(test_stalemate)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: -9.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   | k |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   | q |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "8/8/8/8/8/6k1/5q2/7K w - - 0 1";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_stalemate);
}
END_TEST

START_TEST(test_white_mates)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: +9.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   | K |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   | Q |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | k |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "8/8/8/8/8/6K1/6Q1/7k b - - 0 1";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_white_mates);
}
END_TEST

START_TEST(test_black_mates)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: -9.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   | k |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   | q |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "8/8/8/8/8/6k1/6q1/7K w - - 0 1";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_black_mates);
}
END_TEST

START_TEST(test_king_king)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   |   |   |   | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/8/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_draw_by_insufficient_material);
}
END_TEST

START_TEST(test_white_king_bishop)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   |   |   | B | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +3.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/7B/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_draw_by_insufficient_material);
}
END_TEST

START_TEST(test_black_king_bishop)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   |   |   | b | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: -3.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/7b/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_draw_by_insufficient_material);
}
END_TEST

START_TEST(test_white_king_knight)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   |   |   | N | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +3.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/7N/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_draw_by_insufficient_material);
}
END_TEST

START_TEST(test_black_king_knight)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   |   |   | n | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: -3.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/7n/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_draw_by_insufficient_material);
}
END_TEST

START_TEST(test_2bishops_equal_color)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   | B |   | b | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/5B1b/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_true);
	ck_assert_int_eq(result, chi_result_draw_by_insufficient_material);
}
END_TEST

START_TEST(test_2bishops_unequal_color)
{
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 10.
 5 |   |   |   |   |   |   | B | b | Half moves: 38.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/6Bb/8/8/8/7K w - - 10 20";
	int errnum = chi_set_position(&pos, fen);
	chi_result result;

	ck_assert_int_eq(errnum, 0);

	/* Check that it is okay to pass a null pointer.  */
	(void) chi_game_over(&pos, NULL);

	ck_assert_int_eq(chi_game_over(&pos, &result), chi_false);
	ck_assert_int_eq(result, chi_result_unknown);
}
END_TEST

Suite *
game_over_suite(void)
{
	Suite *suite;
	TCase *tc_draw;

	suite = suite_create("Game Over Detection");

	tc_draw = tcase_create("Draw");
	tcase_add_test(tc_draw, test_stalemate);
	tcase_add_test(tc_draw, test_white_mates);
	tcase_add_test(tc_draw, test_black_mates);
	tcase_add_test(tc_draw, test_king_king);
	tcase_add_test(tc_draw, test_white_king_bishop);
	tcase_add_test(tc_draw, test_black_king_bishop);
	tcase_add_test(tc_draw, test_white_king_knight);
	tcase_add_test(tc_draw, test_black_king_knight);
	tcase_add_test(tc_draw, test_2bishops_equal_color);
	tcase_add_test(tc_draw, test_2bishops_unequal_color);
	suite_add_tcase(suite, tc_draw);

	return suite;
}
