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

#include "uci-option.h"

START_TEST(test_uci_option)
	const char *input;
	
	input = "   option name Hash type spin default 1 min 1 max 128    ";

	ck_assert_int_eq(0, 0);
END_TEST

Suite *
uci_suite(void)
{
	Suite *suite;
	TCase *tc_option;

	suite = suite_create("UCI Functions");

	tc_option = tcase_create("Options");
	tcase_add_test(tc_option, test_uci_option);
	suite_add_tcase(suite, tc_option);

	return suite;
}
