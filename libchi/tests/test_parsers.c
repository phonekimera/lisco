/*
 * Copyright (C) 2002 Guido Flohr (guido@imperia.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <check.h>

#include "libchi.h"

START_TEST(test_parse_move_san_bug)
	chi_pos pos;
	chi_move move;
	const char *fen = "r5k1/1p2b1pp/p2p4/2pqpr2/8/1P1PQN2/P4PPP/1RR4K b - - 1 19";
	int errnum = chi_set_position(&pos, fen);

	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move (&pos, &move, "Ra8f8");
	ck_assert_int_eq(errnum, 0);
	ck_assert_int_eq(chi_move_from(move), chi_coords2shift(0, 7));
	ck_assert_int_eq(chi_move_to(move), chi_coords2shift(5, 7));
//	errnum = chi_parse_move (&pos, &move, "Raf8");
//	ck_assert_int_eq(errnum, 0);
END_TEST

Suite *
parsers_suite(void)
{
	Suite *suite;
	TCase *tc_bugs;
	
	suite = suite_create("libchi");

	tc_bugs = tcase_create("Bugs");

	tcase_add_test(tc_bugs, test_parse_move_san_bug);

	suite_add_tcase(suite, tc_bugs);

	return suite;
}
