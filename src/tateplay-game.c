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

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "stdbool.h"

#include "stringbuf.h"

#include "xmalloca-debug.h"
#include "display-board.h"
#include "log.h"
#include "tateplay-game.h"

static bool legal_tag_value(const char *value);

static void game_start(Game *game);
static chi_bool game_check_over(Game *game);

Game *
game_new(const char *fen)
{
	Game *self = xmalloc(sizeof *self);

	memset(self, 0, sizeof *self);

	self->white = engine_new(self);
	self->black = engine_new(self);
	chi_init_position(&self->pos);

	self->fen = xmalloc(sizeof self->fen[0]);
	self->fen[0] = chi_fen(&self->pos);

	return self;
}

void
game_destroy(Game *self)
{
	size_t i;

	if (self == NULL) return;

	if (self->white) engine_destroy(self->white);
	if (self->black) engine_destroy(self->black);
	if (self->moves) free(self->moves);

	for (i = 0; i <= self->num_moves; ++i) {
		chi_free(self->fen[i]);
	}
	free(self->fen);

	free(self);
}

void
game_set_event(Game *self, const char *event)
{
	if (!legal_tag_value(event))
		error(EXIT_FAILURE, 0, "Event must not contain quote or newlines");

	if (self->event)
		free(self->event);

	self->event = xstrdup(event);
}

void
game_set_site(Game *self, const char *site)
{
	if (!legal_tag_value(site))
		error(EXIT_FAILURE, 0, "Site must not contain quote or newlines");

	if (self->site)
		free(self->site);

	self->site = xstrdup(site);
}

void
game_set_round(Game *self, const char *round)
{
	if (!legal_tag_value(round))
		error(EXIT_FAILURE, 0, "Round must not contain quote or newlines");

	if (self->round)
		free(self->round);

	self->round = xstrdup(round);
}

void
game_set_player_white(Game *self, const char *name)
{
	if (!legal_tag_value(name)) {
		error(EXIT_FAILURE, 0,
		      "White player name must not contain quote or newlines");
	}

	if (self->player_white)
		free(self->player_white);

	self->player_white = xstrdup(name);
}

void
game_set_player_black(Game *self, const char *name)
{
	if (!legal_tag_value(name)) {
		error(EXIT_FAILURE, 0,
		      "Black player name must not contain quote or newlines");
	}

	if (self->player_black)
		free(self->player_black);

	self->player_black = xstrdup(name);
}

