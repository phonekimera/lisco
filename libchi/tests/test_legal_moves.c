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

#include <string.h>

#include <check.h>

#include "libchi.h"

#define TEST_MOVE_GEN(fen, wanted) \
	chi_pos pos; \
	int errnum = chi_set_position(&pos, fen); \
	chi_move moves[CHI_MAX_MOVES]; \
	chi_move *moveptr; \
	size_t wanted_num_moves = sizeof wanted / sizeof wanted[0]; \
	char *got[wanted_num_moves]; \
	size_t got_num_moves; \
	unsigned int bufsize; \
	\
	ck_assert_int_eq(errnum, 0); \
	\
	if (!errnum) { \
		moveptr = chi_legal_moves(&pos, moves); \
		ck_assert_int_eq(moveptr - moves, wanted_num_moves); \
	} \
	\
	qsort(wanted, wanted_num_moves, sizeof wanted[0], compare_strings); \
	\
	moveptr = chi_legal_moves(&pos, moves); \
	\
	got_num_moves = moveptr - moves; \
	\
	ck_assert_uint_eq(got_num_moves, wanted_num_moves); \
	\
	if (got_num_moves != wanted_num_moves) \
		return; \
	\
	memset(got, 0, sizeof got); \
	for (size_t i = 0; i < got_num_moves; ++i) { \
		errnum = chi_coordinate_notation( \
			moves[i], chi_on_move(&pos), &got[i], &bufsize); \
		ck_assert_int_eq(errnum, 0); \
	} \
	\
	qsort(got, got_num_moves, sizeof got[0], compare_strings); \
	\
	for (size_t i = 0; i < got_num_moves; ++i) { \
		ck_assert_str_eq(got[i], wanted[i]); \
		free(got[i]); \
	} \

static int
compare_strings(const void *s1, const void *s2)
{
   return strcmp (* (char * const *) s1, * (char * const *) s2);
}

#include <stdio.h>
START_TEST(test_start_position)
{
	const char *fen =
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	const char *wanted[] = {
		"a2a3", "a2a4", "b2b3", "b2b4", "c2c3", "c2c4", "d2d3", "d2d4", 
		"e2e3", "e2e4", "f2f3", "f2f4", "g2g3", "g2g4", "h2h3", "h2h4",
		"b1a3", "b1c3", "g1f3", "g1h3" 
	};

	TEST_MOVE_GEN(fen, wanted)
}
END_TEST

#include <stdio.h>
#include "../magicmoves.h"
START_TEST(test_legal_moves_bug_1)
{
	const char *fen = 
			"1B3b1R/2q4b/2nn1Kp1/3p2p1/r7/k6r/p1p1p1p1/2RN1B1N b - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   | B |   |   |   | b |   | R | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   | q |   |   |   |   | b | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   | n | n |   | K | p |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   | p |   |   | p |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 | r |   |   |   |   |   |   |   | Material: -16.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 | k |   |   |   |   |   |   | r | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | p |   | p |   | p |   | p |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   | R | N |   | B |   | N |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	const char *wanted[] = {
		"a2a1q", "a2a1r", "a2a1b", "a2a1n", "c2d1q", "c2d1r",
		"c2d1b", "c2d1n", "e2e1q", "e2e1r", "e2e1b", "e2e1n",
		"e2f1q", "e2f1r", "e2f1b", "e2f1n", "e2d1q", "e2d1r",
		"e2d1b", "e2d1n", "g2g1q", "g2g1r", "g2g1b", "g2g1n",
		"g2h1q", "g2h1r", "g2h1b", "g2h1n", "g2f1q", "g2f1r",
		"g2f1b", "g2f1n", "a3b4", "a3b3", "h3h4", "h3h5",
		"h3h6", "h3h2", "h3h1", "h3g3", "h3f3", "h3e3",
		"h3d3", "h3c3", "h3b3", "a4b4", "a4c4", "a4d4",
		"a4e4", "a4f4", "a4g4", "a4h4", "a4a5", "a4a6",
		"a4a7", "a4a8", "d5d4", "g5g4", "c6b8", "c6d8",
		"c6a7", "c6e7", "c6a5", "c6e5", "c6b4", "c6d4",
		"d6c8", "d6e8", "d6b7", "d6f7", "d6b5", "d6f5",
		"d6c4", "d6e4", "c7d7", "c7e7", "c7f7", "c7g7",
		"c7c8", "c7b7", "c7a7", "c7b8", "c7d8", "c7b6",
		"c7a5", "h7g8", "f8g7", "f8h6", "f8e7"
	};

	TEST_MOVE_GEN(fen, wanted)
}
END_TEST

START_TEST(test_en_passant)
{
	const char *fen = 
			"1B1Q3N/pn3n2/p2N2RP/P1pKpPPb/5kP1/8/2P2P2/3B2R1 w - e6";
	const char *wanted[] = {
			"d1e2", "d1f3", "g1h1", "g1g2", "g1g3", "g1f1", "g1e1", "c2c3",
			"c2c4", "f2f3", "g4h5", "d5c6", "d5e6", "d5c4", "f5f6", "f5e6",
			"d6c8", "d6e8", "d6b7", "d6f7", "d6b5", "d6c4", "d6e4", "g6g7",
			"g6g8", "g6f6", "g6e6", "h6h7", "b8c7", "b8a7", "d8e8", "d8f8",
			"d8g8", "d8d7", "d8c8", "d8e7", "d8f6", "d8c7", "d8b6", "h8f7"
	};

	TEST_MOVE_GEN(fen, wanted)
}

Suite *
legal_moves_suite(void)
{
	Suite *suite;
	TCase *tc_legal_moves;

	suite = suite_create("Move Generation");

	tc_legal_moves = tcase_create("Legal Moves");
	tcase_add_test(tc_legal_moves, test_start_position);
	tcase_add_test(tc_legal_moves, test_legal_moves_bug_1);
	tcase_add_test(tc_legal_moves, test_en_passant);
	suite_add_tcase(suite, tc_legal_moves);

	return suite;
}
