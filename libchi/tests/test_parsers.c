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
	ck_assert_str_eq("chi", "chi");
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
