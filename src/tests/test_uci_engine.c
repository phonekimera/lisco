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

#define TEST_UCI_ENGINE 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <check.h>

#include "../state.h"
#include "../uci-engine.h"

#define INIT_UCI(engine_options, output) \
	UCIEngineOptions engine_options; \
	FILE *engine_out = fmemopen((void *) output, sizeof output, "w"); \
	\
	memset((void *) output, 0, sizeof output); \
	uci_init(&engine_options);

#define TEST_UCI_STR(x) #x
#define TEST_UCI_TOSTR(str) TEST_UCI_STR(str)

#if HAVE_FMEMOPEN
START_TEST(test_uci_main)
{
	const char *input = "foobar\n    trim\t   \nquit\n";
	const char output[1024];
	char *expect;
	int status;
	FILE *engine_in = fmemopen((void *) input, strlen(input), "r");

	INIT_UCI(engine_options, output);

	status = uci_main(&engine_options, engine_in, "[in memory buffer]",
	                  engine_out, "[in memory buffer]");
	ck_assert_int_eq(status, 0);
	expect = "Unknown command: foobar\nUnknown command: trim\n";

	ck_assert_str_eq(output, expect);
}
END_TEST

START_TEST(test_uci_quit)
{
	int status;
	UCIEngineOptions engine_options;

	uci_init(&engine_options);

	status = uci_handle_quit(&engine_options);
	ck_assert_int_eq(status, 0);
}
END_TEST

START_TEST(test_uci_uci)
{
	const char *input = "uci\n";
	const char output[1024];
	char *expect;
	int status;
	size_t output_length, expect_length;

	INIT_UCI(engine_options, output);

	status = uci_handle_uci(&engine_options, input, engine_out);
	ck_assert_int_eq(status, 1);

	output_length = strlen(output);

	expect = "id name " PACKAGE " " PACKAGE_VERSION "\n";
	ck_assert_int_eq(strncmp(output, expect, strlen(expect)), 0);

	expect = "\noption name Threads type spin default 1 min 1 max " TEST_UCI_TOSTR(UCI_ENGINE_MAX_THREADS) "\n";
	ck_assert_ptr_nonnull(strstr(output, expect));

	expect = "uciok\n";
	expect_length = strlen(expect);
	ck_assert_int_eq(strncmp(output + output_length - expect_length, expect, expect_length), 0);
}
END_TEST

START_TEST(test_uci_debug)
{
	const char output[1024];
	int status;

	INIT_UCI(engine_options, output);

	status = uci_handle_debug(&engine_options, "on", engine_out);
	ck_assert_int_eq(status, 1);
	ck_assert_int_ne(engine_options.debug, 0);

	status = uci_handle_debug(&engine_options, "off", engine_out);
	ck_assert_int_eq(status, 1);
	ck_assert_int_eq(engine_options.debug, 0);

	ck_assert_str_eq(output, "");
}
END_TEST

START_TEST(test_uci_position)
{
	const char output[1024];
	int status;
	const char *current_fen;

	INIT_UCI(engine_options, output);

	chi_init_position (&tate.position);

	status = uci_handle_position(&engine_options, "startpos", engine_out);
	ck_assert_int_eq(status, 1);
	current_fen = chi_fen(&tate.position);
	ck_assert_str_eq(current_fen,
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	free((void *) current_fen);

	ck_assert_str_eq(output, "");
}
END_TEST

Suite *
uci_engine_suite(void)
{
	Suite *suite;
	TCase *tc_uci_parser;

	suite = suite_create("UCI Engine Functions");

	tc_uci_parser = tcase_create("UCI Parser");
	tcase_add_test(tc_uci_parser, test_uci_main);
	tcase_add_test(tc_uci_parser, test_uci_quit);
	tcase_add_test(tc_uci_parser, test_uci_uci);
	tcase_add_test(tc_uci_parser, test_uci_debug);
	tcase_add_test(tc_uci_parser, test_uci_position);
	suite_add_tcase(suite, tc_uci_parser);

	return suite;
}

#else
Suite *
uci_engine_suite(void)
{
	Suite *suite;

	suite = suite_create("UCI Engine Functions");
	fprintf(stderr, "UCI engine functions not tested becase fmemopen() is not available.\n");

	return suite;
}
#endif
