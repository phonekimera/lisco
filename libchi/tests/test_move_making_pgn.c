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
#include <stdarg.h>
#include <stdio.h>

#include "xalloc.h"

#include "libchi.h"

typedef struct test_game {
	const char *filename;
	const char *lineno;
	const char *event;
	const char *site;
	const char *date;
	const char *round;
	const char *white;
	const char *black;
	const char *eco;
	const char *result;
} TestGame;

/* Each game has the following structure:
 *
 *		Source filename
 *		Line number
 *		Event
 *		Site
 *		Date
 *		Round
 *		White
 *		Black
 *		ECO opening code
 *		Result
 *		Moves (NULL terminated)
 *
 * NULL as the source of filenames marks the end of games.
 */
#define STRINGIFY(INT) #INT
#define STRING(s) STRINGIFY(s)
static const char *tests[] = {
	__FILE__, STRING(__LINE__),
	"Berlin",
	"Berlin GER",
	"1852.??.??",
	"?",
	"Adolf Anderssen",
	"Jean Dufresne",
	"C52",
	"1-0",
	"e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "b4", "Bxb4", "c3", "Ba5",
	"d4", "exd4", "O-O", "d3", "Qb3", "Qf6", "e5", "Qg6", "Re1", "Nge7",
	"Ba3", "b5", "Qxb5", "Rb8", "Qa4", "Bb6", "Nbd2", "Bb7", "Ne4", "Qf5",
	"Bxd3", "Qh5", "Nf6+", "gxf6", "exf6", "Rg8", "Rad1", "Qxf3", "Rxe7+",
	"Nxe7", "Qxd7+", "Kxd7", "Bf5+", "Ke8", "Bd7+", "Kf8", "Bxe7#", NULL,

	__FILE__, STRING(__LINE__),
	"London",
	"London ENG",
	"1851.06.21",
	"?",
	"Adolf Anderssen",
	"Lionel Adalbert Bagration Felix Kieseritzky",
	"C33",
	"1-0",
	"e4", "e5", "f4", "exf4", "Bc4", "Qh4+", "Kf1", "b5", "Bxb5", "Nf6",
	"Nf3", "Qh6", "d3", "Nh5", "Nh4", "Qg5", "Nf5", "c6", "g4", "Nf6",
	"Rg1", "cxb5", "h4", "Qg6", "h5", "Qg5", "Qf3", "Ng8", "Bxf4", "Qf6",
	"Nc3", "Bc5", "Nd5", "Qxb2", "Bd6", "Bxg1", "e5", "Qxa1+", "Ke2",
	"Na6", "Nxg7+", "Kd8", "Qf6+", "Nxf6", "Be7#", NULL,

	__FILE__, STRING(__LINE__),
	"London m4 ;HCL 18",
	"London ENG",
	"1834.??.??",
	"?",
	"Alexander McDonnell",
	"Louis Charles Mahe De La Bourdonnais",
	"B32",
	"0-1",
	"e4", "c5", "Nf3", "Nc6", "d4", "cxd4", "Nxd4", "e5", "Nxc6", "bxc6",
	"Bc4", "Nf6", "Bg5", "Be7", "Qe2", "d5", "Bxf6", "Bxf6", "Bb3", "O-O",
	"O-O", "a5", "exd5", "cxd5", "Rd1", "d4", "c4", "Qb6", "Bc2", "Bb7",
	"Nd2", "Rae8", "Ne4", "Bd8", "c5", "Qc6", "f3", "Be7", "Rac1", "f5",
	"Qc4+", "Kh8", "Ba4", "Qh6", "Bxe8", "fxe4", "c6", "exf3", "Rc2",
	"Qe3+", "Kh1", "Bc8", "Bd7", "f2", "Rf1", "d3", "Rc3", "Bxd7", "cxd7",
	"e4", "Qc8", "Bd8", "Qc4", "Qe1", "Rc1", "d2", "Qc5", "Rg8", "Rd1",
	"e3", "Qc3", "Qxd1", "Rxd1", "e2", NULL,

	NULL
};

static void
report_failure(const TestGame *game,
               unsigned int move_number, const char *move,
			   const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	fprintf(stderr, "Failed test at %s:%s, \n", game->filename, game->lineno);

	if (move) {
		if (move_number & 1) {
			fprintf(stderr, "Move %u. ... %s\n", 1 + move_number / 2, move);
		} else {
			fprintf(stderr, "Move %u. %s\n", 1 + move_number / 2, move);
		}
	}

	vfprintf(stderr, fmt, ap);

	va_end(ap);

	ck_abort();
}

static const char **
test_game(const char *strings[])
{
	TestGame game;
	size_t num_moves, i;
	const char **retval;
	chi_pos pos;
	int errnum;
	chi_move *moves;
	const char **fens;

	game.filename = *strings++;
	game.lineno = *strings++;
	game.event = *strings++;
	game.site = *strings++;
	game.date = *strings++;
	game.round = *strings++;
	game.white = *strings++;
	game.black = *strings++;
	game.eco = *strings++;
	game.result = *strings++;

	for (num_moves = 0; strings[num_moves]; ++num_moves) {}
	retval = strings + num_moves + 1;

	moves = xcalloc(num_moves, sizeof moves[0]);
	fens = xcalloc(num_moves, sizeof fens[0]);

	chi_init_position(&pos);
	
	for (i = 0; i < num_moves; ++i) {
		errnum = chi_parse_move(&pos, &moves[i], strings[i]);
		if (errnum) {
			report_failure(&game, i, strings[i], "parsing move failed: %s\n",
			               chi_strerror(errnum));
		}
		errnum = chi_apply_move(&pos, moves[i]);
		if (errnum) {
			report_failure(&game, i, strings[i], "applying move failed: %s\n",
			               chi_strerror(errnum));
		}
	}

	free(fens);
	free(moves);

	return retval;
}

START_TEST(test_games)
	const char **current = tests;

	while (*current) {
		current = test_game(current);
	}
END_TEST

Suite *
move_making_suite_pgn(void)
{
	Suite *suite;
	TCase *tc_pgn;
	
	suite = suite_create("Play and Unplay PGN Games");

	tc_pgn = tcase_create("PGNs");
	tcase_add_test(tc_pgn, test_games);
	suite_add_tcase(suite, tc_pgn);

	return suite;
}
