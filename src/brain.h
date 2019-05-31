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

#ifndef BRAIN_H
# define BRAIN_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#include "tate.h"

// Set to the maximum depth to search.
#define DEBUG_BRAIN 3
#if DEBUG_BRAIN
# include <stdio.h>
#endif

#ifndef MOVES_PER_TIME_CONTROL
# define MOVES_PER_TIME_CONTROL 0x4000
#endif

#define ONEPLY (64)
#define TWOPLY (ONEPLY << 1)
#define DEPTH2PLIES(d) ((d) >> 6)
#define PLIES2DEPTH(p) ((p) << 6)

#define CHECK_EXT ONEPLY
#define MATE_EXT  (48)

#define FRONTIER_DEPTH PLIES2DEPTH(1)
#define PRE_FRONTIER_DEPTH PLIES2DEPTH(2)
#define PRE_PRE_FRONTIER_DEPTH PLIES2DEPTH(3)

#define ASPIRATION_WINDOW 1

#if 0
#define MOBILITY_SHIFT 1
#define MIN_EVAL_DIFF (1 << MOBILITY_SHIFT)
#endif

#define MAX_POS_SCORE 99999

#define FUTILITY_MARGIN 99999
#define EXT_FUTILITY_MARGIN 99999
#define RAZOR_MARGIN 99999

extern bitv64 total_nodes;
extern bitv64 total_centiseconds;
extern int next_time_control;

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

    move_state_bonny = move_state_captures + 1,
#define move_state_bonny (move_state_captures + 1)

    move_state_clyde = move_state_bonny + 1,
#define move_state_clyde (move_state_bonny + 1)

    move_state_generate_non_captures = move_state_clyde + 1,
#define move_state_generate_non_captures (move_state_clyde + 1)

    move_state_non_captures = move_state_generate_non_captures + 1,
#define move_state_non_captures (move_state_generate_non_captures + 1)
};

typedef struct tree {
    bitv64 signatures[MAX_PLY + 1];

    chi_pos pos;

    int in_check[MAX_PLY];

    /* Initial castling state.  */
    chi_bool w_castled;
    chi_bool b_castled;

    unsigned long nodes;
    // unsigned long qnodes;
    unsigned long time_for_move;

    // unsigned long tt_probes;
    // unsigned long tt_hits;
    // unsigned long tt_moves;
    // unsigned long tt_exact_hits;
    // unsigned long tt_alpha_hits;
    // unsigned long tt_beta_hits;

    // unsigned long qtt_probes;
    // unsigned long qtt_hits;
    // unsigned long qtt_exact_hits;
    // unsigned long qtt_alpha_hits;
    // unsigned long qtt_beta_hits;

    // unsigned long killers;

    // unsigned long null_moves;
    // unsigned long null_fh;
    // unsigned long fh;
    // unsigned long ffh;
    // unsigned long fl;
    // unsigned long qfh;
    // unsigned long qffh;
    // unsigned long qfl;

    // unsigned long fprunes;
    // unsigned long refuted_quiescences;

    unsigned long evals;
    unsigned long ev_hits;
    unsigned long lazy_evals;

    // int deepest_search;
    int status;
    // int iteration_sdepth;
    // int pv_printed;
    // int pv_junk;

    chi_epd_pos* epd;
} TREE;

#define HASH_UNKNOWN ((unsigned int) 0)
#define HASH_ALPHA   ((unsigned int) 1)
#define HASH_BETA    ((unsigned int) 2)
#define HASH_EXACT   ((unsigned int) 3)

extern unsigned int history[];
#define history_lookup(tree, move) \
    history[(move & 0xfff) + (chi_on_move (&tree->pos) << 6)]

extern int think(chi_move* mv, chi_epd_pos* epd);
extern int move_now(chi_move* mv);
extern void stop_thinking(void);

extern int evaluate(TREE* tree, int ply, int alpha, int beta);
extern int evaluate_dev_white(TREE* tree, int ply);
extern int evaluate_dev_black(TREE* tree, int ply);
extern int evaluate_mobility(TREE* tree);

extern void evaluate_move(chi_move move);

extern chi_move next_move(TREE* tree, int ply, int depth);
#if DEBUG_BRAIN
extern void indent_output(TREE* tree, int depth);
#endif
extern void my_print_move(chi_move move);

extern void update_signature(TREE* tree, chi_move move, int ply);

extern void print_current_move(TREE* tree, chi_pos* pos,
					           chi_move move, int count, 
					           int num_moves, int alpha, int beta);
					  
extern void clean_current_move(TREE* tree);

extern void print_fail_high(TREE* tree, int score, int whisper);
extern void print_fail_low(TREE* tree, int score, int whisper);

extern void init_tt_hashs(unsigned long int memuse);
extern void clear_tt_hashs(void);

extern void store_tt_entry(chi_pos* pos,
				           bitv64 signature, chi_move move, int depth,
				           int score, unsigned int flags);
extern unsigned int probe_tt(chi_pos* pos, bitv64 signature, 
				             int depth, int* alpha,
				             int* beta);
extern chi_move best_tt_move(chi_pos* pos, bitv64 signature);

extern void init_qtt_hashs(unsigned long int memuse);
extern void clear_qtt_hashs(void);

extern void store_qtt_entry(chi_pos* pos,
				            bitv64 signature,
				            int score, unsigned int flags);
extern unsigned int probe_qtt(chi_pos* pos, bitv64 signature, int* alpha,
                              int* beta);

extern void init_ev_hashs(unsigned long int memuse);
extern void clear_ev_hashs(void);
extern void store_ev_entry(chi_pos* pos, bitv64 signature, int score);
extern int probe_ev(chi_pos* pos, bitv64 signature, int* score);

extern void store_killer(TREE* tree, chi_move move, int depth, int ply);
extern void init_killers(void);

#if DEBUG_BRAIN
extern void dump_pv(TREE *tree);
#endif

#endif
