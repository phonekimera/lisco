/* This file is part of the chess engine tate.
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

/* This is a code fragment, that gets included by movegen.c.  Do
   not use it directly!  */

#ifndef CHI_MOVEGEN_COLOR
# define CHI_MOVEGEN_COLOR 1

#define M1 ((bitv64) 0x5555555555555555)
#define M2 ((bitv64) 0x3333333333333333)

#define DIAG045 ((bitv64) 0x8040201008040201)
#define DIAG235 ((bitv64) 0x0102040810204080)
#define FILE_UP_MASK ((bitv64) 0x0101010101010100)
#define FILE_DN_MASK ((bitv64) 0x0080808080808080)
#define RANKMASK ((bitv64) 0xff)

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

#endif

chi_move* 
chi_generate_color_captures (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 her_squares = HER_PIECES (pos);
    bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
    bitv64 empty_squares = ~occ_squares;
    bitv64 occ90_squares = pos->w_pieces90 | pos->b_pieces90;

    bitv64 piece_mask = MY_PAWNS (pos);
 
    /* Pawn captures.  */
    if (piece_mask) {
	bitv64 target_squares = her_squares;
	int ep_offset = 0;

	if (chi_ep (pos)) {
	    ep_offset = chi_coords2shift (chi_ep_file (pos), EP_RANK);
	    target_squares |= ((bitv64) 1) << ep_offset;
	}

	while (piece_mask) {
	    int from =
		chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 from_mask = ((bitv64) 1) << from;
	    bitv64 to_mask = 0;

	    /* Pawn promotions.  */
	    if (from_mask & PAWN_PRE_PROMOTE_RANK_MASK) {
		bitv64 to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);
		
		if (to_mask & empty_squares) {
		    chi_move move = from | 
			((from + SINGLE_PAWN_OFFSET) << 6) |
			((~pawn & 0x7) << 13);
		    
		    *moves++ = move | (queen << 19) | 
			(8 << 22);
		    *moves++ = move | (rook << 19) | 
			(4 << 22);
		    *moves++ = move | (bishop << 19) | 
			(2 << 22);
		    *moves++ = move | (knight << 19) | 
			(2 << 22);
		}	
	    }

	    if (from_mask & ~CHI_A_MASK) {
		to_mask = ((bitv64) 1) << (from + LEFT_PAWN_CAPTURE_OFFSET);

		if (to_mask & target_squares) {
		    bitv64 target_square = to_mask & target_squares;
		    int to = from + LEFT_PAWN_CAPTURE_OFFSET;
		    chi_move move = from | (to << 6) | ((~pawn & 0x7) << 13);
		    int material = 1 << 22;
		    chi_piece_t victim = pawn << 16;
		    int ep_flag = 0;
		    chi_move filled;

		    if (target_square & HER_KNIGHTS (pos)) {
			material = 3 << 22;
			victim = knight << 16;
		    } else if (target_square & HER_BISHOPS (pos)) {
			material = 3 << 22;
			victim = bishop << 16;
			if (target_square & HER_ROOKS (pos)) {
			    material = 9 << 22;
			    victim = queen << 16;
			}
		    } else if (target_square & HER_ROOKS (pos)) {
			material = 5 << 22;
			victim = rook << 16;
		    } else if (to == ep_offset) {
			ep_flag = 0x1000;
		    }

		    filled = move | victim | ep_flag | material;

		    if (to_mask & PAWN_PROMOTE_RANK_MASK) {
			*moves++ = filled | (queen << 19) | 
			    (8 << 22);
			*moves++ = filled | (rook << 19) | 
			    (4 << 22);
			*moves++ = filled | (bishop << 19) | 
			    (2 << 22);
			*moves++ = filled | (knight << 19) | 
			    (2 << 22);
		    } else {
			*moves++ = filled;
		    }
		}
	    }
	    
	    if (from_mask & ~CHI_H_MASK) {
		to_mask = ((bitv64) 1) << (from + RIGHT_PAWN_CAPTURE_OFFSET);

		if (to_mask & target_squares) {
		    bitv64 target_square = to_mask & target_squares;
		    int to = from + RIGHT_PAWN_CAPTURE_OFFSET;
		    chi_move move = from | (to << 6) | ((~pawn & 0x7) << 13);
		    int material = 1 << 22;
		    chi_piece_t victim = pawn << 16;
		    int ep_flag = 0;
		    chi_move filled;

		    if (target_square & HER_KNIGHTS (pos)) {
			material = 3 << 22;
			victim = knight << 16;
		    } else if (target_square & HER_BISHOPS (pos)) {
			material = 3 << 22;
			victim = bishop << 16;
			if (target_square & HER_ROOKS (pos)) {
			    material = 9 << 22;
			    victim = queen << 16;
			}
		    } else if (target_square & HER_ROOKS (pos)) {
			material = 5 << 22;
			victim = rook << 16;
		    } else if (to == ep_offset) {
			ep_flag = 0x1000;
		    }

		    filled = move | victim | ep_flag | material;

		    if (to_mask & PAWN_PROMOTE_RANK_MASK) {
			*moves++ = filled | (queen << 19) | 
			    (8 << 22);
			*moves++ = filled | (rook << 19) | 
			    (4 << 22);
			*moves++ = filled | (bishop << 19) | 
			    (2 << 22);
			*moves++ = filled | (knight << 19) | 
			    (2 << 22);
		    } else {
			*moves++ = filled;
		    }
		}
	    }
	    
	    piece_mask = chi_clear_least_set (piece_mask);
	}
    }

    /* Knight captures.  */
    piece_mask = MY_KNIGHTS (pos);
    while (piece_mask) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 attack_mask = knight_attacks[from] & her_squares;
	chi_move move = from | ((~knight & 0x7) << 13);

	while (attack_mask) {
	    unsigned int to = 
		chi_bitv2shift (chi_clear_but_least_set (attack_mask));
	    bitv64 target_square = ((bitv64) 1 << to);
	    int material = 1;
	    chi_piece_t victim = pawn;

	    if (target_square & HER_KNIGHTS (pos)) {
		material = 3;
		victim = knight;
	    } else if (target_square & HER_BISHOPS (pos)) {
		material = 3;
		victim = bishop;
		if (target_square & HER_ROOKS (pos)) {
		    material = 9;
		    victim = queen;
		}
	    } else if (target_square & HER_ROOKS (pos)) {
		material = 5;
		victim = rook;
	    }

	    *moves++ = move | (to << 6) | (victim << 16) | (material << 22);

	    attack_mask = chi_clear_least_set (attack_mask);
	}
	
	piece_mask = chi_clear_least_set (piece_mask);
    }

    /* Bishop attacks.  */
    piece_mask = MY_BISHOPS (pos);
    while (piece_mask) {
	bitv64 isol_mask = chi_clear_but_least_set (piece_mask);
	unsigned int from = chi_bitv2shift (isol_mask);
	unsigned int to;
	chi_piece_t piece = ((((bitv64) 1) << from) & MY_ROOKS (pos)) ?
	    (~queen & 0x7) : (~bishop & 0x7);

	bitv64 dest_mask = (isol_mask & ~(CHI_A_MASK | CHI_8_MASK)) << 9;

	to = from;
	while (dest_mask) {
	    to += 9;

	    if (dest_mask & her_squares) {
		int material = 1;
		chi_piece_t victim = pawn;

		if (dest_mask & HER_KNIGHTS (pos)) {
		    material = 3;
		    victim = knight;
		} else if (dest_mask & HER_BISHOPS (pos)) {
		    material = 3;
		    victim = bishop;
		    if (dest_mask & HER_ROOKS (pos)) {
			material = 9;
			victim = queen;
		    }
		} else if (dest_mask & HER_ROOKS (pos)) {
		    material = 5;
		    victim = rook;
		}
	    
		*moves++ = from | (to << 6) | (piece << 13) | (victim << 16) |
		    (material << 22);
		break;
	    }

	    dest_mask = ((dest_mask & empty_squares & 
			  ~(CHI_A_MASK | CHI_8_MASK)) << 9);
	}

	dest_mask = (isol_mask & ~(CHI_H_MASK | CHI_8_MASK)) << 7;
	to = from;
	while (dest_mask) {
	    to += 7;

	    if (dest_mask & her_squares) {
		int material = 1;
		chi_piece_t victim = pawn;

		if (dest_mask & HER_KNIGHTS (pos)) {
		    material = 3;
		    victim = knight;
		} else if (dest_mask & HER_BISHOPS (pos)) {
		    material = 3;
		    victim = bishop;
		    if (dest_mask & HER_ROOKS (pos)) {
			material = 9;
			victim = queen;
		    }
		} else if (dest_mask & HER_ROOKS (pos)) {
		    material = 5;
		    victim = rook;
		}
	    
		*moves++ = from | (to << 6) | (piece << 13) | (victim << 16) |
		    (material << 22);
		break;
	    }

	    dest_mask = ((dest_mask & empty_squares & 
			  ~(CHI_H_MASK | CHI_8_MASK)) << 7);
	}

	dest_mask = (isol_mask & ~(CHI_A_MASK | CHI_1_MASK)) >> 7;
	to = from;
	while (dest_mask) {
	    to -= 7;

	    if (dest_mask & her_squares) {
		int material = 1;
		chi_piece_t victim = pawn;

		if (dest_mask & HER_KNIGHTS (pos)) {
		    material = 3;
		    victim = knight;
		} else if (dest_mask & HER_BISHOPS (pos)) {
		    material = 3;
		    victim = bishop;
		    if (dest_mask & HER_ROOKS (pos)) {
			material = 9;
			victim = queen;
		    }
		} else if (dest_mask & HER_ROOKS (pos)) {
		    material = 5;
		    victim = rook;
		}
	    
		*moves++ = from | (to << 6) | (piece << 13) | (victim << 16) |
		    (material << 22);
		break;
	    }

	    dest_mask = ((dest_mask & empty_squares & 
			  ~(CHI_A_MASK | CHI_1_MASK)) >> 7);
	}

	dest_mask = (isol_mask & ~(CHI_H_MASK | CHI_1_MASK)) >> 9;
	to = from;
	while (dest_mask) {
	    to -= 9;

	    if (dest_mask & her_squares) {
		int material = 1;
		chi_piece_t victim = pawn;

		if (dest_mask & HER_KNIGHTS (pos)) {
		    material = 3;
		    victim = knight;
		} else if (dest_mask & HER_BISHOPS (pos)) {
		    material = 3;
		    victim = bishop;
		    if (dest_mask & HER_ROOKS (pos)) {
			material = 9;
			victim = queen;
		    }
		} else if (dest_mask & HER_ROOKS (pos)) {
		    material = 5;
		    victim = rook;
		}
	    
		*moves++ = from | (to << 6) | (piece << 13) | (victim << 16) |
		    (material << 22);
		break;
	    }

	    dest_mask = ((dest_mask & empty_squares & 
			  ~(CHI_H_MASK | CHI_1_MASK)) >> 9);
	}

	piece_mask = chi_clear_least_set (piece_mask);
    }

    /* We currently take the traditional approach for the rook moves.
       The normal bitboard for the rooks plus an additional bitboard rotated 
       by 90 degrees is updated.  The target square mask is then read from
       a precomputed array.
       
       Another approach would be to keep four distinct bitboards, rotated
       by 0, 90, 180, and 270 degrees.  Moves to the "left" (in the
       direction of the most significant bit) can then be determined
       with (almost) no memory access:

       Step 1) Isolate the bit of the moving piece into FROM_MASK.

       Step 2) Remember the logical OR of FROM_MASK and (FROM_MASK - 1).
       
       Step 3) Mask out the bits generated by step 2 from the
               bitboard containing all pieces ANDed with the mask for
	       that rank/file, in other words, mask out the moving piece, 
	       plus all pieces to the "right" of it.

       Step 4) Extract the least significant bit from the result
               of step 3, which is the attacked square (whe

       For non-captures we have to take a slightly different approach.

       We can then determine all rook moves without accessing the
       precomputed two-dimensional state/file resp. state/rank 
       array.  On the other hand, we have to updated two more bitboards
       while moving and we either have to lookup the file/rank masks for
       all non-zero-degree directions, or have to do divisions (and
       shifts) to get them.

       It would be interesting to compare that approach to the one
       used here.
     */

    piece_mask = MY_ROOKS (pos);
    while (piece_mask) {
	int from = chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 state = (rank_masks[from] & occ_squares) >> ((from >> 3) << 3);
	bitv64 state90 = (file_masks[from] & occ90_squares) >> 
	    ((rotate90[from] >> 3) << 3);
	bitv64 hor_attack_mask = rook_hor_attack_masks[from][state];
	bitv64 ver_attack_mask = rook_ver_attack_masks[from][state90];
	chi_piece_t piece = ((((bitv64) 1) << from) & MY_BISHOPS (pos)) ?
	    (~queen & 0x7) : (~rook & 0x7);

	bitv64 attack_mask = hor_attack_mask | ver_attack_mask;
	attack_mask &= her_squares;

	while (attack_mask) {
	    unsigned int to = 
		chi_bitv2shift (chi_clear_but_least_set (attack_mask));
	    bitv64 dest_mask = ((bitv64) 1) << to;
	    int material = 1;
	    chi_piece_t victim = pawn;
	    
	    if (dest_mask & HER_KNIGHTS (pos)) {
		material = 3;
		victim = knight;
	    } else if (dest_mask & HER_BISHOPS (pos)) {
		material = 3;
		victim = bishop;
		    if (dest_mask & HER_ROOKS (pos)) {
			material = 9;
			victim = queen;
		    }
	    } else if (dest_mask & HER_ROOKS (pos)) {
		material = 5;
		victim = rook;
	    }
	    
	    *moves++ = from | (to << 6) | (piece << 13) | (victim << 16) |
		(material << 22);
	    
	    attack_mask = chi_clear_least_set (attack_mask);
	}
	
	piece_mask = chi_clear_least_set (piece_mask);
    }

    /* King captures.  */    
    piece_mask = MY_KINGS (pos);
    while (piece_mask) {
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 attack_mask = king_attacks[from] & her_squares;

	while (attack_mask) {
	    unsigned int to = 
		chi_bitv2shift (chi_clear_but_least_set (attack_mask));
	    bitv64 dest_mask = ((bitv64) 1) << to;
	    int material = 1;
	    chi_piece_t victim = pawn;
	    
	    if (dest_mask & HER_KNIGHTS (pos)) {
		material = 3;
		victim = knight;
	    } else if (dest_mask & HER_BISHOPS (pos)) {
		material = 3;
		victim = bishop;
		if (dest_mask & HER_ROOKS (pos)) {
		    material = 9;
		    victim = queen;
		}
	    } else if (dest_mask & HER_ROOKS (pos)) {
		material = 5;
		victim = rook;
	    }
	    
	    *moves++ = from | (to << 6) | ((~king & 0x7) << 13) | (victim << 16) |
		(material << 22);
	    
	    attack_mask = chi_clear_least_set (attack_mask);
	}
	
	break;
	/* piece_mask = chi_clear_least_set (piece_mask); */	
    }
    
    return moves;
}

