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

#include "libchi.h"

START_TEST(test_pawn_move)
	chi_move move = chi_coords2shift(4, 1)
	                | chi_coords2shift(4, 3) << 6
			| pawn;
	char *buf = NULL;
	unsigned int bufsize;
	int errnum;

	errnum = chi_coordinate_notation(move, &buf, &bufsize);
	ck_assert_int_eq(errnum, 0);
END_TEST

Suite *
coordinate_notation_suite(void)
{
	Suite *suite;
	TCase *tc_simple;

	suite = suite_create("Moves in coordinate notation");

	tc_simple = tcase_create("Simple");
	tcase_add_test(tc_simple, test_pawn_move);
	suite_add_tcase(suite, tc_simple);

	return suite;
}
