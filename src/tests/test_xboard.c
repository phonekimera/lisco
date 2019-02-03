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
#include "xboard-feature.h"

START_TEST(test_xboard_feature)
	const char *input;
	XboardFeature *feature;
	const char *endptr;

	input = "done=0 other=42";
	feature = xboard_feature_new(input, NULL);
	ck_assert_ptr_ne(feature, NULL);
	ck_assert_ptr_ne(feature->name, NULL);
	ck_assert_str_eq("done", feature->name);
	ck_assert_ptr_ne(feature->value, NULL);
	ck_assert_str_eq("0", feature->value);
	xboard_feature_destroy(feature);

	input = "done=0 other=42";
	feature = xboard_feature_new(input, &endptr);
	ck_assert_ptr_ne(feature, NULL);
	ck_assert_ptr_ne(feature->name, NULL);
	ck_assert_str_eq("done", feature->name);
	ck_assert_ptr_ne(feature->value, NULL);
	ck_assert_str_eq("0", feature->value);
	ck_assert_ptr_eq(input + 6, endptr);
	xboard_feature_destroy(feature);

	input = " \t done  =\t0 other=42";
	feature = xboard_feature_new(input, &endptr);
	ck_assert_ptr_ne(feature, NULL);
	ck_assert_ptr_ne(feature->name, NULL);
	ck_assert_str_eq("done", feature->name);
	ck_assert_ptr_ne(feature->value, NULL);
	ck_assert_str_eq("0", feature->value);
	ck_assert_ptr_eq(input + 12, endptr);
	xboard_feature_destroy(feature);

	input = "name=\"Tate 1.2.3\"   ";
	feature = xboard_feature_new(input, &endptr);
	ck_assert_ptr_ne(feature, NULL);
	ck_assert_ptr_ne(feature->name, NULL);
	ck_assert_str_eq("name", feature->name);
	ck_assert_ptr_ne(feature->value, NULL);
	ck_assert_str_eq("Tate 1.2.3", feature->value);
	ck_assert_ptr_eq(input + 17, endptr);
	xboard_feature_destroy(feature);
END_TEST

START_TEST(test_xboard_option_button)
	const char *input;
	Option *option;

	input = "Clear Hash -button";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Clear Hash", option->name);
	
	ck_assert_int_eq(option_type_button, option->type);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_save)
	const char *input;
	Option *option;

	input = "Apply -save";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Apply", option->name);
	
	ck_assert_int_eq(option_type_button, option->type);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_reset)
	const char *input;
	Option *option;

	input = "   Reset 	 -reset	 	 	";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Reset", option->name);
	
	ck_assert_int_eq(option_type_button, option->type);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_check)
	const char *input;
	Option *option;

	input = "Nullmove -check 0";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Nullmove", option->name);
	
	ck_assert_int_eq(option_type_check, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("0", option->default_value);

	option_destroy(option);

	input = "Nullmove -check 1";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Nullmove", option->name);
	
	ck_assert_int_eq(option_type_check, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("1", option->default_value);

	option_destroy(option);

	input = "Nullmove -check yes";

	option = option_xboard_new(input);
	ck_assert_ptr_eq(NULL, option);
END_TEST

START_TEST(test_xboard_option_string)
	const char *input;
	Option *option;

	input = "Foobar -string foo 	 bar  ";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Foobar", option->name);
	
	ck_assert_int_eq(option_type_string, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("foo 	 bar", option->default_value);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_spin)
	const char *input;
	Option *option;

	input = "Foobar -spin 0 -2304 2304 ";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Foobar", option->name);
	
	ck_assert_int_eq(option_type_spin, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("0", option->default_value);

	ck_assert_ptr_ne(NULL, option->min);
	ck_assert_str_eq("-2304", option->min);

	ck_assert_ptr_ne(NULL, option->min);
	ck_assert_str_eq("2304", option->max);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_combo)
	const char *input;
	Option *option;

	input = "Style -combo 	Solid /// *Normal /// Risky";

	option = option_xboard_new(input);
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

	input = "Style -combo 	Solid /// *Normal /// Risky   ///";

	option = option_xboard_new(input);
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

	input = "Style -combo 	Solid /// *Normal /// Risky   ///	  ";

	option = option_xboard_new(input);
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

START_TEST(test_xboard_option_slider)
	const char *input;
	Option *option;

	input = "Foobar -slider 0 -2304 2304 ";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Foobar", option->name);
	
	ck_assert_int_eq(option_type_spin, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("0", option->default_value);

	ck_assert_ptr_ne(NULL, option->min);
	ck_assert_str_eq("-2304", option->min);

	ck_assert_ptr_ne(NULL, option->min);
	ck_assert_str_eq("2304", option->max);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_file)
	const char *input;
	Option *option;

	input = "Foobar -file /path/to/file";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Foobar", option->name);
	
	ck_assert_int_eq(option_type_string, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("/path/to/file", option->default_value);

	option_destroy(option);
END_TEST

START_TEST(test_xboard_option_path)
	const char *input;
	Option *option;

	input = "Foobar -path /path/to/file";

	option = option_xboard_new(input);
	ck_assert_ptr_ne(NULL, option);

	ck_assert_ptr_ne(NULL, option->name);
	ck_assert_str_eq("Foobar", option->name);
	
	ck_assert_int_eq(option_type_string, option->type);

	ck_assert_ptr_ne(NULL, option->default_value);
	ck_assert_str_eq("/path/to/file", option->default_value);

	option_destroy(option);
END_TEST

Suite *
xboard_suite(void)
{
	Suite *suite;
	TCase *tc_feature;
	TCase *tc_option;

	suite = suite_create("Xboard Functions");

	tc_feature = tcase_create("Features");
	tcase_add_test(tc_feature, test_xboard_feature);
	suite_add_tcase(suite, tc_feature);

	tc_option = tcase_create("Options");
	tcase_add_test(tc_option, test_xboard_option_button);
	tcase_add_test(tc_option, test_xboard_option_save);
	tcase_add_test(tc_option, test_xboard_option_reset);
	tcase_add_test(tc_option, test_xboard_option_check);
	tcase_add_test(tc_option, test_xboard_option_string);
	tcase_add_test(tc_option, test_xboard_option_spin);
	tcase_add_test(tc_option, test_xboard_option_combo);
	tcase_add_test(tc_option, test_xboard_option_slider);
	tcase_add_test(tc_option, test_xboard_option_file);
	tcase_add_test(tc_option, test_xboard_option_path);
	suite_add_tcase(suite, tc_option);

	return suite;
}
