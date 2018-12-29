/* tate.h - global variables... :-(
 * Copyright (C) 2002 Guido Flohr (guido@imperia.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef TATE_H
# define TATE_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <system.h>

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
#define MATE    -9999
#define INF     ((-(MATE)) << 1)
#define DRAW    (0)

struct game_hist_entry {
    bitv64   signature;
    chi_move move;
    chi_pos  pos;
    /* Bitmask for keeping track of castling state. 
       0x1 - white can castle on one side
       0x2 - white can castle on two sides
       0x4 - white has castled
       0x10 - black can castle on one side
       0x20 - black can castle on two sides
       0x40 - black has castled
    */
    int castling_state;
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
extern int max_ply;
extern int event_pending;
extern int post;
extern int mate_announce;

extern int get_event PARAMS ((void));

#endif
