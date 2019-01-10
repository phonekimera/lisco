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

#if CHI_USE_UTF_8

START_TEST(test_char2figurine)
END_TEST

#endif /* CHI_USE_UTF_8 */

Suite *
presentation_suite(void)
{
	Suite *suite;
	TCase *tc_figurine;
	
	suite = suite_create("Presentation");

#if CHI_USE_UTF_8
	tc_figurine = tcase_create("Figurine");

	tcase_add_test(tc_figurine, test_char2figurine);

	suite_add_tcase(suite, tc_figurine);
#endif

	return suite;
}
