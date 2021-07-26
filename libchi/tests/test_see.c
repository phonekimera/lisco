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

#include <stdio.h>

#include "libchi.h"

#include <check.h>

typedef struct SEETest {
	const char *filename;
	int lineno;
	const char *fen;
	const char *san;
	int score;
} SEETest;

static SEETest tests[] = {
	/* Test positions from Arasan (https://github.com/jdart1/arasan-chess).  */
	{
		__FILE__,
		__LINE__,
		"4R3/2r3p1/5bk1/1p1r3p/p2PR1P1/P1BK1P2/1P6/8 b - - 0 1",
		"hxg4",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"4R3/2r3p1/5bk1/1p1r1p1p/p2PR1P1/P1BK1P2/1P6/8 b - -",
		"hxg4",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"4r1k1/5pp1/nbp4p/1p2p2q/1P2P1b1/1BP2N1P/1B2QPPK/3R4 b - -",
		"Bxf3",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"2r1r1k1/pp1bppbp/3p1np1/q3P3/2P2P2/1P2B3/P1N1B1PP/2RQ1RK1 b - -",
		"dxe5",
		CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"7r/5qpk/p1Qp1b1p/3r3n/BB3p2/5p2/P1P2P2/4RK1R w - -",
		"Re8",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"6rr/6pk/p1Qp1b1p/2n5/1B3p2/5p2/P1P2P2/4RK1R w - -",
		"Re8",
		-CHI_SEE_ROOK_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"7r/5qpk/2Qp1b1p/1N1r3n/BB3p2/5p2/P1P2P2/4RK1R w - -",
		"Re8",
		-CHI_SEE_ROOK_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"6RR/4bP2/8/8/5r2/3K4/5p2/4k3 w - -",
		"f8=Q",
		CHI_SEE_BISHOP_VALUE - CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"6RR/4bP2/8/8/5r2/3K4/5p2/4k3 w - -",
		"f8=N",
		CHI_SEE_KNIGHT_VALUE - CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		/* Moved the rook so that the white king is not in chess.  */
		"7R/5P2/8/8/6r1/3K4/5p2/4k3 w - - 0 1",
		"f8=Q",
		CHI_SEE_QUEEN_VALUE - CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		/* Moved the rook so that the white king is not in chess.  */
		"7R/5P2/8/8/6r1/3K4/5p2/4k3 w - - 0 1",
		"f8=B",
		CHI_SEE_BISHOP_VALUE - CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"7R/4bP2/8/8/1q6/3K4/5p2/4k3 w - -",
		"f8=R",
		-CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"8/4kp2/2npp3/1Nn5/1p2PQP1/7q/1PP1B3/4KR1r b - -",
		"Rxf1+",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"8/4kp2/2npp3/1Nn5/1p2P1P1/7q/1PP1B3/4KR1r b - -",
		"Rxf1+",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"2r2r1k/6bp/p7/2q2p1Q/3PpP2/1B6/P5PP/2RR3K b - -",
		"Qxc1",
		2 * CHI_SEE_ROOK_VALUE - CHI_SEE_QUEEN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"r2qk1nr/pp2ppbp/2b3p1/2p1p3/8/2N2N2/PPPP1PPP/R1BQR1K1 w kq -",
		"Nxe5",
		CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"6r1/4kq2/b2p1p2/p1pPb3/p1P2B1Q/2P4P/2B1R1P1/6K1 w - -",
		"Bxe5",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"3q2nk/pb1r1p2/np6/3P2Pp/2p1P3/2R4B/PQ3P1P/3R2K1 w - h6",
		"gxh6",
		CHI_SEE_NO_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"3q2nk/pb1r1p2/np6/3P2Pp/2p1P3/2R1B2B/PQ3P1P/3R2K1 w - h6",
		"gxh6",
		CHI_SEE_PAWN_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"2r4r/1P4pk/p2p1b1p/7n/BB3p2/2R2p2/P1P2P2/4RK2 w - -",
		"Rxc8",
		CHI_SEE_ROOK_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"2r5/1P4pk/p2p1b1p/5b1n/BB3p2/2R2p2/P1P2P2/4RK2 w - -",
		"Rxc8",
		CHI_SEE_BISHOP_VALUE  // Was originally CHI_SEE_ROOK_VALUE.
	},
	{
		__FILE__,
		__LINE__,
		"2r4k/2r4p/p7/2b2p1b/4pP2/1BR5/P1R3PP/2Q4K w - -",
		"Rxc5",
		CHI_SEE_BISHOP_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"8/pp6/2pkp3/4bp2/2R3b1/2P5/PP4B1/1K6 w - -",
		"Bxc6",
		CHI_SEE_PAWN_VALUE - CHI_SEE_BISHOP_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"4q3/1p1pr1k1/1B2rp2/6p1/p3PP2/P3R1P1/1P2R1K1/4Q3 b - -",
		"Rxe4",
		CHI_SEE_PAWN_VALUE - CHI_SEE_ROOK_VALUE
	},
	{
		__FILE__,
		__LINE__,
		"4q3/1p1pr1kb/1B2rp2/6p1/p3PP2/P3R1P1/1P2R1K1/4Q3 b - -","Bxe4",
		CHI_SEE_PAWN_VALUE
	},
};

