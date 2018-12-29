/* brain.h - find the correct move...
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

#ifndef BRAIN_H
# define BRAIN_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <system.h>

#include <libchi.h>

#include "tate.h"

#define DEBUG_BRAIN 0

#define TRACK_MATERIAL 1

#define TRACK_ZOBRIST 0

#define NULL_R 2

#if 0
#define MOBILITY_SHIFT 1
#define MIN_EVAL_DIFF (1 << MOBILITY_SHIFT)
#endif

#define MIN_EVAL_DIFF 100

extern bitv64 total_nodes;
extern bitv64 total_centiseconds;

typedef struct path {
    int length;
    chi_move moves[MAX_PLY];
} PATH;

enum move_state {
    move_state_init = 0,
#define move_state_init 0

    move_state_pv = move_state_init + 1,
#define move_state_pv (move_state_init + 1)

    move_state_captures = move_state_pv + 1,
#define move_state_captures (move_state_pv + 1)

    move_state_king_castlings = move_state_captures + 1,
#define move_state_king_castlings (move_state_captures + 1)

    move_state_pawn_double_steps = move_state_king_castlings + 1,
#define move_state_pawn_double_steps (move_state_king_castlings + 1)

    move_state_pawn_single_steps = move_state_pawn_double_steps + 1,
#define move_state_pawn_single_steps (move_state_pawn_double_steps + 1)

    move_state_knight_moves = move_state_pawn_single_steps + 1,
#define move_state_knight_moves (move_state_pawn_single_steps + 1)

    move_state_bishop_moves = move_state_knight_moves + 1,
#define move_state_bishop_moves (move_state_knight_moves + 1)

    move_state_rook_moves = move_state_bishop_moves + 1,
#define move_state_rook_moves (move_state_bishop_moves + 1)

    move_state_king_moves = move_state_rook_moves + 1,
#define move_state_king_moves (move_state_rook_moves + 1)

};

typedef struct tree {
    bitv64 signatures[MAX_PLY + 1];

    chi_pos pos;

    PATH pv[MAX_PLY];

    enum move_state move_states[MAX_PLY];
    chi_move* move_stack[MAX_PLY];
    chi_move* move_ptr[MAX_PLY];
    int moves_left[MAX_PLY];
    int cached_moves[MAX_PLY];
    int in_check[MAX_PLY];

    unsigned long nodes;
    unsigned long time_for_move;

    unsigned long tt_probes;
    unsigned long tt_hits;
    unsigned long tt_moves;
    unsigned long tt_exact_hits;
    unsigned long tt_alpha_hits;
    unsigned long tt_beta_hits;

    unsigned long evals;
    unsigned long lazy_one_pawn;
    unsigned long lazy_two_pawns;

    int max_ply;
    int status;
    int iteration_depth;

    char castling_states[MAX_PLY + 1];
} TREE;

#define HASH_UNKNOWN 0
#define HASH_EXACT 1
#define HASH_ALPHA 2
#define HASH_BETA  3

extern int think PARAMS ((chi_move* mv));
extern int move_now PARAMS ((chi_move* mv));
extern void stop_thinking PARAMS ((void));

extern int evaluate PARAMS ((TREE* tree, int ply, 
			     int alpha, int beta));

extern void evaluate_move PARAMS ((chi_move move));

extern chi_move* next_move PARAMS ((TREE* tree, int ply, int depth));
#if DEBUG_BRAIN
extern void indent_output PARAMS ((TREE* tree, int depth));
#endif
extern void my_print_move PARAMS ((chi_move move));

extern void update_signature PARAMS ((TREE* tree, chi_move move, int ply));

extern int search PARAMS ((TREE* tree, int depth, int ply,
			   int alpha, int beta, int make_null));

extern int quiescence PARAMS ((TREE* tree, int ply,
			       int alpha, int beta));

extern void print_pv PARAMS ((TREE* tree, int score, 
			      int whisper, int ply));

extern void init_tt_hashs PARAMS ((unsigned long int memuse));
extern void clear_tt_hashs PARAMS ((void));

extern void store_tt_entry PARAMS ((bitv64 signature, chi_move move, int depth,
				    int score, int flags));
extern int probe_tt PARAMS ((bitv64 signature, int depth, int* alpha,
			     int* beta));
extern chi_move best_tt_move PARAMS ((bitv64 signature));

extern void update_castling_state PARAMS ((TREE* tree, chi_move move, int ply));

#endif
