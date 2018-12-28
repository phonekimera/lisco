/* movegen_color.c - Color-specific move generators.
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

/* This is a code fragment, that gets included by movegen.c.  Do
   not use it directly!  */

#ifndef CHI_MOVEGEN_COLOR
# define CHI_MOVEGEN_COLOR 1

#define M1 ((unsigned long long) 0x5555555555555555)
#define M2 ((unsigned long long) 0x3333333333333333)

/* Give the compiler a chance to inline this.  */
static unsigned int 
find_first (b)
     bitv64 b;
{
    unsigned int n;

    bitv64 a = b - 1 - (((b - 1) >> 1) & M1);
    bitv64 c = (a & M2) + ((a >> 2) & M2);
    
    n = ((unsigned int) c) + ((unsigned int) (c >> 32));
    n = (n & 0x0f0f0f0f) + ((n >> 4) & 0x0f0f0f0f);
    n = (n & 0xffff) + (n >> 16);
    n = (n & 0xff) + (n >> 8);
    
    return n;    
}

#define chi_bitv2shift(b) find_first (b)
#endif

static chi_move* 
chi_generate_color_captures (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    chi_move* move_ptr = moves;
    bitv64 opp_squares = OPP_PIECES (pos);
    bitv64 piece_mask;
    
    /* Knight captures.  */
    piece_mask = MY_KNIGHTS (pos);
    while (piece_mask) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 attack_mask = knight_attacks[from] & opp_squares;

	while (attack_mask) {
	    unsigned int to = 
		chi_bitv2shift (chi_clear_but_least_set (attack_mask));
	    chi_move move;

	    chi_packed (move) = from;
	    chi_to (move) = to;
	    *move_ptr++ = move;
	    attack_mask = chi_clear_least_set (attack_mask);
	}
	
	piece_mask = chi_clear_least_set (piece_mask);
    }
    
    return move_ptr;
}

chi_move* 
chi_generate_color_non_captures (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    chi_move* move_ptr = moves;
    bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
    bitv64 empty_squares = ~occ_squares;

    bitv64 piece_mask = MY_PAWNS (pos);

    while (piece_mask) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 to_mask;
	
	to_mask = ((bitv64) 1) << (from + DOUBLE_PAWN_OFFSET);
	
	if (to_mask & empty_squares) {
	    chi_move move;

	    chi_packed (move) = from;
	    chi_to (move) = from + DOUBLE_PAWN_OFFSET;
	    *move_ptr++ = move;
	}
	
	to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);
	
	if (to_mask & empty_squares) {
	    chi_move move;

	    chi_packed (move) = from;
	    chi_to (move) = from + SINGLE_PAWN_OFFSET;
	    *move_ptr++ = move;
	}
	
	piece_mask = chi_clear_least_set (piece_mask);
    }

    return move_ptr;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