chi_move* 
chi_generate_color_pawn_double_steps (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 piece_mask = PAWN_START_MASK & MY_PAWNS (pos);

    if (piece_mask) {
	bitv64 empty_squares = ~(pos->w_pieces | pos->b_pieces);
	
	/* Pawn double steps.  */
	while (piece_mask) {
	    unsigned int from = 
		chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);
	    
	    if (to_mask & empty_squares) {	    
		to_mask = ((bitv64) 1) << (from + DOUBLE_PAWN_OFFSET);
		
		if (to_mask & empty_squares)
		    *moves++ = (from | ((from + DOUBLE_PAWN_OFFSET) << 6) |
				((~pawn & 0x7) << 13));
	    }
	    
	    piece_mask = chi_clear_least_set (piece_mask);
	}
    }

    return moves;
}

chi_move* 
chi_generate_color_pawn_single_steps (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 piece_mask = MY_PAWNS (pos) & ~PAWN_PRE_PROMOTE_RANK_MASK;

    if (piece_mask) {
	bitv64 empty_squares = ~(pos->w_pieces | pos->b_pieces);

	while (piece_mask) {
	    unsigned int from = 
		chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 to_mask = ((bitv64) 1) << (from + SINGLE_PAWN_OFFSET);
	    
	    if (to_mask & empty_squares)
		*moves++ = (from | ((from + SINGLE_PAWN_OFFSET) << 6) |
			       ((~pawn & 0x7) << 13));
	    
	    piece_mask = chi_clear_least_set (piece_mask);
	}
    }

    return moves;
}

