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
#include <stdlib.h>
#include <string.h>

#include <error.h>

#include <libchi.h>

#include "brain.h"
#include "tate.h"
#include "time_ctrl.h"
#include "board.h"

bitv64 total_nodes = 0;
bitv64 total_centiseconds = 0;
bitv64 nps_peak = 0;

static void init_tree(TREE *tree);
static int negamax(TREE *tree, int ply, int alpha, int beta);

void
evaluate_move (chi_move move)
{
	fprintf (stdout, "  todo\n");
}

int
think(chi_move* mv, chi_epd_pos *epd)
{
    TREE tree;
	chi_move moves[CHI_MAX_MOVES];
	chi_move* move_ptr;
    
    size_t i;

	int num_moves;
    int score;
    int value;

	move_ptr = chi_legal_moves(&current, moves);

	num_moves = move_ptr - moves;

    score = -INF;

#if 0
	current_score = chi_material (&current) * 100;
	if (chi_on_move (&current) != chi_white)
	current_score = - current_score;
#endif
	fprintf (stdout, "Current score is: %d\n", current_score);
    if (num_moves == 0) {
        if (chi_check_check (&current)) {
            if (chi_on_move (&current) == chi_white) {
                fprintf (stdout, "0-1 {Black mates}\n");
            } else {
                fprintf (stdout, "1-0 {White mates}\n");
            }
        } else {
            fprintf (stdout, "1/2-1/2 {Stalemate}\n");
        }

        return EVENT_GAME_OVER;
    }

	if (current.half_move_clock >= 100) {
		fprintf (stdout, "1/2-1/2 {Fifty-move rule}\n");
		return EVENT_GAME_OVER;
	}

	*mv = moves[0];

	/* Better than nothing ...  */
	if (num_moves == 1) {
		return EVENT_CONTINUE;
	}

#if DEBUG_BRAIN
	max_ply = DEBUG_BRAIN;
	tree.time_for_move = 999999;
#endif

    init_tree(&tree);
    for (i = 0; i < num_moves; ++i) {
        move_ptr = &moves[i];
        chi_apply_move(&tree.pos, *move_ptr);
#if DEBUG_BRAIN
        indent_output(&tree, 1);
        my_print_move(*move_ptr);
        fprintf(stderr, "\n");
        fflush(stderr);
#endif
        tree.in_check[0] = chi_check_check (&tree.pos);
        tree.signatures[1] = chi_zk_update_signature(zk_handle, 
                                                     tree.signatures[0],
                                                     *move_ptr,
                                                     chi_on_move(&tree.pos));
        value = -negamax(&tree, 1, -INF, +INF);
        if (value > score) {
                mv = move_ptr;
                score = value;
        }
        chi_unapply_move(&tree.pos, *move_ptr);
    }

	return EVENT_CONTINUE;
}


// NEGAMAX
static int
negamax(TREE *tree, int ply, int alpha, int beta)
{
        chi_move moves[CHI_MAX_MOVES];
        chi_move *end = chi_legal_moves(&tree->pos, moves);
        size_t num_moves = end - moves;
        size_t i;
        int best_score;

        ++tree->nodes;

        if (ply >= max_ply || num_moves == 0) 
            return evaluate(tree, ply, alpha, beta);

        best_score = -INF;
        for (i = 0; i < num_moves; ++i) {
                chi_move *move = &moves[i];
                chi_apply_move(&tree->pos, *move);
#if DEBUG_BRAIN
        indent_output(tree, ply + 1);
        my_print_move(*move);
        fprintf(stderr, "\n");
        fflush(stderr);
#endif
                int node_score = -negamax(tree, ply + 1, -beta, -alpha);
                chi_unapply_move(&tree->pos, *move);
                if (node_score > best_score) {
                        best_score = node_score;
                }
                if (node_score > alpha) {
                    alpha = node_score;
                }
                if (alpha >= beta) {
                    break;
                }
        }
        
        return best_score;
}

#if DEBUG_BRAIN
void
indent_output (TREE* tree, int ply)
{
	int i;

//    int ply = tree->current_depth - depth;

//    for (i = depth; i < tree->current_depth; ++i)
//        fprintf (stderr, " ");

	for (i = 0; i < ply; ++i)
		fputc (' ', stderr);

	/* Assumed to be called *after* a move has been applied.  */
	if (chi_on_move(&tree->pos) != chi_white)
		fprintf(stderr, " [%s(%d)]: ", "white", ply);
	else
		fprintf(stderr, " [%s(%d)]: ", "black", ply);
}
#endif

void
my_print_move (chi_move mv)
{
	switch (chi_move_attacker (mv)) {
	case knight:
		fputc ('N', stderr);
		break;
	case bishop:
		fputc ('B', stderr);
		break;
	case rook:
		fputc ('R', stderr);
		break;
	case queen:
		fputc ('Q', stderr);
		break;
	case king:
		fputc ('K', stderr);
		break;
	}

    fprintf(stderr, "%s%c%s", 
	        chi_shift2label (chi_move_from (mv)),
	        chi_move_victim (mv) ? 'x' : '-',
	        chi_shift2label (chi_move_to (mv)));
	switch (chi_move_promote (mv)) {
	case knight:
		fprintf (stderr, "=N");
		break;
	case bishop:
	    fprintf (stderr, "=B");
	    break;
	case rook:
		fprintf (stderr, "=R");
		break;
	case queen:
		fprintf (stderr, "=Q");
		break;
	}
}

static void
init_tree(TREE *tree)
{
    memset(tree, 0, sizeof *tree);
    tree->pos = current;
    tree->in_check[0] = chi_check_check(&tree->pos);
    tree->w_castled = chi_w_castled(&tree->pos);
    tree->b_castled = chi_b_castled(&tree->pos);

    tree->signatures[0] = game_hist[game_hist_ply].signature;

    // FIXME! Initialize time_for_move.
}