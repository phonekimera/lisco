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

#include "tateplay-option.h"

START_TEST(test_uci_spin_option)
	const char *input;
	Option *option;

	input = "  name Has \t Space type spin default 1 min 1 max 128    ";

	option = option_uci_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Has \t Space", option->name);
	
	ck_assert_int_eq(option_type_spin, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("1", option->default_value);

	ck_assert_ptr_ne(NULL, option->min);
	ck_assert_str_eq("1", option->min);

	ck_assert_ptr_ne(NULL, option->max);
	ck_assert_str_eq("128", option->max);

	option_destroy(option);

	input = " name Selectivity type spin default 2 min 0 max 4";

	option = option_uci_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Selectivity", option->name);
	
	ck_assert_int_eq(option_type_spin, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("2", option->default_value);

	ck_assert_ptr_ne(NULL, option->min);
	ck_assert_str_eq("0", option->min);

	ck_assert_ptr_ne(NULL, option->max);
	ck_assert_str_eq("4", option->max);

	option_destroy(option);
END_TEST

START_TEST(test_uci_check_option)
	const char *input;
	Option *option;

	input = " name Nullmove type check default true";

	option = option_uci_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Nullmove", option->name);
	
	ck_assert_int_eq(option_type_check, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("true", option->default_value);

	option_destroy(option);
END_TEST

START_TEST(test_uci_combo_option)
	const char *input;
	Option *option;

	input = "name Style type combo default Normal var Solid var Normal var Risky";

	option = option_uci_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Style", option->name);
	
	ck_assert_int_eq(option_type_combo, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("Normal", option->default_value);

	ck_assert_int_eq(3, option->num_vars);
	ck_assert_ptr_ne(NULL, option->vars);

	ck_assert_ptr_ne(NULL, option->vars[0]);
	ck_assert_str_eq("Solid", option->vars[0]);
	ck_assert_ptr_ne(NULL, option->vars[1]);
	ck_assert_str_eq("Normal", option->vars[1]);
	ck_assert_ptr_ne(NULL, option->vars[2]);
	ck_assert_str_eq("Risky", option->vars[2]);
	
	option_destroy(option);
END_TEST

START_TEST(test_stockfish_logfile_option)
	const char *input;
	Option *option;

	input = " name Debug Log File type string default";

	option = option_uci_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Debug Log File", option->name);

	ck_assert_int_eq(option_type_string, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("", option->default_value);

	option_destroy(option);
END_TEST

Suite *
uci_suite(void)
{
	Suite *suite;
	TCase *tc_option;

	suite = suite_create("UCI Functions");

	tc_option = tcase_create("Options");
	tcase_add_test(tc_option, test_uci_spin_option);
	tcase_add_test(tc_option, test_uci_check_option);
	tcase_add_test(tc_option, test_uci_combo_option);
	tcase_add_test(tc_option, test_stockfish_logfile_option);
	suite_add_tcase(suite, tc_option);

	return suite;
}
