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

#ifndef _TATEPLAY_GAME_H
# define _TATEPLAY_GAME_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/time.h>

#include "engine.h"
#include "libchi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Game {
	Engine *white;
	Engine *black;

	chi_pos pos;

	chi_move *moves;
	size_t num_moves;

	struct timeval start;

	/* Normally NULL.  Otherwise, the FEN of the starting position.  */
	char *initial_fen;

	/* PGN properties (Seven Tag Roster STR) that can be set via
	 * command-line options.
	 */
	char *event;
	char *site;
	char *round;
	char *player_white;
	char *player_black;
} Game;

extern Game *game_new();
extern void game_destroy(Game *game);

extern void game_set_event(Game *game, const char *event);
extern void game_set_site(Game *game, const char *site);
extern void game_set_round(Game *game, const char *round);
extern void game_set_fen(Game *game, const char *fen);
extern void game_set_player_white(Game *game, const char *white);
extern void game_set_player_black(Game *game, const char *black);

extern void game_print_pgn(const Game *game);

#ifdef __cplusplus
}
#endif

#endif
