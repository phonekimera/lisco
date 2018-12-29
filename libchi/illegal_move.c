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
chi_illegal_move (pos, move, ext_check)
     chi_pos* pos;
     chi_move move;
     int ext_check;
{
    chi_pos tmp_pos;
    int errnum;

    if (!pos)
	return CHI_ERR_YOUR_FAULT;

    // FIXME: This check is only necessary for history and killer
    // moves.  Generated moves and pv moves will never fail this test!
    // A possible solution would be an extra argument that triggers 
    // the extended checking.
    if (ext_check) {
	chi_move moves[CHI_MAX_MOVES];
	chi_move* mv = moves;
	chi_move* mv_ptr;
	chi_piece_t attacker = chi_move_attacker (move);
	chi_piece_t victim = chi_move_victim (move);

	switch (attacker) {
	    case pawn:
		if (victim || chi_move_promote (move)) {
		    mv = chi_generate_captures (pos, moves);
		} else {
		    mv = chi_generate_pawn_double_steps (pos, moves);
		    mv = chi_generate_pawn_single_steps (pos, mv);
		}
		break;
	    case knight:
		if (victim) {
		    mv = chi_generate_captures (pos, moves);
		} else {
		    mv = chi_generate_knight_moves (pos, moves);
		}
		break;
	    case bishop:
		if (victim) {
		    mv = chi_generate_captures (pos, moves);
		} else {
		    mv = chi_generate_bishop_moves (pos, moves);
		}
		break;
	    case rook:
		if (victim) {
		    mv = chi_generate_captures (pos, moves);
		} else {
		    mv = chi_generate_rook_moves (pos, moves);
		}
		break;
	    case queen:
		if (victim) {
		    mv = chi_generate_captures (pos, moves);
		} else {
		    mv = chi_generate_bishop_moves (pos, moves);
		    mv = chi_generate_rook_moves (pos, mv);
		}
		break;
	    case king:
		if (victim) {
		    mv = chi_generate_captures (pos, moves);
		} else {
		    mv = chi_generate_king_moves (pos, moves);
		    mv = chi_generate_king_castling_moves (pos, mv);
		}
		break;
	    default:
		return CHI_ERR_ILLEGAL_MOVE;
	}
	
	for (mv_ptr = moves; mv_ptr < mv; ++mv_ptr) {
	    if (*mv_ptr == move)
		break;
	}

	if (*mv_ptr != move)
	    return CHI_ERR_ILLEGAL_MOVE;
    }

    tmp_pos = *pos;

    if (chi_move_from (move) == 3) {
	bitv64 from_mask = ((bitv64) 1) << 3;
	int to = chi_move_to (move);
	
	if ((to == 5 || to == 1) && from_mask & pos->w_kings) {
	    int tmp_to = (3 + to) >> 1;
	    
	    if (chi_check_check (pos))
		return CHI_ERR_ILLEGAL_MOVE;
	    
	    chi_move_set_to (move, tmp_to);
	    chi_make_move (&tmp_pos, move);
	    if (chi_check_check (&tmp_pos))
		return CHI_ERR_ILLEGAL_MOVE;
	    tmp_pos = *pos;
	    chi_move_set_to (move, to);
	}
    } else if (chi_move_from (move) == 59) {
	bitv64 from_mask = ((bitv64) 1) << 59;
	int to = chi_move_to (move);
	
	if ((to == 61 || to == 57) && from_mask & pos->b_kings) {
	    int tmp_to = (3 + to) >> 1;
	    
	    if (chi_check_check (pos))
		return CHI_ERR_ILLEGAL_MOVE;
	    
	    chi_move_set_to (move, tmp_to);
	    chi_make_move (&tmp_pos, move);
	    if (chi_check_check (&tmp_pos))
		return CHI_ERR_ILLEGAL_MOVE;
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
	chi_material (pos) += chi_move_material (move);
    else
	chi_material (pos) -= chi_move_material (move);

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
