/* This file is part of the chess engine lisco.
 *
 * Copyright (C) 2002-2021 cantanea EOOD.
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

#include <string.h>

#include <libchi.h>

#include "lisco.h"

static void update_tree(Tree *tree, int ply, chi_pos *position, chi_move move);

int
quiesce(Tree *tree, int ply, int alpha, int beta)
{
	int value = evaluate(tree, ply, alpha, beta);

	if (value >= beta) {
		return beta;
	}
	if (value > alpha) {
		alpha = value;
	}

	chi_pos *position = &tree->position;

	// FIXME! The move selector should only generate captures and promotions.
	MoveSelector selector;
	move_selector_quiescence_init(&selector, tree);

	chi_move move;
	while ((move = move_selector_next(&selector))) {
		if (tree->move_now) {
			return alpha;
		}

		chi_apply_move(position, move);
		update_tree(tree, ply, position, move);

		value = -quiesce(tree, ply + 1, -beta, -alpha);

		/*
		store_qtt_entry(position, tree->signatures[ply + 1], move, depth, value,
				HASH_EXACT);
		*/

		chi_unapply_move(position, move);
	}

	return alpha;
}

static void
update_tree(Tree *tree, int ply, chi_pos *position, chi_move move)
{
	tree->signatures[ply + 1] = chi_zk_update_signature(lisco.zk_handle,
		tree->signatures[ply], move, chi_on_move(position));
}