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

#include "tateplay-game.h"

START_TEST(test_stalemate)
	Game *game = game_new();

	ck_assert_ptr_ne(game, NULL);

	game_destroy(game);
END_TEST

Suite *
game_suite(void)
{
	Suite *suite;
	TCase *tc_draw;

	suite = suite_create("Game");

	tc_draw = tcase_create("Draw detection");
	tcase_add_test(tc_draw, test_stalemate);
	suite_add_tcase(suite, tc_draw);

	return suite;
}
