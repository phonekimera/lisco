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

	ck_assert_int_eq(time_control_init_st(&tc, "2304"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_true);
	ck_assert_int_eq(tc.seconds_per_move, 2304);

	ck_assert_int_eq(time_control_init_st(&tc, "-10"), chi_false);

	ck_assert_int_eq(time_control_init_st(&tc, "0"), chi_false);
END_TEST

START_TEST(test_st_calculate_flag)
	TimeControl tc;
	struct timeval now;

	ck_assert_int_eq(time_control_init_st(&tc, "10"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_true);

	now.tv_sec = 3;
	now.tv_usec = 500000;

	time_control_start_thinking(&tc, &now);
	ck_assert_int_eq(tc.flag.tv_sec, 13);
	ck_assert_int_eq(tc.flag.tv_usec, 500000);

	now.tv_sec = 13;
	now.tv_usec = 499999;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	now.tv_sec = 13;
	now.tv_usec = 500000;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_false);
END_TEST

START_TEST(test_level_sudden_death)
	TimeControl tc;
	struct timeval now;

	ck_assert_int_eq(time_control_init_level(&tc, "0 0:30 0"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);

	now.tv_sec = 0;
	now.tv_usec = 0;
	time_control_start_thinking(&tc, &now);

	now.tv_sec = 29;
	now.tv_usec = 999999;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	time_control_start_thinking(&tc, &now);

	now.tv_sec = 30;
	now.tv_usec = 0;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_false);
END_TEST

START_TEST(test_level_sudden_death_with_increment)
	TimeControl tc;
	struct timeval now;

	ck_assert_int_eq(time_control_init_level(&tc, "0 0:30 1"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);

	now.tv_sec = 0;
	now.tv_usec = 0;
	time_control_start_thinking(&tc, &now);

	now.tv_sec = 29;
	now.tv_usec = 999999;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	time_control_start_thinking(&tc, &now);

	now.tv_sec = 30;
	now.tv_usec = 0;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	time_control_start_thinking(&tc, &now);

	now.tv_sec = 32;
	now.tv_usec = 0;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_false);
END_TEST

START_TEST(test_level_conventional)
	TimeControl tc;
	struct timeval now;

	ck_assert_int_eq(time_control_init_level(&tc, "2 0:30 0"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);

	now.tv_sec = 0;
	now.tv_usec = 0;
	time_control_start_thinking(&tc, &now);

	now.tv_sec = 15;
	now.tv_usec = 0;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	time_control_start_thinking(&tc, &now);

	now.tv_sec = 29;
	now.tv_usec = 999999;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	time_control_start_thinking(&tc, &now);

	now.tv_sec = 45;
	now.tv_usec = 0;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_true);

	time_control_start_thinking(&tc, &now);

	now.tv_sec = 60;
	now.tv_usec = 0;
	ck_assert_int_eq(time_control_stop_thinking(&tc, &now), chi_false);
END_TEST

START_TEST(test_level_constructor)
	TimeControl tc;

	ck_assert_int_eq(time_control_init_level(&tc, "40 5 0"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);
	ck_assert_int_eq(tc.moves_per_time_control, 40);
	ck_assert_int_eq(tc.seconds_per_time_control, 300);
	ck_assert_int_eq(tc.increment, 0);

	ck_assert_int_eq(time_control_init_level(&tc, "40 00:30 0"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);
	ck_assert_int_eq(tc.moves_per_time_control, 40);
	ck_assert_int_eq(tc.seconds_per_time_control, 30);
	ck_assert_int_eq(tc.increment, 0);

	ck_assert_int_eq(time_control_init_level(&tc, "0 2 12"), chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);
	ck_assert_int_eq(tc.moves_per_time_control, 0);
	ck_assert_int_eq(tc.seconds_per_time_control, 120);
	ck_assert_int_eq(tc.increment, 12);

	ck_assert_int_eq(time_control_init_level(&tc, "   40-00:30|0 "),
	                                         chi_true);
	ck_assert_int_eq(tc.fixed_time, chi_false);
	ck_assert_int_eq(tc.moves_per_time_control, 40);
	ck_assert_int_eq(tc.seconds_per_time_control, 30);
	ck_assert_int_eq(tc.increment, 0);
END_TEST

Suite *
time_control_suite(void)
{
	Suite *suite;
	TCase *tc_search_time;
	TCase *tc_level;

	suite = suite_create("Time control");

	tc_search_time = tcase_create("Fixed search time");
	tcase_add_test(tc_search_time, test_st_constructor);
	tcase_add_test(tc_search_time, test_st_calculate_flag);
	suite_add_tcase(suite, tc_search_time);

	tc_level = tcase_create("Level time control");
	tcase_add_test(tc_level, test_level_constructor);
	tcase_add_test(tc_level, test_level_sudden_death);
	tcase_add_test(tc_level, test_level_sudden_death_with_increment);
	tcase_add_test(tc_level, test_level_conventional);
	suite_add_tcase(suite, tc_level);

	return suite;
}
