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

START_TEST(test_movetime)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.movetime = 120000;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.fixed_time, 120000);
	ck_assert(tree.nodes_to_tc > 0);
}
END_TEST

START_TEST(test_nodes)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.nodes = 2304;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.fixed_time, 0);
	ck_assert_uint_eq(tree.nodes_to_tc, 2304);
}
END_TEST

START_TEST(test_depth)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.depth = 3;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.max_depth, 3);
}
END_TEST

START_TEST(test_mate)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.mate = 2;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.max_depth, 3);
}
END_TEST

START_TEST(test_sudden_death)
{
	SearchParams params;
	Tree tree;

	memset(&params, 0, sizeof params);
	memset(&tree, 0, sizeof tree);

	params.mytime = 40000;
	params.hertime = 40000;

	process_search_params(&tree, &params);

	ck_assert_uint_eq(tree.fixed_time, 3);
}
END_TEST

Suite *
time_control_suite(void)
{
	Suite *suite;
	TCase *tc_process_search_params;
	TCase *tc_time_allocation;

	suite = suite_create("Time Control");

	tc_process_search_params = tcase_create("Process Parameters");
	tcase_add_test(tc_process_search_params, test_movetime);
	tcase_add_test(tc_process_search_params, test_nodes);
	tcase_add_test(tc_process_search_params, test_depth);
	tcase_add_test(tc_process_search_params, test_mate);
	suite_add_tcase(suite, tc_process_search_params);

	tc_time_allocation = tcase_create("Time Allocation");
	tcase_add_test(tc_time_allocation, test_sudden_death);
	suite_add_tcase(suite, tc_time_allocation);

	return suite;
}
