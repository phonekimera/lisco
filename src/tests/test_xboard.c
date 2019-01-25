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

Suite *
xboard_suite(void)
{
	Suite *suite;
	TCase *tc_feature;

	suite = suite_create("Xboard Functions");

	tc_feature = tcase_create("Features");
	tcase_add_test(tc_feature, test_xboard_feature);
	suite_add_tcase(suite, tc_feature);

	return suite;
}
