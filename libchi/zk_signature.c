/* zk_signature.c - Create a zobrist key for a chess position.
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

#include <stdlib.h>

#include <libchi.h>

#define ZK_ARRAY_SIZE (((king + 1) * 2 * 64) + 1)

bitv64
chi_zk_signature (zk_handle, pos)
     chi_zk_handle zk_handle;
     chi_pos* pos;
{
    bitv64 sig = (bitv64) 0;
    bitv64 piece_mask;

    piece_mask = pos->w_pawns;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, pawn, chi_white, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->b_pawns;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, pawn, chi_black, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->w_knights;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, knight, chi_white, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->b_knights;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, knight, chi_black, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->w_bishops & ~pos->w_rooks;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, bishop, chi_white, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->b_bishops & ~pos->b_rooks;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, bishop, chi_black, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->w_rooks & ~pos->w_bishops;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, rook, chi_white, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->b_rooks & ~pos->b_bishops;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, rook, chi_black, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->w_bishops & pos->w_rooks;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, queen, chi_white, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->b_bishops & pos->b_rooks;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, queen, chi_black, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->w_kings;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, king, chi_white, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    piece_mask = pos->b_kings;
    while (piece_mask) {
	unsigned int shift = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	
	sig ^= chi_zk_lookup (zk_handle, king, chi_black, shift);

	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    if (chi_on_move (pos) != chi_white)
	sig ^= zk_handle[ZK_ARRAY_SIZE - 1];

    return sig;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
