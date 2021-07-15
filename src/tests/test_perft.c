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

#include "libchi.h"
#include "../src/perft.h"
#include "../src/rtime.h"

typedef struct PerftTest {
	const char *filename;
	unsigned int lineno;
	const char *name;
	const char *fen;
	long nodes[10];
} PerftTest;

const PerftTest tests[] = {
	{
		__FILE__, __LINE__,
		"Start position",
		NULL,
		{ 20UL, 400UL, 8902UL, 197281UL, 4865609UL, 119060324UL, 3195901860UL,
		  -1UL }
	},
	{
		__FILE__, __LINE__,
		"Kiwipete",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
		{ 48UL, 2039UL, 97862UL, 4085603UL, 193690690UL, -1 }
	},
	{
		__FILE__, __LINE__,
		"Discovered Check",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
		{ 14UL, 191UL, 2812UL, 43238UL, 674624UL, 11030083UL, 178633661UL,
		  3009794393UL, -1UL }
	},
	{
		__FILE__, __LINE__,
		"Chessprogramming.org Position 4",
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
		{ 6UL, 264UL, 9467UL, 422333UL, 15833292UL, 706045033UL, -1UL }
	},
	{
		__FILE__, __LINE__,
		"Chessprogramming.org Position 4 Reversed",
		"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
		{ 6UL, 264UL, 9467UL, 422333UL, 15833292UL, 706045033UL, -1UL }
	},
	{
		__FILE__, __LINE__,
		"Chessprogramming.org Position 5",
		"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
		{ 44UL, 1486UL, 62379UL, 2103487UL, 89941194UL, -1UL }
	},
	{
		__FILE__, __LINE__,
		"Steven Edwards Alternative (chessprogramming.org #6)",
		"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
		{ 46UL, 2079UL, 89890UL, 3894594UL, 164075551UL, -1UL }
	},
	{
		__FILE__, __LINE__,
		"Most legal moves (Nenad Petrovic 1964)",
		"R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 1 1",
		{ 218UL, 99UL, 19073UL, 85043UL, 13853661UL, 115892741UL, -1UL }
	}
};

void
report_failure(unsigned long got_nodes, int depth, const PerftTest *test)
{
	fprintf(stderr, "Failed test at %s:%u\n", test->filename, test->lineno);
	ck_assert_uint_eq(got_nodes, test->nodes[depth - 1]);
}

START_TEST(test_perft)
{
	unsigned long num_tests = sizeof tests / sizeof tests[0];
	long maxnodes = 50000000L;
	unsigned long long counts[CHI_MAX_MOVES];
	
	if (getenv("TATE_STRESS_TEST") != NULL) {
		fprintf(stderr, "Unset the environment variable TATE_STRESS_TEST"
		        " if you want to limit tests to %ld nodes.\n", maxnodes);
		maxnodes = -1L;
	} else {
		fprintf(stderr, "Limiting tests to a maximum of %ld nodes. Set"
		        " the environment variable TATE_STRESS_TEST for the full test"
		        " suite.\n", maxnodes);
	}
 
	for (unsigned long i = 0; i < num_tests; ++i) {
		const PerftTest *test = &tests[i];
		fprintf(stderr, "Executing perft test %s at %s:%u.\n",
		        test->name, test->filename, test->lineno);
		for (int j = 0; test->nodes[j] >= 0; ++j) {
			int depth = j + 1;
			chi_pos pos;
			int errnum;

			if (maxnodes > 0 && test->nodes[j] > maxnodes)
				break;

			fprintf(stderr, "	depth %d ...", depth);
			fflush(stderr);
			if (test->fen) {
				errnum = chi_set_position(&pos, test->fen);
				ck_assert_int_eq(errnum, 0);
			} else
				chi_init_position(&pos);

			rtime_t start = rtime();
			unsigned long nodes = perft(&pos, depth,
					(unsigned long long *) &counts, NULL);
			unsigned long int elapsed = rdifftime (rtime (), start);
			fprintf (stderr, " (nodes: %lu, %lu.%02lu s, nps: %lu)\n",
				nodes, elapsed / 100, elapsed % 100,
				(100 * nodes) / (elapsed ? elapsed : 1));
const char *fen = chi_fen(&pos);
fprintf(stderr, "        fen: %s\n", fen);
free((void *) fen);

			if (nodes != test->nodes[depth - 1])
				report_failure(nodes, depth, test);
		}
	}
}
END_TEST

Suite *
perft_suite(void)
{
	Suite *suite;
	TCase *tc_perft;

	chi_mm_init();

	suite = suite_create("Perft");

	tc_perft = tcase_create("Perft");
	tcase_set_timeout(tc_perft, 31557600); // One year. ;)
	tcase_add_test(tc_perft, test_perft);
	suite_add_tcase(suite, tc_perft);

	return suite;
}
