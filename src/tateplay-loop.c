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

#include <errno.h>
#include <stdio.h>

#include "error.h"
#include "stdbool.h"

#include "log.h"
#include "tateplay-loop.h"

int child_exited = 0;

#define ever (;;)

bool
tateplay_loop(Game *game)
{
	fd_set active_read_fd_set, read_fd_set;
	fd_set write_fd_set;
	Engine *white = game->white;
	Engine *black = game->black;
	int i;
	struct timeval timeout;

	log_realm("info", "starting white engine");
	if (!engine_start(game->white))
		error(EXIT_FAILURE, errno, "error starting white engine '%s'",
		      game->white->nick);
	engine_negotiate(game->white);

	log_realm("info", "starting black engine");
	if (!engine_start(game->black))
		error(EXIT_FAILURE, errno, "error starting black engine '%s'",
		      game->black->nick);
	engine_negotiate(game->black);

	FD_ZERO(&active_read_fd_set);
	FD_SET(white->out, &active_read_fd_set);
	FD_SET(white->err, &active_read_fd_set);
	FD_SET(black->out, &active_read_fd_set);
	FD_SET(black->err, &active_read_fd_set);

	for ever {
		/* Multiplex input and output.  */
		FD_ZERO(&write_fd_set);
		if (white->outbuf && white->outbuf[0] != '\0') {
			FD_SET(white->in, &write_fd_set);
		}
		if (black->outbuf && black->outbuf[0] != '\0') {
			FD_SET(black->in, &write_fd_set);
		}
		
		read_fd_set = active_read_fd_set;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;
		if (select(FD_SETSIZE, &read_fd_set, &write_fd_set, NULL,
		           &timeout) < 0)
			error(EXIT_FAILURE, errno, "select failed");

		for (i = 0; i < FD_SETSIZE; ++i) {
			/* Input available.  */
			if (FD_ISSET(i, &read_fd_set)) {
				if (white->out == i) {
					if (!engine_read_stdout(white))
						return false;
				} else if(white->err == i) {
					if (!engine_read_stderr(white))
						return false;
				} else if(black->out == i) {
					if (!engine_read_stdout(black))
						return false;
				} else if(black->err == i) {
					if (!engine_read_stderr(black))
						return false;
				}
			}
		}

		for (i = 0; i < FD_SETSIZE; ++i) {
			/* Output possible.  */
			if (FD_ISSET(i, &write_fd_set)) {
				if (white->in == i) {
					if (!engine_write_stdin(white))
						return false;
				} else if (black->in == i) {
					if (!engine_write_stdin(black))
						return false;
				}
			}
		}

		if (!game_ping(game))
			break;
	}

	return true;
}