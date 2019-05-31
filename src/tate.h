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

#ifndef TATE_H
# define TATE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

/* Events from main loop.  */
#define EVENT_CONTINUE      0x00000000
#define EVENT_TERMINATE     0x00000001
#define EVENT_MOVE_NOW      0x00000002
#define EVENT_GAME_OVER     0x00000004
#define EVENT_STOP_THINKING 0x00000010

#define EVENT_WANT_HINT     0x00010000
#define EVENT_WANT_BOOK     0x00020000
#define EVENT_POST          0x00040000
#define EVENT_UNPOST        0x00080000
#define EVENT_PONDER        0x00100000
#define EVENT_NO_PONDER     0x00200000

#define EVENTMASK_TERMINATE   EVENT_TERMINATE
#define EVENTMASK_ENGINE_STOP 0x1f

#define MAX_PLY 512
#define MATE    -10000
#define INF     ((-(MATE)) << 1)
// FIXME: contempt factor!
#define DRAW    0

#include "time-management.h"

struct game_hist_entry {
    bitv64   signature;
    chi_move move;
    chi_pos  pos;
};

extern struct game_hist_entry* game_hist;
extern unsigned int game_hist_ply;

extern chi_pos current;
extern chi_zk_handle zk_handle;

extern char* name;
extern int xboard;
extern int force;
extern int game_over;
extern int thinking;
extern int pondering;
extern int allow_pondering;
extern int ics;
extern int computer;
extern chi_color_t engine_color;
extern int max_depth;
extern int event_pending;
extern int post;
extern int mate_announce;
extern int current_score;

extern int isa_tty;

extern TimePoint min_thinking_time;
extern TimePoint move_overhead;
extern TimePoint slow_mover;
extern TimePoint npmsec;

extern int get_event(void);
extern int handle_epd(const char* epdstr, chi_epd_pos* epd);
extern int handle_epdfile (const char* command);
extern int handle_go (chi_epd_pos* epd);

#endif