static void
report_failure(const SEETest *test, int errnum, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	fprintf(stderr, "Failed test at %s:%d\n", test->filename, test->lineno + 1);
	fprintf(stderr, "pos: %s\n", test->fen);
	fprintf(stderr, "move: %s\n", test->san);

	vfprintf(stderr, fmt, ap);

	va_end(ap);

	if (errnum) {
		fprintf(stderr, "Error code %d: %s\n", errnum, chi_strerror(errnum));
	}
	ck_abort();
}

START_TEST(test_see_positions)
{
	size_t num_tests = sizeof tests / sizeof tests[0];

	for (size_t i = 0; i < num_tests; ++i) {
		SEETest *test = &tests[i];

		chi_pos position;
		int errnum = chi_set_position(&position, test->fen);
		if (errnum != 0) {
			report_failure(test, errnum, "Invalid position.\n");
		}

		chi_move move;
		errnum = chi_parse_move(&position, &move, test->san);
		if (errnum != 0) {
			report_failure(test, errnum, "Invalid move.\n");
		}

		int score = chi_see(&position, move);
		if (score != test->score) {
			report_failure(test, 0, "Expected score %d, got score %d.\n",
					test->score, score);
		}
	}
}
END_TEST

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
	ck_assert_int_eq(black_attackers[0], CHI_A7 | CHI_SEE_PAWN_VALUE << 8);
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

	ck_assert_int_eq(white_attackers[0], CHI_A2 | CHI_SEE_PAWN_VALUE << 8);
	ck_assert_int_eq(white_attackers[1], 0);
	ck_assert_int_eq(black_attackers[0], 0);
}
END_TEST

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

	ck_assert_int_eq(white_attackers[0], CHI_F5 | CHI_SEE_PAWN_VALUE << 8);
	/* Note that a bishop is counted as a knight to avoid branching.  */
	ck_assert_int_eq(white_attackers[1], CHI_F7 | CHI_SEE_KNIGHT_VALUE << 8);
	ck_assert_int_eq(white_attackers[2], CHI_H6 | CHI_SEE_QUEEN_VALUE << 8);
	ck_assert_int_eq(white_attackers[3], 0);

	ck_assert_int_eq(black_attackers[0], CHI_D7 | CHI_SEE_PAWN_VALUE << 8);
	ck_assert_int_eq(black_attackers[1], CHI_G5 | CHI_SEE_KNIGHT_VALUE << 8);
	ck_assert_int_eq(black_attackers[2], CHI_E3 | CHI_SEE_ROOK_VALUE << 8);
	ck_assert_int_eq(black_attackers[3], CHI_E7 | CHI_SEE_KING_VALUE << 8);
	ck_assert_int_eq(black_attackers[4], 0);

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

	ck_assert_int_eq(white_attackers[0], CHI_D2 | CHI_SEE_PAWN_VALUE << 8);
	ck_assert_int_eq(white_attackers[1], CHI_G4 | CHI_SEE_KNIGHT_VALUE << 8);
	ck_assert_int_eq(white_attackers[2], CHI_E6 | CHI_SEE_ROOK_VALUE << 8);
	ck_assert_int_eq(white_attackers[3], CHI_E2 | CHI_SEE_KING_VALUE << 8);
	ck_assert_int_eq(white_attackers[4], 0);

	ck_assert_int_eq(black_attackers[0], CHI_F4 | CHI_SEE_PAWN_VALUE << 8);
	/* Note that a bishop is counted as a knight to avoid branching.  */
	ck_assert_int_eq(black_attackers[1], CHI_F2 | CHI_SEE_KNIGHT_VALUE << 8);
	ck_assert_int_eq(black_attackers[2], CHI_H3 | CHI_SEE_QUEEN_VALUE << 8);
	ck_assert_int_eq(black_attackers[3], 0);
}
END_TEST

