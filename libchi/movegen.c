/* movegen.c - Move generators.
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

#include "bitmasks.h"

static chi_move* chi_generate_white_captures CHI_PARAMS ((chi_pos*, 
							  chi_move*));
static chi_move* chi_generate_black_captures CHI_PARAMS ((chi_pos*, 
							  chi_move*));
static chi_move* chi_generate_white_non_captures CHI_PARAMS ((chi_pos*, 
							      chi_move*));
static chi_move* chi_generate_black_non_captures CHI_PARAMS ((chi_pos*, 
							      chi_move*));

chi_move* 
chi_generate_captures (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    if (chi_to_move (pos) == chi_white)
	return chi_generate_white_captures (pos, moves);
    else
	return chi_generate_black_captures (pos, moves);
}

chi_move* 
chi_generate_non_captures (pos, moves)
     chi_pos* pos;
     chi_move* moves;
{
    if (chi_to_move (pos) == chi_white)
	return chi_generate_white_non_captures (pos, moves);
    else
	return chi_generate_black_non_captures (pos, moves);
}

/* Include the code for white and black moves respectively.  */
#define chi_generate_color_captures chi_generate_white_captures
#define chi_generate_color_non_captures chi_generate_white_non_captures
#define MY_PIECES(p) ((p)->w_pieces)
#define OPP_PIECES(p) ((p)->b_pieces)
#define MY_PAWNS(p) ((p)->w_pawns)
#define MY_KNIGHTS(p) ((p)->w_knights)
#define SINGLE_PAWN_OFFSET (8)
#define DOUBLE_PAWN_OFFSET (16)
#include "movegen_color.c"

#undef chi_generate_color_captures
#undef chi_generate_color_non_captures
#undef MY_PIECES
#undef OPP_PIECES
#undef MY_PAWNS
#undef MY_KNIGHTS
#undef SINGLE_PAWN_OFFSET
#undef DOUBLE_PAWN_OFFSET

#define chi_generate_color_captures chi_generate_black_captures
#define chi_generate_color_non_captures chi_generate_black_non_captures
#define MY_PIECES(p) ((p)->b_pieces)
#define OPP_PIECES(p) ((p)->w_pieces)
#define MY_PAWNS(p) ((p)->b_pawns)
#define MY_KNIGHTS(p) ((p)->b_knights)
#define SINGLE_PAWN_OFFSET (-8)
#define DOUBLE_PAWN_OFFSET (-16)
#include "movegen_color.c"

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
