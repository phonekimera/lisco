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

START_TEST(test_fixed_time)
{
}
END_TEST

Suite *
time_control_suite(void)
{
	Suite *suite;
	TCase *tc_process_search_params;

	suite = suite_create("Time Control");

	tc_process_search_params = tcase_create("Process Parameters");
	tcase_add_test(tc_process_search_params, test_fixed_time);
	suite_add_tcase(suite, tc_process_search_params);

	return suite;
}
