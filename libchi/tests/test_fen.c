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

START_TEST(test_immortal_game)
	const char *fen = "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 b - - 0 23";
/*
     a   b   c   d   e   f   g   h
   +---+---+---+---+---+---+---+---+
 8 | r |   | b | k |   |   |   | r | En passant not possible.
   +---+---+---+---+---+---+---+---+ White king castle: no.
 7 | p |   |   | p | B | p | N | p | White queen castle: no.
   +---+---+---+---+---+---+---+---+ Black king castle: no.
 6 | n |   |   |   |   | n |   |   | Black queen castle: no.
   +---+---+---+---+---+---+---+---+ Half move clock (50 moves): 0.
 5 |   | p |   | N | P |   |   | P | Half moves: 45.
   +---+---+---+---+---+---+---+---+ Next move: black.
 4 |   |   |   |   |   |   | P |   | Material: -21.
   +---+---+---+---+---+---+---+---+ Black has castled: no.
 3 |   |   |   | P |   |   |   |   | White has castled: no.
   +---+---+---+---+---+---+---+---+
 2 | P |   | P |   | K |   |   |   |
   +---+---+---+---+---+---+---+---+
 1 | q |   |   |   |   |   | b |   |
   +---+---+---+---+---+---+---+---+
     a   b   c   d   e   f   g   h
*/
END_TEST

Suite *
fen_suite(void)
{
	Suite *suite;
	TCase *tc_basic;
	
	suite = suite_create("Forsyth-Edwards Notation (FEN)");

	tc_basic = tcase_create("Basic");
	tcase_add_test(tc_basic, test_immortal_game);
	suite_add_tcase(suite, tc_basic);

	return suite;
}
