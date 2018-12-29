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

static const int rotate90[64] = {
     7, 15, 23, 31, 39, 47, 55, 63,
     6, 14, 22, 30, 38, 46, 54, 62,
     5, 13, 21, 29, 37, 45, 53, 61,
     4, 12, 20, 28, 36, 44, 52, 60,
     3, 11, 19, 27, 35, 43, 51, 59,
     2, 10, 18, 26, 34, 42, 50, 58,
     1,  9, 17, 25, 33, 41, 49, 57,
     0,  8, 16, 24, 32, 40, 48, 56,
};

int
chi_make_move (pos, move)
     chi_pos* pos;
     chi_move move;
{
    int from = chi_move_from (move);
    int to = chi_move_to (move);
    bitv64 from_mask = ((bitv64) 1 << from);
    bitv64 to_mask = ((bitv64) 1 << to);
    bitv64 from90_mask = ((bitv64) 1 << rotate90[from]);
    bitv64 to90_mask = ((bitv64) 1 << rotate90[to]);
    chi_piece_t victim = chi_move_victim (move);
    chi_piece_t promote = chi_move_promote (move);
    
    ++pos->half_moves;
    ++pos->half_move_clock;
    chi_ep (pos) = 0;
    
    if (chi_on_move (pos) == chi_white) {	
	if (victim) {
	    pos->half_move_clock = 0;
	    pos->b_pieces &= ~to_mask;
	    pos->b_pieces90 &= ~to90_mask;

	    switch (victim) {
		case pawn:
		    if (chi_move_is_ep (move)) {
			int ep_target = to - 8;
			bitv64 ep_mask = ((bitv64) 1) << ep_target;
			bitv64 ep_mask90 = ((bitv64) 1) << 
			    rotate90[ep_target];
			
			pos->b_pieces &= ~ep_mask;
			pos->b_pawns &= ~ep_mask;
			pos->b_pieces90 &= ~ep_mask90;
		    } else {
			pos->b_pawns &= ~to_mask;
		    }
		    break;
		case knight:
		    pos->b_knights &= ~to_mask;
		    break;
		case bishop:
		    pos->b_bishops &= ~to_mask;
		    break;
		case rook:
		    pos->b_rooks &= ~to_mask;
		    if (to_mask == (CHI_A_MASK & CHI_8_MASK))
			chi_bq_castle (pos) = 0;
		    else if (to_mask == (CHI_H_MASK & CHI_8_MASK))
			chi_bk_castle (pos) = 0;
		    break;
		default:  /* Queen.  */
		    pos->b_rooks &= ~to_mask;
		    pos->b_bishops &= ~to_mask;
		    break;
	    }
	}

	pos->w_pieces &= ~from_mask;
	pos->w_pieces |= to_mask;
	pos->w_pieces90 &= ~from90_mask;
	pos->w_pieces90 |= to90_mask;

	switch (chi_move_attacker (move)) {
	    case pawn:
		pos->half_move_clock = 0;
		pos->w_pawns &= ~from_mask;
		switch (promote) {
		    case knight:
			pos->w_knights |= to_mask;
			break;
		    case bishop:
			pos->w_bishops |= to_mask;
			break;
		    case rook:
			pos->w_rooks |= to_mask;
			break;
		    case queen:
			pos->w_rooks |= to_mask;
			pos->w_bishops |= to_mask;
			break;
		    default:
			pos->w_pawns |= to_mask;
			if (to - from == 16) {
			    chi_ep (pos) = 1;
			    chi_ep_file (pos) = 7 - (from % 8);
			}
			break;
		}
		break;
	    case knight:
		pos->w_knights &= ~from_mask;
		pos->w_knights |= to_mask;
		break;
	    case bishop:
		pos->w_bishops &= ~from_mask;
		pos->w_bishops |= to_mask;
		break;
	    case rook:
		pos->w_rooks &= ~from_mask;
		pos->w_rooks |= to_mask;
		if (from_mask == (CHI_A_MASK & CHI_1_MASK))
		    chi_wq_castle (pos) = 0;
		else if (from_mask == (CHI_H_MASK & CHI_1_MASK))
		    chi_wk_castle (pos) = 0;
		break;
	    case queen:
		pos->w_bishops &= ~from_mask;
		pos->w_bishops |= to_mask;
		pos->w_rooks &= ~from_mask;
		pos->w_rooks |= to_mask;
		break;
	    case king:
		pos->w_kings &= ~from_mask;
		pos->w_kings |= to_mask;
		if (from_mask == (CHI_E_MASK & CHI_1_MASK)) {
		    chi_wq_castle (pos) = 0;
		    chi_wk_castle (pos) = 0;
		    if (to_mask == (CHI_G_MASK & CHI_1_MASK)) {
			bitv64 rook_from_mask = ((bitv64) 1) << (to - 1);
			bitv64 rook_to_mask = ((bitv64) 1) << (to + 1);
			bitv64 rook_from90_mask = ((bitv64) 1) <<
			    rotate90[to - 1];
			bitv64 rook_to90_mask = ((bitv64) 1) <<
			    rotate90[to + 1];

			chi_w_castled (pos) = 1;
			pos->w_rooks |= rook_to_mask;
			pos->w_rooks &= ~rook_from_mask;
			pos->w_pieces |= rook_to_mask;
			pos->w_pieces &= ~rook_from_mask;
			pos->w_pieces90 |= rook_to90_mask;
			pos->w_pieces90 &= ~rook_from90_mask;
		    } else if (to_mask == (CHI_C_MASK & CHI_1_MASK)) {
			bitv64 rook_from_mask = ((bitv64) 1) << (to + 2);
			bitv64 rook_to_mask = ((bitv64) 1) << (to - 1);
			bitv64 rook_from90_mask = ((bitv64) 1) <<
			    rotate90[to + 2];
			bitv64 rook_to90_mask = ((bitv64) 1) <<
			    rotate90[to - 1];
			
			chi_w_castled (pos) = 1;
			pos->w_rooks |= rook_to_mask;
			pos->w_rooks &= ~rook_from_mask;
			pos->w_pieces |= rook_to_mask;
			pos->w_pieces &= ~rook_from_mask;
			pos->w_pieces90 |= rook_to90_mask;
			pos->w_pieces90 &= ~rook_from90_mask;
		    }
		}
		break;
	}
    } else {
	if (victim) {
	    pos->half_move_clock = 0;
	    pos->w_pieces &= ~to_mask;
	    pos->w_pieces90 &= ~to90_mask;

	    switch (victim) {
		case pawn:
		    if (chi_move_is_ep (move)) {
			int ep_target = to + 8;
			bitv64 ep_mask = ((bitv64) 1) << ep_target;
			bitv64 ep_mask90 = ((bitv64) 1) << 
			    rotate90[ep_target];
			
			pos->w_pieces &= ~ep_mask;
			pos->w_pawns &= ~ep_mask;
			pos->w_pieces90 &= ~ep_mask90;
		    } else {
			pos->w_pawns &= ~to_mask;
		    }
		    break;
		case knight:
		    pos->w_knights &= ~to_mask;
		    break;
		case bishop:
		    pos->w_bishops &= ~to_mask;
		    break;
		case rook:
		    pos->w_rooks &= ~to_mask;
		    if (to_mask == (CHI_A_MASK & CHI_1_MASK))
			chi_wq_castle (pos) = 0;
		    else if (to_mask == (CHI_H_MASK & CHI_1_MASK))
			chi_wk_castle (pos) = 0;
		    break;
		default: /* Queen.  */
		    pos->w_rooks &= ~to_mask;
		    pos->w_bishops &= ~to_mask;
		    break;
	    }
	}

	pos->b_pieces &= ~from_mask;
	pos->b_pieces |= to_mask;
	pos->b_pieces90 &= ~from90_mask;
	pos->b_pieces90 |= to90_mask;

	switch (chi_move_attacker (move)) {
	    case pawn:
		pos->half_move_clock = 0;
		pos->b_pawns &= ~from_mask;
		switch (promote) {
		    case knight:
			pos->b_knights |= to_mask;
			break;
		    case bishop:
			pos->b_bishops |= to_mask;
			break;
		    case rook:
			pos->b_rooks |= to_mask;
			break;
		    case queen:
			pos->b_rooks |= to_mask;
			pos->b_bishops |= to_mask;
			break;
		    default:
			pos->b_pawns |= to_mask;
			if (from - to == 16) {
			    chi_ep (pos) = 1;
			    chi_ep_file (pos) = 7 - (from % 8);
			}
			break;
		}
		break;
	    case knight:
		pos->b_knights &= ~from_mask;
		pos->b_knights |= to_mask;
		break;
	    case bishop:
		pos->b_bishops &= ~from_mask;
		pos->b_bishops |= to_mask;
		break;
	    case rook:
		pos->b_rooks &= ~from_mask;
		pos->b_rooks |= to_mask;
		if (from_mask == (CHI_A_MASK & CHI_8_MASK))
		    chi_bq_castle (pos) = 0;
		else if (from_mask == (CHI_H_MASK & CHI_8_MASK))
		    chi_bk_castle (pos) = 0;
		break;
	    case queen:
		pos->b_bishops &= ~from_mask;
		pos->b_bishops |= to_mask;
		pos->b_rooks &= ~from_mask;
		pos->b_rooks |= to_mask;
		break;
	    case king:
		pos->b_kings &= ~from_mask;
		pos->b_kings |= to_mask;
		if (from_mask == (CHI_E_MASK & CHI_8_MASK)) {
		    chi_bq_castle (pos) = 0;
		    chi_bk_castle (pos) = 0;

		    if (to_mask == (CHI_G_MASK & CHI_8_MASK)) {
			bitv64 rook_from_mask = ((bitv64) 1) << (to - 1);
			bitv64 rook_to_mask = ((bitv64) 1) << (to + 1);
			bitv64 rook_from90_mask = ((bitv64) 1) <<
			    rotate90[to - 1];
			bitv64 rook_to90_mask = ((bitv64) 1) <<
			    rotate90[to + 1];
			
			chi_w_castled (pos) = 1;
			pos->b_rooks |= rook_to_mask;
			pos->b_rooks &= ~rook_from_mask;
			pos->b_pieces |= rook_to_mask;
			pos->b_pieces &= ~rook_from_mask;
			pos->b_pieces90 |= rook_to90_mask;
			pos->b_pieces90 &= ~rook_from90_mask;
		    } else if (to_mask == (CHI_C_MASK & CHI_8_MASK)) {
			bitv64 rook_from_mask = ((bitv64) 1) << (to + 2);
			bitv64 rook_to_mask = ((bitv64) 1) << (to - 1);
			bitv64 rook_from90_mask = ((bitv64) 1) <<
			    rotate90[to + 2];
			bitv64 rook_to90_mask = ((bitv64) 1) <<
			    rotate90[to - 1];
			
			chi_b_castled (pos) = 1;
			pos->b_rooks |= rook_to_mask;
			pos->b_rooks &= ~rook_from_mask;
			pos->b_pieces |= rook_to_mask;
			pos->b_pieces &= ~rook_from_mask;
			pos->b_pieces90 |= rook_to90_mask;
			pos->b_pieces90 &= ~rook_from90_mask;
		    }
		}
		break;
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
