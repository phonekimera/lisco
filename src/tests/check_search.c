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

#include <error.h>

#include <check.h>

#include "libchi.h"

#include "../lisco.h"

extern Suite *quiescence_suite();

#ifdef DEBUG_XMALLOC
# include "../xmalloc-debug.c"
#endif

int
main(int argc, char *argv[])
{
	int failed = 0;
	SRunner *runner;

#ifdef DEBUG_XMALLOC
	init_xmalloc_debug();
#endif

	lisco_initialize(argv[0]);

	runner = srunner_create(quiescence_suite());

	srunner_run_all(runner, CK_NORMAL);
	failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
