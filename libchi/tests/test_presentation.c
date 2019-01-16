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

#if CHI_USE_UTF_8

START_TEST(test_char2figurine)
	ck_assert_str_eq(chi_char2figurine('K'), "♔");
	ck_assert_str_eq(chi_char2figurine('Q'), "♕");
	ck_assert_str_eq(chi_char2figurine('R'), "♖");
	ck_assert_str_eq(chi_char2figurine('B'), "♗");
	ck_assert_str_eq(chi_char2figurine('N'), "♘");
	ck_assert_str_eq(chi_char2figurine('P'), "♙");
	ck_assert_str_eq(chi_char2figurine('k'), "♚");
	ck_assert_str_eq(chi_char2figurine('q'), "♛");
	ck_assert_str_eq(chi_char2figurine('r'), "♜");
	ck_assert_str_eq(chi_char2figurine('b'), "♝");
	ck_assert_str_eq(chi_char2figurine('n'), "♞");
	ck_assert_str_eq(chi_char2figurine('p'), "♟");
	ck_assert_str_eq(chi_char2figurine('x'), " ");
END_TEST

#endif /* CHI_USE_UTF_8 */

Suite *
presentation_suite(void)
{
	Suite *suite;
	TCase *tc_figurine;
	
	suite = suite_create("Presentation");

#if CHI_USE_UTF_8
	tc_figurine = tcase_create("Figurine");

	tcase_add_test(tc_figurine, test_char2figurine);

	suite_add_tcase(suite, tc_figurine);
#endif

	return suite;
}
