/* legal_move.c - Check one single move for legality.
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
chi_illegal_move (pos, move)
     chi_pos* pos;
     chi_move move;
{
    chi_pos tmp_pos;
    int errnum;

    if (!pos)
	return CHI_ERR_YOUR_FAULT;

    // FIXME: This check is only necessary for history and killer
    // moves.  Generated moves and pv moves will never fail this test!
    // A possible solution would be an extra argument that triggers 
    // the extended checking.
    if (chi_on_move (pos) == chi_white) {
	bitv64 from_mask = ((bitv64) 1) << chi_move_from (move);
	chi_piece_t attacker = chi_move_attacker (move);
	bitv64 to_mask = ((bitv64) 1) << chi_move_to (move);
	chi_piece_t victim = chi_move_victim (move);

	if (!(pos->w_pieces & from_mask))
	    return CHI_ERR_EMPTY_SQUARE;
	else {
	    switch (attacker) {
		case pawn:
		    if (!(pos->w_pawns & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case knight:
		    if (!(pos->w_knights & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case bishop:
		    if (!(pos->w_bishops & ~pos->w_rooks & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case rook:
		    if (!(pos->w_rooks & ~pos->w_bishops & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case queen:
		    if (!(pos->w_rooks & pos->w_bishops & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case king:
		    if (!(pos->w_kings & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		default:
		    return CHI_ERR_ILLEGAL_MOVE;
		    break;
	    }
	}

	if (victim) {
	    switch (victim) {
		case pawn:
		    if (!(pos->b_pawns & to_mask)) {
			if (chi_move_is_ep (move)) {
			    int ep_file = 7 - (chi_move_from (move) % 8);
			    
			    if (!(chi_ep (pos)) ||
				chi_ep_file (pos) != ep_file)
				return CHI_ERR_ILLEGAL_MOVE;

			    to_mask >>= 8;
			    
			    if (!(pos->b_pawns & to_mask))
				return CHI_ERR_ILLEGAL_MOVE;
			}
			return CHI_ERR_ILLEGAL_MOVE;
		    }
		    break;
		case knight:
		    if (!(pos->b_knights & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case bishop:
		    if (!(pos->b_bishops & ~pos->b_rooks & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case rook:
		    if (!(pos->b_rooks & ~pos->b_bishops & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case queen:
		    if (!(pos->b_rooks & pos->b_bishops & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		default:
		    return CHI_ERR_ILLEGAL_MOVE;
		    break;
	    }
	} else if ((pos->w_pieces & to_mask) || (pos->b_kings & to_mask)){
	    return CHI_ERR_ILLEGAL_MOVE;
	}
    } else {
	bitv64 from_mask = ((bitv64) 1) << chi_move_from (move);
	chi_piece_t attacker = chi_move_attacker (move);
	bitv64 to_mask = ((bitv64) 1) << chi_move_to (move);
	chi_piece_t victim = chi_move_victim (move);

	if (!(pos->b_pieces & from_mask))
	    return CHI_ERR_EMPTY_SQUARE;
	else {
	    switch (attacker) {
		case pawn:
		    if (!(pos->b_pawns & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case knight:
		    if (!(pos->b_knights & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case bishop:
		    if (!(pos->b_bishops & ~pos->b_rooks & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case rook:
		    if (!(pos->b_rooks & ~pos->b_bishops & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case queen:
		    if (!(pos->b_rooks & pos->b_bishops & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case king:
		    if (!(pos->b_kings & from_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		default:
		    return CHI_ERR_ILLEGAL_MOVE;
		    break;
	    }
	}

	if (victim) {
	    switch (victim) {
		case pawn:
		    if (!(pos->w_pawns & to_mask)) {
			if (chi_move_is_ep (move)) {
			    int ep_file = 7 - (chi_move_from (move) % 8);
			    
			    if (!(chi_ep (pos)) ||
				chi_ep_file (pos) != ep_file)
				return CHI_ERR_ILLEGAL_MOVE;

			    to_mask <<= 8;
			    
			    if (!(pos->w_pawns & to_mask))
				return CHI_ERR_ILLEGAL_MOVE;
			}
			return CHI_ERR_ILLEGAL_MOVE;
		    }
		    break;
		case knight:
		    if (!(pos->w_knights & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case bishop:
		    if (!(pos->w_bishops & ~pos->w_rooks & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case rook:
		    if (!(pos->w_rooks & ~pos->w_bishops & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		case queen:
		    if (!(pos->w_rooks & pos->w_bishops & to_mask))
			return CHI_ERR_ILLEGAL_MOVE;
		    break;
		default:
		    return CHI_ERR_ILLEGAL_MOVE;
		    break;
	    }
	} else if ((pos->b_pieces & to_mask) || (pos->w_kings & to_mask)) {
	    return CHI_ERR_ILLEGAL_MOVE;
	}
    }

    tmp_pos = *pos;

    if (chi_move_from (move) == 3) {
	bitv64 from_mask = ((bitv64) 1) << 3;
	int to = chi_move_to (move);
	
	if ((to == 5 || to == 1) && from_mask & pos->w_kings) {
	    int tmp_to = (3 + to) >> 1;
	    
	    if (chi_check_check (pos))
		return 1;
	    
	    chi_move_set_to (move, tmp_to);
	    chi_make_move (&tmp_pos, move);
	    if (chi_check_check (&tmp_pos))
		return 1;
	    tmp_pos = *pos;
	    chi_move_set_to (move, to);
	}
    } else if (chi_move_from (move) == 59) {
	bitv64 from_mask = ((bitv64) 1) << 59;
	int to = chi_move_to (move);
	
	if ((to == 61 || to == 57) && from_mask & pos->b_kings) {
	    int tmp_to = (3 + to) >> 1;
	    
	    if (chi_check_check (pos))
		return 1;
	    
	    chi_move_set_to (move, tmp_to);
	    chi_make_move (&tmp_pos, move);
	    if (chi_check_check (&tmp_pos))
		return 1;
	    tmp_pos = *pos;
	    chi_move_set_to (move, to);
	}
    }
    
    if (0 != (errnum = chi_make_move (&tmp_pos, move)))
	return errnum;
    
    if (chi_check_check (&tmp_pos))
	return CHI_ERR_IN_CHECK;
    
    *pos = tmp_pos;

    if (chi_on_move (pos) == chi_white)
	pos->material += chi_move_material (move);
    else
	pos->material -= chi_move_material (move);

    chi_on_move (pos) = !chi_on_move (pos);

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