chi_move* 
chi_generate_color_knight_moves (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 piece_mask = MY_KNIGHTS (pos);

    if (piece_mask) {
	bitv64 empty_squares = ~(pos->w_pieces | pos->b_pieces);
	while (piece_mask) {
	    unsigned int from = 
		chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 attack_mask = knight_attacks[from] & empty_squares;
	    
	    while (attack_mask) {
		unsigned int to = 
		    chi_bitv2shift (chi_clear_but_least_set (attack_mask));
		*moves++ = from | (to << 6) | ((~knight & 0x7) << 13);
		attack_mask = chi_clear_least_set (attack_mask);
	    }
	    
	    piece_mask = chi_clear_least_set (piece_mask);
	}
    }

    return moves;
}

chi_move* 
chi_generate_color_bishop_moves (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 piece_mask = MY_BISHOPS (pos);

    if (piece_mask) {
	bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
	bitv64 empty_squares = ~occ_squares;

	while (piece_mask) {
	    bitv64 isol_mask = chi_clear_but_least_set (piece_mask);
	    unsigned int from = chi_bitv2shift (isol_mask);
	    chi_piece_t piece = ((((bitv64) 1) << from) & MY_ROOKS (pos)) ?
		(~queen & 0x7) : (~bishop & 0x7);
	    unsigned int to;
	    
	    bitv64 dest_mask = empty_squares & 
		((isol_mask & ~(CHI_A_MASK | CHI_8_MASK)) << 9);

	    to = from;
	    while (dest_mask) {
		to += 9;
		*moves++ = from | (to << 6) | (piece << 13);		
		dest_mask = empty_squares & 
		    ((dest_mask & ~(CHI_A_MASK | CHI_8_MASK)) << 9);
	    }
	    
	    dest_mask = empty_squares & 
		((isol_mask & ~(CHI_H_MASK | CHI_8_MASK)) << 7);
	    to = from;
	    while (dest_mask) {
		to += 7;
		*moves++ = from | (to << 6) | (piece << 13);
		dest_mask = empty_squares & 
		    ((dest_mask & ~(CHI_H_MASK | CHI_8_MASK)) << 7);
	    }
	    
	    dest_mask = empty_squares & 
		((isol_mask & ~(CHI_A_MASK | CHI_1_MASK)) >> 7);
	    to = from;
	    while (dest_mask) {
		to -= 7;
		*moves++ = from | (to << 6) | (piece << 13);		
		dest_mask = empty_squares & 
		    ((dest_mask & ~(CHI_A_MASK | CHI_1_MASK)) >> 7);
	    }
	    
	    dest_mask = empty_squares & 
		((isol_mask & ~(CHI_H_MASK | CHI_1_MASK)) >> 9);
	    to = from;
	    while (dest_mask) {
		to -= 9;
		*moves++ = from | (to << 6) | (piece << 13);		
		dest_mask = empty_squares & 
		    ((dest_mask & ~(CHI_H_MASK | CHI_1_MASK)) >> 9);
	    }
	    
	    piece_mask = chi_clear_least_set (piece_mask);
	}
    }

    return moves;
}

