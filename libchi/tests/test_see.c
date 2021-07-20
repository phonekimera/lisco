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

#include "libchi.h"

#include <check.h>

static unsigned piece_values[6] = {
	100, 300, 325, 500, 900, 10000
};

/* Example from
 * http://mediocrechess.blogspot.com/2007/03/guide-static-exchange-evaluation-see.html
 */
START_TEST(test_obvious_attackers_lone_pawn)
{
	const char *fen_white = "7k/p7/1p6/8/8/1Q6/8/7K w - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   | k | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 | p |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   | p |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +7.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   | Q |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	chi_pos position;
	int errnum;
	unsigned white_attackers[16], black_attackers[16];
	chi_move move;

	errnum = chi_set_position(&position, fen_white);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Qxb6");
	ck_assert_int_eq(errnum, 0);

	chi_obvious_attackers(&position, move, white_attackers, black_attackers);

	ck_assert_int_eq(white_attackers[0], 0);
	ck_assert_int_eq(black_attackers[0], CHI_A7 | pawn << 8);
	ck_assert_int_eq(black_attackers[1], 0);

	const char *fen_black = "7K/8/1q6/8/8/1P6/P7/7k b - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   | K | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   | q |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: -7.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   | P |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | k |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	errnum = chi_set_position(&position, fen_black);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Qxb3");
	ck_assert_int_eq(errnum, 0);

	chi_obvious_attackers(&position, move, white_attackers, black_attackers);

	ck_assert_int_eq(white_attackers[0], CHI_A2 | pawn << 8);
	ck_assert_int_eq(white_attackers[1], 0);
	ck_assert_int_eq(black_attackers[0], 0);
}
END_TEST

#include <stdio.h>
START_TEST(test_obvious_attackers_all_pieces)
{
	/* This position is actually not a legal position because the last move
	 * must have been e7-e5 (en passant field is e6!) but e7 is occupied by
	 * the black king.  Who cares?
	 */
	const char *fen_white = "8/3pkB2/7Q/2KPpPn1/8/4r3/8/8 w - e6 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   |   | En passant possible on file e.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   | p | k | B |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   | Q | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   | K | P | p | P | n |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +4.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   | r |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	chi_pos position;
	int errnum;
	unsigned white_attackers[16], black_attackers[16];
	chi_move move;

	errnum = chi_set_position(&position, fen_white);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "dxe");
	ck_assert_int_eq(errnum, 0);

	chi_obvious_attackers(&position, move, white_attackers, black_attackers);

	ck_assert_int_eq(white_attackers[0], CHI_F5 | pawn << 8);
	/* Note that a bishop is counted as a knight to avoid branching.  */
	ck_assert_int_eq(white_attackers[1], CHI_F7 | knight << 8);
	ck_assert_int_eq(white_attackers[2], CHI_H6 | queen << 8);
	ck_assert_int_eq(white_attackers[3], 0);

	ck_assert_int_eq(black_attackers[0], CHI_D7 | pawn << 8);
	ck_assert_int_eq(black_attackers[1], CHI_G5 | knight << 8);
	ck_assert_int_eq(black_attackers[2], CHI_E3 | rook << 8);
	ck_assert_int_eq(black_attackers[3], CHI_E7 | king << 8);
	ck_assert_int_eq(white_attackers[4], 0);

	const char *fen_black = "8/8/4R3/8/2kpPpN1/7q/3PKb2/8 b - e3 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   |   | En passant possible on file e.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   | R |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   | k | p | P | p | N |   | Material: -4.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   | q | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   | P | K | b |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	errnum = chi_set_position(&position, fen_black);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "dxe");
	ck_assert_int_eq(errnum, 0);

	chi_obvious_attackers(&position, move, white_attackers, black_attackers);

	ck_assert_int_eq(white_attackers[0], CHI_D2 | pawn << 8);
	ck_assert_int_eq(white_attackers[1], CHI_G4 | knight << 8);
	ck_assert_int_eq(white_attackers[2], CHI_E6 | rook << 8);
	ck_assert_int_eq(white_attackers[3], CHI_E2 | king << 8);
	ck_assert_int_eq(white_attackers[4], 0);

	ck_assert_int_eq(black_attackers[0], CHI_F4 | pawn << 8);
	/* Note that a bishop is counted as a knight to avoid branching.  */
	ck_assert_int_eq(black_attackers[1], CHI_F2 | knight << 8);
	ck_assert_int_eq(black_attackers[2], CHI_H3 | queen << 8);
	ck_assert_int_eq(black_attackers[3], 0);
}
END_TEST

START_TEST(test_see_rook_wins_pawn)
{
	const char *fen_white = "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   | k |   | r |   |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   | p | p |   |   |   |   | p | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 | p |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   | p |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 | P |   |   |   |   |   | P |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   | P | P |   |   |   |   | P |
   +---+---+---+---+---+---+---+---+
 1 |   |   | K |   | R |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	chi_pos position;
	int errnum;
	chi_move move;
	int score;

	errnum = chi_set_position(&position, fen_white);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Rxe5");
	ck_assert_int_eq(errnum, 0);

	score = chi_see(&position, move, piece_values);
	ck_assert_int_eq(score, 100);
}

/* Example from
 * http://mediocrechess.blogspot.com/2007/03/guide-static-exchange-evaluation-see.html
 */
START_TEST(test_see_queen_hits_defended_pawn)
{
	const char *fen_white = "7k/p7/1p6/8/8/1Q6/8/7K w - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   |   |   |   | k | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 | p |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   | p |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +7.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   | Q |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   |   |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	chi_pos position;
	int errnum;
	chi_move move;
	int score;

	errnum = chi_set_position(&position, fen_white);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Qxb6");
	ck_assert_int_eq(errnum, 0);

	score = chi_see(&position, move, piece_values);
	ck_assert_int_eq(score, -800);
}

Suite *
see_suite(void)
{
	Suite *suite;
	TCase *tc_obvious_attackers;
	TCase *tc_see;

	suite = suite_create("Static Exchange Evaluation");

	tc_obvious_attackers = tcase_create("Obvious Attackers");
	tcase_add_test(tc_obvious_attackers, test_obvious_attackers_lone_pawn);
	tcase_add_test(tc_obvious_attackers, test_obvious_attackers_all_pieces);
	suite_add_tcase(suite, tc_obvious_attackers);

	tc_see = tcase_create("Static Exchange Evaluation");
	tcase_add_test(tc_see, test_see_rook_wins_pawn);
	tcase_add_test(tc_see, test_see_queen_hits_defended_pawn);
	suite_add_tcase(suite, tc_see);

	return suite;
}
