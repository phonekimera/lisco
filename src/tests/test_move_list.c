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

#include <check.h>

#include "lisco.h"

START_TEST(test_basic)
{
	MoveList list;

	move_list_init(&list);
	ck_assert_uint_eq(list.num_moves, 0);
	ck_assert(list.moves == NULL);

	move_list_add(&list, 123);
	ck_assert_uint_eq(list.num_moves, 1);
	ck_assert(list.moves != NULL);

	move_list_add(&list, 234);
	ck_assert_uint_eq(list.num_moves, 2);
	ck_assert(list.moves != NULL);

	move_list_add(&list, 567);
	ck_assert_uint_eq(list.num_moves, 3);
	ck_assert(list.moves != NULL);

	move_list_destroy(&list);
}	
END_TEST

START_TEST(test_contains)
{
	MoveList list;

	move_list_init(&list);
	ck_assert_uint_eq(list.num_moves, 0);
	ck_assert(list.moves == NULL);

	move_list_add(&list, 1303);
	ck_assert_uint_eq(list.num_moves, 1);
	ck_assert(list.moves != NULL);
	ck_assert(move_list_contains(&list, 1303));
	ck_assert(!move_list_contains(&list, 2304));

	move_list_add(&list, 2304);
	ck_assert_uint_eq(list.num_moves, 2);
	ck_assert(list.moves != NULL);
	ck_assert(move_list_contains(&list, 2304));

	ck_assert(!move_list_contains(&list, 9999));

	/* Check that irrelevant bits are ignored.  */
	ck_assert(move_list_contains(&list, 1303 | 1ULL << CHI_MOVE_RELEVANT_BITS));
	ck_assert(move_list_contains(&list, 2304 | 0xffffffffULL << 32));

	move_list_destroy(&list);
}	
END_TEST

Suite *
move_list_suite(void)
{
	Suite *suite;
	TCase *tc_basic;

	suite = suite_create("Move List");

	tc_basic = tcase_create("Basic");
	tcase_add_test(tc_basic, test_basic);
	tcase_add_test(tc_basic, test_contains);
	suite_add_tcase(suite, tc_basic);

	return suite;
}
