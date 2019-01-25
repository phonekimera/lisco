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

START_TEST(test_uci_spin_option)
	const char *input;
	UCIOption *option;

	input = "  name Hash type spin default 1 min 1 max 128    ";

	option = uci_option_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Hash", option->name);

	uci_option_destroy(option);
END_TEST

Suite *
uci_suite(void)
{
	Suite *suite;
	TCase *tc_option;

	suite = suite_create("UCI Functions");

	tc_option = tcase_create("Options");
	tcase_add_test(tc_option, test_uci_spin_option);
	suite_add_tcase(suite, tc_option);

	return suite;
}