chi_move* 
chi_generate_color_rook_moves (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 piece_mask = MY_ROOKS (pos);

    if (piece_mask) {
	bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
	bitv64 occ90_squares = pos->w_pieces90 | pos->b_pieces90;

	while (piece_mask) {
	    int from = chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	    bitv64 state = (rank_masks[from] & occ_squares) >> 
		((from >> 3) << 3);
	    bitv64 state90 = (file_masks[from] & occ90_squares) >> 
		((rotate90[from] >> 3) << 3);
	    bitv64 hor_slide_mask = rook_hor_slide_masks[from][state];
	    bitv64 ver_slide_mask = rook_ver_slide_masks[from][state90];
	    bitv64 slide_mask = hor_slide_mask | ver_slide_mask;
	    chi_piece_t piece = ((((bitv64) 1) << from) & MY_BISHOPS (pos)) ?
		(~queen & 0x7) : (~rook & 0x7);
	    
	    while (slide_mask) {
		unsigned int to = 
		    chi_bitv2shift (chi_clear_but_least_set (slide_mask));

		*moves++ = (from | (to << 6) | (piece << 13));
		slide_mask = chi_clear_least_set (slide_mask);
	    }
	    
	    piece_mask = chi_clear_least_set (piece_mask);
	}
    }

    return moves;
}

