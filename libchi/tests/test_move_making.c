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

#include "libchi.h"

START_TEST(test_pawn_moves)
	chi_pos pos;
	chi_init_position(&pos);
	chi_move move1, move2;
	int errnum;
	char *wanted;
	char *got;
	
	errnum = chi_parse_move(&pos, &move1, "e4");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move1);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
	
	errnum = chi_parse_move(&pos, &move2, "e5");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move2);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move2);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move1);
	ck_assert_int_eq(errnum, 0);

	wanted = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST

START_TEST(test_white_capture)
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | r | n | b | q | k | b | n | r | En passant possible on file d.
   +---+---+---+---+---+---+---+---+ White king castle: yes.
 7 | p | p | p |   | p | p | p | p | White queen castle: yes.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   |   |   |   |   |   |   | Black queen castle: yes.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   | p |   |   |   |   | Half moves: 2.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   | P |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P | P | P | P |   | P | P | P |
   +---+---+---+---+---+---+---+---+
 1 | R | N | B | Q | K | B | N | R |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

	errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "exd");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;

START_TEST(test_black_capture)
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | r | n | b | q | k | b | n | r | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: yes.
 7 | p | p | p |   | p | p | p | p | White queen castle: yes.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   |   |   |   |   |   |   | Black queen castle: yes.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   | p |   |   |   |   | Half moves: 3.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   | P |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   | N |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P | P | P | P |   | P | P | P |
   +---+---+---+---+---+---+---+---+
 1 | R |   | B | Q | K | B | N | R |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 0 2";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

	errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "dxe");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "rnbqkbnr/ppp1pppp/8/8/4p3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 0 3";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;


START_TEST(test_white_ep_capture)
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant possible on file f.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   | P | p |   |   | Half moves: 2.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 | K |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */
	const char *fen = "k7/8/8/4Pp2/8/8/8/K7 w - f6 0 2";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

	errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "exf6");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "k7/8/5P2/8/8/8/8/K7 b - - 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;


START_TEST(test_black_ep_capture)
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | k |   |   |   |   |   |   |   | En passant possible on file f.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   | P | p |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 | K |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
 */

	const char *fen = "k7/8/8/8/5Pp1/8/8/K7 b - f3 0 1";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

	errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "gxf3");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "k7/8/8/8/8/5p2/8/K7 w - - 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;

/* White rook attacks black rook */
START_TEST(test_ks_black_rook_capture)
	chi_pos pos;

    /*    
    a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   | k |   |   | r | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   | R |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   | K |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
*/

    const char *fen = "4k2r/8/8/8/8/8/7R/4K3 w k - 0 1";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

    errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "Rxh8");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "4k2R/8/8/8/8/8/8/4K3 b - - 0 1";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;

START_TEST(test_ks_white_rook_capture)
	chi_pos pos;
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   | k |   |   |   | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: yes.
 7 |   |   |   |   |   |   |   | r | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 1.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   | K |   |   | R |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
*/
    const char *fen = "4k3/7r/8/8/8/8/8/4K2R b K - 0 1";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

    errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "Rxh1");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "4k3/8/8/8/8/8/8/4K2r w - - 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;


START_TEST(test_qs_black_rook_capture)
	chi_pos pos;

    /*    
    a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 |   |   |   |   | k |   |   | r | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 |   |   |   |   |   |   |   |   | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: yes.
 6 |   |   |   |   |   |   |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   |   |   |   |   |   |   |   | Half moves: 0.
   +---+---+---+---+---+---+---+---+ Next move: white.
 4 |   |   |   |   |   |   |   |   | Material: +0.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   |   |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 |   |   |   |   |   |   |   | R |
   +---+---+---+---+---+---+---+---+
 1 |   |   |   |   | K |   |   |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
*/

    const char *fen = "r3k3/8/8/8/8/8/R7/4K3 w q - 0 2";
	chi_move move;
	const char *wanted;
	char *got;
	int errnum;

    errnum = chi_set_position(&pos, fen);
	ck_assert_int_eq(errnum, 0);

	errnum = chi_parse_move(&pos, &move, "Rxa8");
	ck_assert_int_eq(errnum, 0);

	errnum = chi_apply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = "R3k3/8/8/8/8/8/8/4K3 b - - 0 2";
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);

	errnum = chi_unapply_move(&pos, move);
	ck_assert_int_eq(errnum, 0);
	wanted = fen;
	got = chi_fen(&pos);
	ck_assert_str_eq(wanted, got);
	free(got);
END_TEST;

Suite *
move_making_suite(void)
{
	Suite *suite;
	TCase *tc_pawn;
    TCase *tc_rook;
	
	suite = suite_create("Make/Unmake Moves");

	tc_pawn = tcase_create("Pawn Moves");
	tcase_add_test(tc_pawn, test_pawn_moves);
	tcase_add_test(tc_pawn, test_white_capture);
	tcase_add_test(tc_pawn, test_black_capture);
	tcase_add_test(tc_pawn, test_white_ep_capture);
    tcase_add_test(tc_pawn, test_black_ep_capture);
	suite_add_tcase(suite, tc_pawn);

    tc_rook = tcase_create("Castling States");
    tcase_add_test(tc_rook, test_ks_black_rook_capture);
    tcase_add_test(tc_rook, test_ks_white_rook_capture);
    tcase_add_test(tc_rook, test_qs_black_rook_capture);
    suite_add_tcase(suite, tc_rook);

	return suite;
}



