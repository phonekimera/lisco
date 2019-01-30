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

#include "tateplay-time-control.h"

START_TEST(test_st_constructor)
	TimeControl tc;

	ck_assert_int_eq(time_control_init_st(&tc, ""), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_true);
END_TEST

Suite *
time_control_suite(void)
{
	Suite *suite;
	TCase *tc_search_time;

	suite = suite_create("Time Control");

	tc_search_time = tcase_create("Fixed search time");
	tcase_add_test(tc_search_time, test_st_constructor);
	suite_add_tcase(suite, tc_search_time);

	return suite;
}