chi_move* 
chi_generate_color_king_moves (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 piece_mask = MY_KINGS (pos);

    if (piece_mask) {
	bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
	bitv64 empty_squares = ~occ_squares;
	unsigned int from = 
	    chi_bitv2shift (chi_clear_but_least_set (piece_mask));
	bitv64 attack_mask = king_attacks[from] & empty_squares;
	    
	while (attack_mask) {
	    unsigned int to = 
		chi_bitv2shift (chi_clear_but_least_set (attack_mask));
	    
	    *moves++ = from | (to << 6) | ((~king & 0x7) << 13);
	    attack_mask = chi_clear_least_set (attack_mask);
	}
    }

    return moves;
}

chi_move* 
chi_generate_color_king_castling_moves (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    bitv64 occ_squares = pos->w_pieces | pos->b_pieces;

    if (KING_CASTLE (pos) && 
	(!(occ_squares & KING_CASTLE_CROSS_MASK)))
	*moves++ = KING_CASTLE_MOVE;

    if (QUEEN_CASTLE (pos) && 
	(!(occ_squares & QUEEN_CASTLE_CROSS_MASK)))
	*moves++ = QUEEN_CASTLE_MOVE;

    return moves;
}

chi_move* 
chi_generate_color_non_captures (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    moves = chi_generate_color_king_castling_moves (pos, moves);
    moves = chi_generate_color_pawn_double_steps (pos, moves);
    moves = chi_generate_color_pawn_single_steps (pos, moves);
    moves = chi_generate_color_knight_moves (pos, moves);
    moves = chi_generate_color_bishop_moves (pos, moves);
    moves = chi_generate_color_rook_moves (pos, moves);
    moves = chi_generate_color_king_moves (pos, moves);

    return moves;
}

