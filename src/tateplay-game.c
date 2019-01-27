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
#include "xmalloca-debug.h"

#include "tateplay-game.h"

static bool legal_tag_value(const char *value);

Game *
game_new(const char *fen)
{
	Game *self = xmalloc(sizeof *self);

	memset(self, 0, sizeof *self);

	self->white = engine_new(self);
	self->black = engine_new(self);
	chi_init_position(&self->pos);

	// FIXME! This should start, when the time control starts.
	gettimeofday(&self->start, NULL);

	return self;
}

void
game_destroy(Game *self)
{
	if (self == NULL) return;

	if (self->white) engine_destroy(self->white);
	if (self->black) engine_destroy(self->black);
	if (self->moves) free(self->moves);
	if (self->initial_fen) free(self->initial_fen);

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
		self->started = chi_true;
	}

	return chi_true;
}