void
game_print_pgn(const Game *self)
{
	struct tm *tm;
	const char *white_player;
	const char *black_player;
	size_t i;
	chi_pos pos;
	char *buf = NULL;
	unsigned int bufsize;
	int errnum;
	chi_stringbuf *sb;
	size_t last_space, column;
	char *moves;
	size_t length;

	printf("[Site \"%s\"]\012", self->site ? self->site : "?");
	printf("[Event \"%s\"]\012", self->event ? self->event : "?");

	tm = localtime(&self->start.tv_sec);
	printf("[Date \"%04u.%02u.%02u]\"\n",
	       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

	printf("[Event \"%s\"]\012", self->event ? self->event : "?");

	if (self->player_white)
		white_player = self->player_white;
	else if (self->white->nick)
		white_player = self->white->nick;
	else
		white_player = "?";
	printf("[White \"%s\"]\n", white_player);

	if (self->player_black)
		black_player = self->player_black;
	else if (self->white->nick)
		black_player = self->black->nick;
	else
		white_player = "?";
	printf("[Black \"%s\"]\n", black_player);

	printf("\n");

	chi_init_position(&pos);
	sb = _chi_stringbuf_new(128);

	for (i = 0; i < self->num_moves; ++i) {
		errnum = chi_print_move(&pos, self->moves[i], &buf, &bufsize, 1);
		if (errnum) {
			error(EXIT_FAILURE, 0, "internal error: cannot generate move: %s",
			      chi_strerror(errnum));
		}

		if (!(i & 1)) {
			if (i)
				_chi_stringbuf_append_char(sb, ' ');

			_chi_stringbuf_append_unsigned(sb, 1 + (i >> 1), 10);
			_chi_stringbuf_append_char(sb, '.');
		}
		_chi_stringbuf_append_char(sb, ' ');
		_chi_stringbuf_append(sb, buf);

		errnum = chi_apply_move(&pos, self->moves[i]);
		if (errnum) {
			error(EXIT_FAILURE, 0, "internal error: cannot apply move: %s",
			      chi_strerror(errnum));
		}
	}
	if (buf)
		chi_free(buf);

	/* Break lines.  */
	last_space = column = 0;
	moves = (char *) _chi_stringbuf_get_string(sb);
	length = strlen(moves);

	for (i = 0; i < length; ++i) {
		if (column >= 80) {
			moves[last_space] = '\n';
			column = i - last_space;
		} else {
			if (' ' == moves[i] && '.' != moves[i - 1]) {
				last_space = i;
			}
			++column;
		}
	}

	printf("%s\n", moves);

	_chi_stringbuf_destroy(sb);
}

static bool
legal_tag_value(const char *value)
{
	const char *ptr = value;

	while (*ptr) {
		if (*ptr == '"' || *ptr == '\n')
			return false;
		++ptr;
	}

	return true;
}

chi_bool
game_ping(Game *self)
{
	/* FIXME! Check engine timeout! */

	if (!self->started
	    && self->white->state == ready && self->black->state == ready) {
		game_start(self);
	}

	return chi_true;
}

chi_bool
game_do_move(Game *self, const char *movestr)
{
	chi_move move;
	int errnum;
	Engine *mover = chi_on_move(&self->pos) == chi_white ?
		self->white : self->black;
	Engine *waiter = chi_on_move(&self->pos) == chi_white ?
		self->black : self->white;
	chi_pos old_pos;
	char *fen;

	errnum = chi_parse_move(&self->pos, &move, movestr);
	if (errnum) {
		log_engine_fatal(mover->nick, "move %s: %s", movestr,
		                 chi_strerror(errnum));
	}

	chi_copy_pos(&old_pos, &self->pos);

	errnum = chi_apply_move(&self->pos, move);
	if (errnum) {
		log_engine_fatal(mover->nick, "move %s: %s", movestr,
		                 chi_strerror(errnum));
	}

	if (verbose) {
		fen = chi_fen(&self->pos);
		if (chi_on_move(&self->pos) == chi_white)
			log_engine_error(self->white->nick, "FEN: %s\n", fen);
		else 
			log_engine_error(self->black->nick, "FEN: %s\n", fen);
		display_board(stderr, &self->pos);
		chi_free(fen);
	}

	/* Extend the game history.  */
	self->moves = xrealloc(self->moves,
	                       sizeof self->moves[0] * ++self->num_moves);
	self->moves[self->num_moves - 1] = move;
	self->fen = xrealloc(self->fen,
	                     sizeof self->fen[0] * (self->num_moves + 1));
	self->fen[self->num_moves] = chi_fen(&self->pos);

	if (game_check_over(self)) {
		/* FIXME! Tell engines to quit! */
		return false;
	}

	engine_ponder(mover, &self->pos);
	engine_think(waiter, &old_pos, move);

	return true;
}

static chi_bool
game_check_over(Game *self)
{
	size_t seen = 1;
	size_t i;
	char *fen;
	size_t relevant_fen_length;
	int spaces = 0;

	if (chi_game_over(&self->pos, &self->result)) {
		self->white->state = finished;
		self->black->state = finished;
		return true;
	}

	if (self->pos.half_move_clock > 100) {
		self->result = chi_result_draw_by_50_moves_rule;
		self->white->state = finished;
		self->black->state = finished;
		return true;
	}

	if (self->pos.half_move_clock < 8)
		return false;

	/* Check for draw by 3-fold repetition.  First find the length of the
	 * FEN substring that holds the actual position, without the half move
	 * clock and move number.
	 */
	fen = self->fen[self->num_moves];
	for (relevant_fen_length = 0; spaces < 4; ++relevant_fen_length) {
		if (fen[relevant_fen_length] == ' ')
			++spaces;
	}
	--relevant_fen_length;

	for (i = 2; i <= self->pos.half_move_clock; i += 2) {
		if (strncmp(fen, self->fen[self->num_moves - i],
		            relevant_fen_length) == 0)
			++seen;
		if (seen >= 3) {
			self->result = chi_result_draw_by_50_moves_rule;
			self->white->state = finished;
			self->black->state = finished;
			return true;
		}
	}

	return false;
}

static void
game_start(Game *self)
{
	chi_pos copy;

	self->started = chi_true;

	chi_copy_pos(&copy, &self->pos);
	engine_think(self->white, &self->pos, (chi_move) 0);
	engine_ponder(self->black, &self->pos);
}
