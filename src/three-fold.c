/* search.c - negamax search.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#include "three-fold.h"
#include "tate.h"
#include "brain.h"

int
three_fold_repetition(TREE *tree, int ply, int* game_hist_hash)
{
	chi_pos* pos = &tree->pos;
	bitv64 this_signature = tree->signatures[ply];
	int hist_hash_key = this_signature % HASH_HIST_SIZE;

	/* Maybe a draw by 3-fold repetition.  */
	if (ply > 1 && pos->half_move_clock >= 8 &&
	    game_hist_hash[hist_hash_key] > 1) {
		
		int seen = 0;
		int i;
	
#if DEBUG_BRAIN
		fprintf (stderr, "searching for 3-fold repetition at ply %d (%016llx).\n", ply, this_signature);
#endif

		for (i = ply % 2; ((i <= game_hist_ply)
		     && (pos->half_move_clock - i - ply >= 0)); i += 2) {

#if DEBUG_BRAIN
			fprintf(stderr, "checking with i = %d (%016llx<%016llx>%016llx\n", 
			        i, 
			        game_hist[game_hist_ply - i - 1].signature,
			        game_hist[game_hist_ply - i].signature,
			        game_hist[game_hist_ply - i + 1].signature
			        );
#endif

			if (game_hist[game_hist_ply - i].signature == this_signature) {
				++seen;
#if DEBUG_BRAIN
				fprintf (stderr, "Hit #%d\n", seen);
#endif
				if (seen >= 2) {
#if DEBUG_BRAIN
					indent_output (tree, ply);
					fprintf(stderr, "Draw: %d (3-fold)\n", DRAW);
#endif

					return 1;
				}
				i += 2;
			}
		}
	}

	return 0;
}