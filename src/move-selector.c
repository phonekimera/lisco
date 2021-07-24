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

#include <sys/types.h>

#include <libchi.h>

#include "lisco.h"

void
move_selector_init(MoveSelector *self, const Tree *tree, chi_move bestmove)
{
	chi_move *move_ptr = chi_legal_moves(&tree->position, self->moves);
	self->num_moves = move_ptr - self->moves;
	self->selected = 0;
	chi_move *sorted = self->moves;
	size_t size = self->num_moves;

	if (bestmove) {
		/* Move the best move to the front and take the opportunity to already
		 * sort the array a little for the following insertion sort.
		 */
		for (size_t i = 0; i < size; ++i) {
			if (bestmove == self->moves[i]) {
				++sorted;
				--size;
				self->moves[i] = self->moves[0];
				self->moves[0] = bestmove;
				break;
			}
			if (i && self->moves[i] > self->moves[i - 1]) {
				chi_move tmp_move = self->moves[i - 1];
				self->moves[i - 1] = self->moves[i];
				self->moves[i] = tmp_move;
			}
		}
	}

	/* FIXME! It probably doesn't make sense to sort the entire array but
	 * only those moves with a material gain or a promotion.
	 */
	for (size_t step = 1; step < size; ++step) {
		chi_move key = sorted[step];
		int j = step - 1;

		while (key > sorted[j] && j >= 0) {
			sorted[j + 1] = sorted[j];
			--j;
		}
		sorted[j + 1] = key;
	}
}

#include <stdio.h>
void
move_selector_quiescence_init(MoveSelector *self, const Tree *tree)
{
	chi_move *move_ptr = chi_legal_moves(&tree->position, self->moves);
	size_t size = move_ptr - self->moves;
	self->selected = 0;
	chi_move *sorted = self->moves;
	size_t num_moves = 0;
	const chi_pos *position = &tree->position;

	/* First prune all non-captures, non-promotions and bad captures.  */
	for (int i = 0; i < size; ++i) {
		chi_move move = sorted[i];
		bitv64 see;
		if ((chi_move_victim(move) || chi_move_promote(move))
		    && (see = chi_see(position, move) > 0)) {
			sorted[num_moves++] = move | see << 32;
		}
	}
	self->num_moves = num_moves;

	for (size_t step = 1; step < size; ++step) {
		chi_move key = sorted[step];
		int j = step - 1;

		while (key > sorted[j] && j >= 0) {
			sorted[j + 1] = sorted[j];
			--j;
		}
		sorted[j + 1] = key;
	}
}

chi_move
move_selector_next(MoveSelector *self)
{
	/* FIXME! Trade memory for performance and make the array one item bigger
	 * and insert a 0 at the end. Provided that the caller stops pulling moves
	 * on a null move, this is safe.
	 */
	if (self->selected >= self->num_moves)
		return 0;

	return self->moves[self->selected++];
}