START_TEST(test_obvious_attackers_promotion)
{
	const char *white_fen = "1R2q1rk/2pP1Q2/8/8/8/8/8/4R2K w - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   | R |   |   | q |   | r | k | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   | p | P |   | Q |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +5.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   | R |   |   | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	chi_pos position;
	int errnum;
	unsigned white_attackers[16], black_attackers[16];
	chi_move move;

	errnum = chi_set_position(&position, white_fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Rbxe8");
	ck_assert_int_eq(errnum, 0);

	chi_obvious_attackers(&position, move, white_attackers, black_attackers);

	ck_assert_int_eq(white_attackers[0], CHI_E1 | CHI_SEE_ROOK_VALUE << 8);
	ck_assert_int_eq(white_attackers[1], CHI_D7
			| (-CHI_SEE_PAWN_VALUE + CHI_SEE_QUEEN_VALUE) << 8);
	ck_assert_int_eq(white_attackers[2], CHI_F7 | CHI_SEE_QUEEN_VALUE << 8);
	ck_assert_int_eq(white_attackers[3], 0);

	ck_assert_int_eq(black_attackers[0], CHI_G8 | CHI_SEE_ROOK_VALUE << 8);
	ck_assert_int_eq(black_attackers[1], 0);

	const char *black_fen = "4r2k/8/8/8/8/8/2Pp1q2/1r2Q1RK b - - 0 1";
	/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   | r |   |   | k | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: -5.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   | P | p |   | q |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   | r |   |   | Q |   | R | K |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/

	errnum = chi_set_position(&position, black_fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Rbxe1");
	ck_assert_int_eq(errnum, 0);

	chi_obvious_attackers(&position, move, white_attackers, black_attackers);

	ck_assert_int_eq(black_attackers[0], CHI_E8 | CHI_SEE_ROOK_VALUE << 8);
	ck_assert_int_eq(black_attackers[1], CHI_D2
			| (-CHI_SEE_PAWN_VALUE + CHI_SEE_QUEEN_VALUE) << 8);
	ck_assert_int_eq(black_attackers[2], CHI_F2 | CHI_SEE_QUEEN_VALUE << 8);
	ck_assert_int_eq(black_attackers[3], 0);

	ck_assert_int_eq(white_attackers[0], CHI_G1 | CHI_SEE_ROOK_VALUE << 8);
	ck_assert_int_eq(white_attackers[1], 0);
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

	score = chi_see(&position, move);
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

	score = chi_see(&position, move);
	ck_assert_int_eq(score, -800);
}

/* Example from
 * https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm#Position_2
 *
 * It is actually a bad example because the x-ray attacks do not change the
 * result at all.
 */
START_TEST(test_see_x_ray_attacks)
{
	const char *fen_white = "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1";
	/*
   +---+---+---+---+---+---+---+---+
 8 |   | k |   | r |   |   |   | q | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   | p | p | n |   |   |   | p | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 | p |   |   |   |   | b |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   | p |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 | P |   |   | N |   |   | P |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   | P | P |   | R |   | B | P |
   +---+---+---+---+---+---+---+---+
 1 |   |   | K |   | Q |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
	*/
	chi_pos position;
	int errnum;
	chi_move move;
	int score;

	errnum = chi_set_position(&position, fen_white);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&position, &move, "Nxe5");
	ck_assert_int_eq(errnum, 0);

	score = chi_see(&position, move);
	ck_assert_int_eq(score, -200);
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
	tcase_add_test(tc_obvious_attackers, test_obvious_attackers_promotion);
	suite_add_tcase(suite, tc_obvious_attackers);

	tc_see = tcase_create("Static Exchange Evaluation");
	tcase_add_test(tc_see, test_see_rook_wins_pawn);
	tcase_add_test(tc_see, test_see_queen_hits_defended_pawn);
	tcase_add_test(tc_see, test_see_x_ray_attacks);
	tcase_add_test(tc_see, test_see_positions);
	suite_add_tcase(suite, tc_see);

	return suite;
}
