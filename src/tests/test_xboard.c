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

#include "xmalloca.h"

#include "util.h"

START_TEST(test_xboard_feature)
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
