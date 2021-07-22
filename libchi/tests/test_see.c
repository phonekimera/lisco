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

#define P 100
#define N 300
#define B 300
#define R 500
#define Q 900
#define K 10000

static unsigned piece_values[6] = {
	P, N, B, R, Q, K
};

typedef struct SEETest {
	const char *filename;
	int lineno;
	const char *fen;
	const char *san;
	int score;
} SEETest;

/* Taken from vajolet.  */
#define Move(f, t) #f "FIXME" #t
static SEETest tests[] = {
	/* Capture initial move.  */
	{
		__FILE__,
		__LINE__,
	 	"3r3k/3r4/2n1n3/8/3p4/2PR4/1B1Q4/3R3K w - - 0 1",
		 "Rxd4",
		 P - R + N - P
	},
	{
		__FILE__,
		__LINE__,
		"1k1r4/1ppn3p/p4b2/4n3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1",
		"Nxe5",
		N - N + B - R + N
	},
	{
		__FILE__,
		__LINE__,
		"1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1",
		"Nxe5",
		P - N
	},
	{
		__FILE__,
		__LINE__,
		"rnb2b1r/ppp2kpp/5n2/4P3/q2P3B/5R2/PPP2PPP/RN1QKB2 w Q - 1 1",
		"Bxf6",
		N - B + P
	},
	{
		__FILE__,
		__LINE__,
		"r2q1rk1/2p1bppp/p2p1n2/1p2P3/4P1b1/1nP1BN2/PP3PPP/RN1QR1K1 b - - 1 1",
		"Bxf3",
		N - B
	},
	{
		__FILE__,
		__LINE__,
		"r1bqkb1r/2pp1ppp/p1n5/1p2p3/3Pn3/1B3N2/PPP2PPP/RNBQ1RK1 b kq - 2 1",
		"Nxd4",
		P - N + N - P
	},
	{
		__FILE__,
		__LINE__,
		"r1bq1r2/pp1ppkbp/4N1p1/n3P1B1/8/2N5/PPP2PPP/R2QK2R w KQ - 2 1",
		"Nxg7",
		B - N
	},
	{
		__FILE__,
		__LINE__,
		"r1bq1r2/pp1ppkbp/4N1pB/n3P3/8/2N5/PPP2PPP/R2QK2R w KQ - 2 1",
		"Nxg7",
		B
	},
	{
		__FILE__,
		__LINE__,
		"rnq1k2r/1b3ppp/p2bpn2/1p1p4/3N4/1BN1P3/PPP2PPP/R1BQR1K1 b kq - 0 1",
		"Bxh2",
		P - B
	},
	{
		__FILE__,
		__LINE__,
		"rn2k2r/1bq2ppp/p2bpn2/1p1p4/3N4/1BN1P3/PPP2PPP/R1BQR1K1 b kq - 5 1",
		"Bxh2",
		P
	},
	{
		__FILE__,
		__LINE__,
		"r2qkbn1/ppp1pp1p/3p1rp1/3Pn3/4P1b1/2N2N2/PPP2PPP/R1BQKB1R b KQq - 2 1",
		"Bxf3",
		N - B + P
	},
	{
		__FILE__,
		__LINE__,
		"rnbq1rk1/pppp1ppp/4pn2/8/1bPP4/P1N5/1PQ1PPPP/R1B1KBNR b KQ - 1 1",
		"Bxc3",
		N - B
	},
	{
		__FILE__,
		__LINE__,
		"r4rk1/3nppbp/bq1p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - - 1 1",
		"Qxb2",
		P - Q
	},
	{
		__FILE__,
		__LINE__,
		"r4rk1/1q1nppbp/b2p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - - 1 1",
		"Nxd5",
		P - N
	},
	{
		__FILE__,
		__LINE__,
		"1r3r2/5p2/4p2p/2k1n1P1/2PN1nP1/1P3P2/8/2KR1B1R b - - 0 29",
		"Rxb3",
		P - R
	},
	{
		__FILE__,
		__LINE__,
		"1r3r2/5p2/4p2p/4n1P1/kPPN1nP1/5P2/8/2KR1B1R b - - 0 1",
		"Rxb4",
		P
	},
	{
		__FILE__,
		__LINE__,
		"2r2rk1/5pp1/pp5p/q2p4/P3n3/1Q3NP1/1P2PP1P/2RR2K1 b - - 1 22",
		"Rxc1",
		R - R
	},
	{
		__FILE__,
		__LINE__,
		"1r3r1k/p4pp1/2p1p2p/qpQP3P/2P5/3R4/PP3PP1/1K1R4 b - - 0 1",
		"Qxa2",
		P - Q
	},
	{
		__FILE__,
		__LINE__,
		"1r5k/p4pp1/2p1p2p/qpQP3P/2P2P2/1P1R4/P4rP1/1K1R4 b - - 0 1",
		"Qxa2",
		P
	},
	{
		__FILE__,
		__LINE__,
		"r2q1rk1/1b2bppp/p2p1n2/1ppNp3/3nP3/P2P1N1P/BPP2PP1/R1BQR1K1 w - - 4 14",
		"Nxe7",
		B - N
	},
	{
		__FILE__,
		__LINE__,
		"rnbqrbn1/pp3ppp/3p4/2p2k2/4p3/3B1K2/PPP2PPP/RNB1Q1NR w - - 0 1",
		"Bxe4",
		P
	},
		/* Non-capture initial move. */
	{
		__FILE__,
		__LINE__,
		"rnb1k2r/p3p1pp/1p3p1b/7n/1N2N3/3P1PB1/PPP1P1PP/R2QKB1R w KQkq - 0 1",
		"Nxd6",
		0 - N + P
	},
	{
		__FILE__,
		__LINE__,
		"r1b1k2r/p4npp/1pp2p1b/7n/1N2N3/3P1PB1/PPP1P1PP/R2QKB1R w KQkq - 0 1",
		"Nd6",
		0 - N + N
	},
	{
		__FILE__,
		__LINE__,
		"2r1k2r/pb4pp/5p1b/2KB3n/4N3/2NP1PB1/PPP1P1PP/R2Q3R w k - 0 1",	
		"Bc6",
		0 - B
	},
	{
		__FILE__,
		__LINE__,
		"2r1k2r/pb4pp/5p1b/2KB3n/1N2N3/3P1PB1/PPP1P1PP/R2Q3R w k - 0 1",
		"Bc6",
		0 - B + B
	},
	{
		__FILE__,
		__LINE__,
		"2r1k3/pbr3pp/5p1b/2KB3n/1N2N3/3P1PB1/PPP1P1PP/R2Q3R w - - 0 1",
		"Bc6",
		0 - B + B - N
	},
	/* Initial move promotion. */
	{
		__FILE__,
		__LINE__,
		"5k2/p2P2pp/1b6/1p6/1Nn1P1n1/8/PPP4P/R2QK1NR w KQ - 0 1",
		"d8Q",
		0 + ( -P + Q ) - Q + B
	},
	{
		__FILE__,
		__LINE__,
		"4kbnr/p1P1pppp/b7/4q3/7n/8/PP1PPPPP/RNBQKBNR w KQk - 0 1",
		"c8q",
		0 + (-P + Q) - Q
	},
	{
		__FILE__,
		__LINE__,
		"4kbnr/p1P1pppp/b7/4q3/7n/8/PPQPPPPP/RNB1KBNR w KQk - 0 1",
		"c8q",
		0 + (-P + Q) - Q + B
	},
	{
		__FILE__,
		__LINE__,
		"4kbnr/p1P1pppp/b7/4q3/7n/8/PPQPPPPP/RNB1KBNR w KQk - 0 1",
		"c8n",
		0 + (-P + N)
	},
	/* Initial move En Passant.  */
	{
		__FILE__,
		__LINE__,
		"4kbnr/p1P4p/b1q5/5pP1/4n3/5Q2/PP1PPP1P/RNB1KBNR w KQk f6 0 2",
		"gxf6",
		P - P
	},
	{
		__FILE__,
		__LINE__,
		"4kbnr/p1P4p/b1q5/5pP1/4n2Q/8/PP1PPP1P/RNB1KBNR w KQk f6 0 2",
		"gxf6",
		P - P
	},
	/* Initial move capture promotion.  */
	{
		__FILE__,
		__LINE__,
		"1n2kb1r/p1P4p/2qb4/5pP1/4n2Q/8/PP1PPP1P/RNB1KBNR w KQk - 0 1",	
		"cxb8q",
		N + (-P + Q) -Q
	},
	{
		__FILE__,
		__LINE__,
		"rnbqk2r/pp3ppp/2p1pn2/3p4/3P4/N1P1BN2/PPB1PPPb/R2Q1RK1 w kq - 0 1",
		"Kxh2",
		B
	},
	{
		__FILE__,
		__LINE__,
		"3N4/2K5/2n5/1k6/8/8/8/8 b - - 0 1",
		"Nxd8",
		0
	},
	/* Promotion inside the loop. */
	{
		__FILE__,
		__LINE__,
		"3N4/2P5/2n5/1k6/8/8/8/4K3 b - - 0 1",
		"Nxd8",
		N - (N - P + Q)
	},
	{
		__FILE__,
		__LINE__,
		"3N4/2P5/2n5/1k6/8/8/8/4K3 b - - 0 1",
		"Nb8",
		0 - (N - P + Q)
	},
	{
		__FILE__,
		__LINE__,
		"3n3r/2P5/8/1k6/8/8/3Q4/4K3 w - - 0 1",
		"Qxd8",
		N
	},
	{
		__FILE__,
		__LINE__,
		"3n3r/2P5/8/1k6/8/8/3Q4/4K3 w - - 0 1",
		"cxdq",
		(N - P + Q) - Q + R
	},
	/* Double promotion inside the loop. */
	{
		__FILE__,
		__LINE__,
		"r2n3r/2P1P3/4N3/1k6/8/8/8/4K3 w - - 0 1",
		"Nxd8",
		N
	},
	{
		__FILE__,
		__LINE__,
		"8/8/8/1k6/6b1/4N3/2p3K1/3n4 w - - 0 1",
		"Nxd1",
		N - (N - P + Q)
	},
	{
		__FILE__,
		__LINE__,
		"8/8/1k6/8/8/2N1N3/2p1p1K1/3n4 w - - 0 1",
		"Nexd1",
		N - (N - P + Q)
	},
	{
		__FILE__,
		__LINE__,
		"8/8/1k6/8/8/2N1N3/4p1K1/3n4 w - - 0 1",
		"Ncxd1",
		N - (N - P + Q) + Q
	},
	{
		__FILE__,
		__LINE__,
		"r1bqk1nr/pppp1ppp/2n5/1B2p3/1b2P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
		"O-O",
		0
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

	unsigned cpo_piece_values[6] = {
		100, 325, 325, 500, 900, 10000
	};

	score = chi_see(&position, move, cpo_piece_values);
	ck_assert_int_eq(score, -225);
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
	tcase_add_test(tc_see, test_see_x_ray_attacks);
	tcase_add_test(tc_see, test_see_positions);
	suite_add_tcase(suite, tc_see);

	return suite;
}
