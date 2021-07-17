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

#define TEST_UCI_ENGINE 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <check.h>

#include "lisco.h"

START_TEST(test_tt_init)
{
	tt_init(100000);
	tt_clear();
	tt_destroy();
}
END_TEST

Suite *
tt_suite(void)
{
	Suite *suite;
	TCase *tc_basic;

	suite = suite_create("Main Transposition table");

	tc_basic = tcase_create("Basic functions");
	tcase_add_test(tc_basic, test_tt_init);
	suite_add_tcase(suite, tc_basic);

	return suite;
}
