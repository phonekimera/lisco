/* This file is part of the chess engine tate.
 *
 * Copyright (C) 2002-2019 cantanea EOOD.
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

START_TEST(test_pawn_move)
	chi_move move = chi_coords2shift(4, 1)
	                | chi_coords2shift(4, 3) << 6
			| ((~pawn & 0x7) << 13);
	char *buf = NULL;
	unsigned int bufsize;
	int errnum;

	errnum = chi_coordinate_notation(move, chi_white, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
	ck_assert_ptr_ne(buf, NULL);
	ck_assert_str_eq(buf, "e2e4");
	ck_assert_int_ge(bufsize, 5);
	free(buf);
END_TEST

START_TEST(test_knight_move)
	chi_move move = chi_coords2shift(6, 0)
	                | chi_coords2shift(5, 2) << 6
			| ((~knight & 0x7) << 13);
	char *buf = NULL;
	unsigned int bufsize;
	int errnum;

	errnum = chi_coordinate_notation(move, chi_white, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
	ck_assert_ptr_ne(buf, NULL);
	ck_assert_str_eq(buf, "g1f3");
	ck_assert_int_ge(bufsize, 5);
	free(buf);
END_TEST

START_TEST(test_castling)
	chi_move move;

	char *buf = NULL;
	unsigned int bufsize;
	int errnum;

	move = chi_coords2shift(4, 0)
	       | (chi_coords2shift(6, 0) << 6)
	       | ((~king & 0x7) << 13);
	errnum = chi_coordinate_notation(move, chi_white, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
	ck_assert_ptr_ne(buf, NULL);
	ck_assert_str_eq(buf, "O-O");
	ck_assert_int_ge(bufsize, 5);

	move = chi_coords2shift(4, 0)
	       | chi_coords2shift(2, 0) << 6
               | ((~king & 0x7) << 13);
	errnum = chi_coordinate_notation(move, chi_white, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
	ck_assert_ptr_ne(buf, NULL);
	ck_assert_str_eq(buf, "O-O-O");
	ck_assert_int_ge(bufsize, 5);

	move = chi_coords2shift(4, 7)
	       | chi_coords2shift(6, 7) << 6
               | ((~king & 0x7) << 13);
	errnum = chi_coordinate_notation(move, chi_black, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
	ck_assert_ptr_ne(buf, NULL);
	ck_assert_str_eq(buf, "O-O");
	ck_assert_int_ge(bufsize, 5);

	move = chi_coords2shift(4, 7)
	       | chi_coords2shift(2, 7) << 6
               | ((~king & 0x7) << 13);
	errnum = chi_coordinate_notation(move, chi_black, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
	ck_assert_ptr_ne(buf, NULL);
	ck_assert_str_eq(buf, "O-O-O");
	ck_assert_int_ge(bufsize, 5);

	free(buf);
END_TEST

Suite *
coordinate_notation_suite(void)
{
	Suite *suite;
	TCase *tc_simple;
	TCase *tc_special;

	suite = suite_create("Moves in coordinate notation");

	tc_simple = tcase_create("Simple moves");
	tcase_add_test(tc_simple, test_pawn_move);
	tcase_add_test(tc_simple, test_knight_move);
	suite_add_tcase(suite, tc_simple);

	tc_special = tcase_create("Special moves");
	tcase_add_test(tc_special, test_castling);
	suite_add_tcase(suite, tc_special);

	return suite;
}
