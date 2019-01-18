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

START_TEST(test_pawn_moves)
	chi_pos pos;
	chi_init_position(&pos);
	chi_move move1, move2;
	int errnum;
	char *wanted;
	char *got;
	
	errnum = chi_parse_move(&pos, &move1, "e4");
	ck_assert_int_eq(errnum, 0);

	chi_apply_move(&pos, move1);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	
	errnum = chi_parse_move(&pos, &move2, "e5");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move2);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppp1ppp/8/4P3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);

	errnum = chi_unapply_move(&pos, move2);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);

	errnum = chi_unapply_move(&pos, move1);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
END_TEST

Suite *
move_making_suite(void)
{
	Suite *suite;
	TCase *tc_pawn;
	
	suite = suite_create("Make/Unmake Moves");

	tc_pawn = tcase_create("Pawn Moves");
	tcase_add_test(tc_pawn, test_pawn_moves);
	suite_add_tcase(suite, tc_pawn);

	return suite;
}