int
chi_color_check_check (pos)
     chi_pos* pos;
{
    bitv64 king_mask = MY_KINGS (pos);
    register unsigned int king_shift = 
	chi_bitv2shift (chi_clear_but_least_set (king_mask));
    bitv64 occ_squares = pos->w_pieces | pos->b_pieces;
    bitv64 her_bishops, her_rooks;

    if (knight_attacks[king_shift] & HER_KNIGHTS (pos))
	return 1;

    if (king_attacks[king_shift] & HER_KINGS (pos))
	return 1;

    /* Pawn attacks.  */
    if (king_mask & ~PAWN_PROMOTE_RANK_MASK) {
	if ((king_mask & ~CHI_A_MASK) &&
	    (HER_PAWNS (pos) & (LEFT_PAWN_CAPTURE_SHIFT (king_mask))))
	    return 1;
	if ((king_mask & ~CHI_H_MASK) &&
	    (HER_PAWNS (pos) & (RIGHT_PAWN_CAPTURE_SHIFT (king_mask))))
	    return 1;
    }

    /* Bishop attacks.  */
    her_bishops = HER_BISHOPS (pos) & bishop_king_attacks[king_shift];
    while (her_bishops) {
	int bishop_shift = 
	    chi_bitv2shift (chi_clear_but_least_set (her_bishops));

	if (!(bishop_king_intermediates[king_shift][bishop_shift] &
	      occ_squares))
	    return 1;

	her_bishops = chi_clear_least_set (her_bishops);
    }

    /* Rook attacks.  */
    her_rooks = HER_ROOKS (pos) & rook_king_attacks[king_shift];
    while (her_rooks) {
	int rook_shift = 
	    chi_bitv2shift (chi_clear_but_least_set (her_rooks));

	if (!(rook_king_intermediates[king_shift][rook_shift] &
	      occ_squares))
	    return 1;

	her_rooks = chi_clear_least_set (her_rooks);
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
