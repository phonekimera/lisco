/* make_move.c - Apply a move to a position.
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

int
chi_make_move (pos, move)
     chi_pos* pos;
     chi_move move;
{
        int from = move.fields.from;
	int to = move.fields.to;
	int to_move = chi_to_move (pos);
	bitv64 from_mask = ((bitv64) 1 << from);
	bitv64 to_mask = ((bitv64) 1 << to);
#if FIXME
	int ep = chi_ep (pos);
	int ep_file = chi_ep_file (pos);
#endif

	++pos->half_moves;
	++pos->half_move_clock;
	chi_ep (pos) = 0;

	if (to_move == chi_white) {
		int is_capture;
		if (to_mask & pos->b_pieces) {
			is_capture = 1;
			pos->b_pieces &= ~to_mask;
			pos->half_move_clock = 0;

			if (to == chi_coords2shift (CHI_FILE_A, 
							CHI_RANK_8))
				chi_bq_castle (pos) = 0;
			else if (to == chi_coords2shift (CHI_FILE_H,
							     CHI_RANK_8))
				chi_bk_castle (pos) = 0;
			
		} else {
			is_capture = 0;
		}

		pos->w_pieces &= ~from_mask;
		pos->w_pieces |= to_mask;

		if (from_mask & pos->w_pawns) {
			pos->half_move_clock = 0;

			pos->w_pawns |= to_mask;
			pos->w_pawns &= ~from_mask;

			/* Update en passant status.  */
			if (from - to == -16) {
				chi_ep (pos) = 1;
				chi_ep_file (pos) = (from + 1) % 8;
			} else {
				chi_ep (pos) = 0;
			}
		} else if (from_mask & pos->w_knights) {
			pos->w_knights |= to_mask;
			pos->w_knights &= ~from_mask;
		} else if (from_mask & pos->w_bishops) {
			pos->w_bishops |= to_mask;
			pos->w_bishops &= ~from_mask;
			if (from_mask & pos->w_rooks) {
				pos->w_rooks |= to_mask;
				pos->w_rooks &= ~from_mask;
			}
		} else if (from_mask & pos->w_rooks) {
			pos->w_rooks |= to_mask;
			pos->w_rooks &= ~from_mask;
			if (from_mask == (CHI_A_MASK & CHI_1_MASK))
				chi_wq_castle (pos) = 0;
			else if (from_mask == (CHI_H_MASK & CHI_1_MASK))
				chi_wk_castle (pos) = 0;
		} else if (from_mask & pos->w_kings) {
			printf ("King move!\n");
			chi_wk_castle (pos) 
				= chi_wq_castle (pos) = 0;
			pos->w_kings |= to_mask;
			pos->w_kings &= ~from_mask;
		}

	} else {
		int is_capture;

		if (to_mask & pos->w_pieces) {
			is_capture = 1;
			pos->w_pieces &= ~to_mask;
			pos->half_move_clock = 0;

			if (to == chi_coords2shift (CHI_FILE_A, 
							CHI_RANK_1))
				chi_wq_castle (pos) = 0;
			else if (to == chi_coords2shift (CHI_FILE_H,
							     CHI_RANK_1))
				chi_wk_castle (pos) = 0;

		} else {
			is_capture = 0;
		}

		pos->b_pieces &= ~from_mask;
		pos->b_pieces |= to_mask;

		if (from_mask & pos->b_pawns) {
			pos->half_move_clock = 0;

			pos->b_pawns |= to_mask;
			pos->b_pawns &= ~from_mask;

			/* Update en passant status.  */
			if (from - to == 16) {
				chi_ep (pos) = 1;
				chi_ep_file (pos) = (from + 1) % 8;
			} else {
				chi_ep (pos) = 0;
			}
		} else if (from_mask & pos->b_knights) {
			pos->b_knights |= to_mask;
			pos->b_knights &= ~from_mask;
		} else if (from_mask & pos->b_bishops) {
			pos->b_bishops |= to_mask;
			pos->b_bishops &= ~from_mask;
			if (from_mask & pos->b_rooks) {
				pos->b_rooks |= to_mask;
				pos->b_rooks &= ~from_mask;
			}
		} else if (from_mask & pos->b_rooks) {
			pos->b_rooks |= to_mask;
			pos->b_rooks &= ~from_mask;
			if (from_mask == (CHI_A_MASK & CHI_8_MASK))
				chi_bq_castle (pos) = 0;
			else if (from_mask == (CHI_H_MASK & CHI_8_MASK))
				chi_bk_castle (pos) = 0;
		} else if (from_mask & pos->b_kings) {
			chi_bk_castle (pos) 
				= chi_bq_castle (pos) = 0;
			pos->b_kings |= to_mask;
			pos->b_kings &= ~from_mask;
		}		
	}

	return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
