/* This file is part of the chess engine tate.
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
#include <stdlib.h>

#include "libchi.h"

extern Suite *parsers_suite();
extern Suite *presentation_suite();
extern Suite *game_over_suite();
extern Suite *move_making_suite();
extern Suite *move_making_suite_pgn();
extern Suite *fen_suite();
extern Suite *coordinate_notation_suite();

int
main(int argc, char *argv[])
{
	int failed = 0;
	SRunner *runner;

	runner = srunner_create(parsers_suite());
	srunner_add_suite(runner, presentation_suite());
	srunner_add_suite(runner, game_over_suite());
	srunner_add_suite(runner, move_making_suite());
	srunner_add_suite(runner, move_making_suite_pgn());
	srunner_add_suite(runner, fen_suite());
	srunner_add_suite(runner, coordinate_notation_suite());

	srunner_run_all(runner, CK_NORMAL);
	failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}