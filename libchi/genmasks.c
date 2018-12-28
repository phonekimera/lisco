/* genmasks.c - Create pre-computed bitmasks.
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

#include <system.h>

#include <libchi.h>

#include <stdio.h>

#include <string.h>

static void bitv64_dump CHI_PARAMS ((bitv64, bitv64, int));
static const char* shift2label CHI_PARAMS ((unsigned int));
static void generate_knight_masks CHI_PARAMS ((void));
static void generate_king_masks CHI_PARAMS ((void));
#ifdef PAWN_LOOKUP
static void generate_wpawn_sg_steps CHI_PARAMS ((void));
static void generate_bpawn_sg_steps CHI_PARAMS ((void));
static void generate_wpawn_dbl_steps CHI_PARAMS ((void));
static void generate_bpawn_dbl_steps CHI_PARAMS ((void));

static void print_escaped_moves CHI_PARAMS ((unsigned int, 
					     const chi_move*));
static void print_rank_state CHI_PARAMS ((unsigned char state));
#endif

static const char* coordinates[] = {
    "h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
    "h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
    "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
    "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
    "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
    "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
    "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
    "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8",
};

int
main (argc, argv)
     int argc;
     char* argv[];
{
    printf ("\
/* This file is generated!  Edit genmasks.c for changes.\n\
\n\
   The attack masks for knights and kings map a bitshift value into a\n\
   bitmask of fields attacked by that piece.  */\n"); 
   
    generate_knight_masks ();
    generate_king_masks ();
#ifdef PAWN_LOOKUP
    generate_wpawn_sg_steps ();
    generate_bpawn_sg_steps ();
    generate_wpawn_dbl_steps ();
    generate_bpawn_dbl_steps ();
#endif

    return 0;
}

static void
bitv64_dump (from_mask, to_mask, piece_char)
     bitv64 from_mask;
     bitv64 to_mask;
     int piece_char; 
{
    int file, rank;

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1 && rank <= CHI_RANK_8; --rank) {
	printf ("\t   ");
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int shift = chi_coords2shift (file, rank);

	    if ((1ULL << shift) & from_mask) {
		printf (" %c", piece_char);
	    } else if ((1ULL << shift) & to_mask) {
		printf (" X");
	    } else {
		printf (" .");
	    }
	}
	printf ("\n");
    }

}

static const char*
shift2label (shift)
     unsigned int shift;
{
	if (shift < 64)
		return coordinates[shift];
	else
		return "??";
}

static void
generate_king_masks ()
{
    bitv64 to_mask[64];
    int file, rank, i;

    memset (to_mask, 0, sizeof to_mask);

    for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
	    int from_shift = chi_coords2shift (file, rank);

	    if (file > CHI_FILE_A && rank < CHI_RANK_8)
		to_mask[from_shift] |= (1ULL << (from_shift + 9));

	    if (rank < CHI_RANK_8)
		to_mask[from_shift] |= (1ULL << (from_shift + 8));

	    if (file < CHI_FILE_H && rank < CHI_RANK_8)
		to_mask[from_shift] |= (1ULL << (from_shift + 7));

	    if (file < CHI_FILE_H)
		to_mask[from_shift] |= (1ULL << (from_shift - 1));

	    if (file < CHI_FILE_H && rank > CHI_RANK_1)
		to_mask[from_shift] |= (1ULL << (from_shift - 9));

	    if (rank > CHI_RANK_1)
		to_mask[from_shift] |= (1ULL << (from_shift - 8));

	    if (file > CHI_FILE_A && rank > CHI_RANK_1)
		to_mask[from_shift] |= (1ULL << (from_shift - 7));

	    if (file > CHI_FILE_A)
		to_mask[from_shift] |= (1ULL << (from_shift + 1));
	}
    }

    printf ("\n/* King attack masks.  */\n");
    printf ("static const bitv64 king_attacks[64] = {\n");

    for (i = 0; i < 64; ++i) {
	printf ("\t/* (0x%016llx) K%s ->\n", 
		(bitv64) 1 << i, shift2label (i));
	bitv64_dump ((bitv64) 1 << i, to_mask[i], 'K');
	printf ("\t*/\n");
	printf ("\t(bitv64) 0x%016llx,\n", to_mask[i]);
    }

    printf ("};\n");
}

static void
generate_knight_masks ()
{
    bitv64 to_mask[64];
    int file, rank, i;

    memset (to_mask, 0, sizeof to_mask);

    for (rank = CHI_RANK_1; rank <= CHI_RANK_7; ++rank) {
	for (file = CHI_FILE_C; file <= CHI_FILE_H; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file - 2, rank + 1);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_1; rank <= CHI_RANK_6; ++rank) {
	for (file = CHI_FILE_B; file <= CHI_FILE_H; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file - 1, rank + 2);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_1; rank <= CHI_RANK_6; ++rank) {
	for (file = CHI_FILE_A; file <= CHI_FILE_G; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file + 1, rank + 2);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_1; rank <= CHI_RANK_7; ++rank) {
	for (file = CHI_FILE_A; file <= CHI_FILE_F; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file + 2, rank + 1);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_2; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_A; file <= CHI_FILE_F; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file + 2, rank - 1);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_3; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_A; file <= CHI_FILE_G; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file + 1, rank - 2);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_3; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_B; file <= CHI_FILE_H; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file - 1, rank - 2);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    for (rank = CHI_RANK_3; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_C; file <= CHI_FILE_H; ++file) {
	    int from_shift = chi_coords2shift (file, rank);
	    int target_shift = chi_coords2shift (file - 2, rank - 1);
	    to_mask[from_shift] |= (1ULL << target_shift);
	}
    }

    printf ("\n/* Knight attack masks.  */\n");
    printf ("static const bitv64 knight_attacks[64] = {\n");

    for (i = 0; i < 64; ++i) {
	printf ("\t/* (0x%016llx) N%s ->\n", 
		(bitv64) 1 << i, shift2label (i));
	bitv64_dump ((bitv64) 1 << i, to_mask[i], 'N');
	printf ("\t*/\n");
	printf ("\t(bitv64) 0x%016llx,\n", to_mask[i]);
    }

    printf ("};\n");
}

#ifdef PAWN_LOOKUP
static void
print_escaped_moves (members, moves)
     unsigned int members;
     const chi_move* moves;
{
    int i;

    printf ("\t\"\\x%02x", members * sizeof *moves);
    for (i = 0; i < members; ++i) {
	unsigned char* ptr = (unsigned char*) (moves + i);
	int j;

	for (j = 0; j < sizeof *moves; ++j) {
	    printf ("\\x%02x", ptr[j]);
	}
    }
    printf ("\",\n");
}

static void
print_rank_state (state)
     unsigned char state;
{
    int i;

    printf ("\t/* Rank state: 0x%02x (", state);
    for (i = 0; i < 8; ++i) {
	if (state & (1 << i)) {
	    fputc ('1', stdout);
	} else {
	    fputc ('0', stdout);
	}
    }
    printf (").  */\n");
}

static void
generate_wpawn_sg_steps ()
{
    int state;
    chi_move moves[8];

    printf ("\n\
/* Pawn non-capturing moves from 2nd to 3rd rank.  The index into the\n\
   array is the \"state\" of the 3rd rank.  This state is calculated by\n\
   advancing the pawns one rank, and then ANDing it with the empty\n\
   squares.  */\n\
\n\
static const unsigned char* wpawn_sg_steps[256] = {\n");

    for (state = 0; state < 256; ++state) {
	int shift;
	unsigned int members;

	members = 0;
	
	print_rank_state (state);

	memset (moves, 0, sizeof moves);

	for (shift = 7; shift >= 0; --shift) {
	    if (state & (1 << shift)) {
		moves[members].fields.from = chi_coords2shift (7 - shift, 
							       CHI_RANK_2);
		moves[members].fields.to = chi_coords2shift (7 - shift, 
							     CHI_RANK_3);
		++members;
	    }
	}
	print_escaped_moves (members, moves);
    }

    printf ("};\n");
}

static void
generate_bpawn_sg_steps ()
{
    int state;
    chi_move moves[8];

    printf ("\n\
/* Pawn non-capturing moves from 7th to 6th rank.  The index into the\n\
   array is the \"state\" of the 6th rank.  This state is calculated by\n\
   advancing the pawns one rank, and then ANDing it with the empty\n\
   squares.  */\n\
\n\
static const unsigned char* bpawn_sg_steps[256] = {\n");

    for (state = 0; state < 256; ++state) {
	int shift;
	unsigned int members;

	members = 0;
	
	print_rank_state (state);

	memset (moves, 0, sizeof moves);

	for (shift = 7; shift >= 0; --shift) {
	    if (state & (1 << shift)) {
		moves[members].fields.from = chi_coords2shift (7 - shift, 
							       CHI_RANK_7);
		moves[members].fields.to = chi_coords2shift (7 - shift, 
							     CHI_RANK_6);
		++members;
	    }
	}
	print_escaped_moves (members, moves);
    }

    printf ("};\n");
}

static void
generate_wpawn_dbl_steps ()
{
    int state;
    chi_move moves[8];

    printf ("\n\
/* Pawn non-capturing moves from 2nd to 4th rank.  The index into the\n\
   array is the \"state\" of the 4th rank.  This state is calculated by\n\
   advancing the pawns two ranks, and then ANDing it with the empty\n\
   squares.  */\n\
\n\
static const unsigned char* wpawn_dbl_steps[256] = {\n");

    for (state = 0; state < 256; ++state) {
	int shift;
	unsigned int members;

	members = 0;
	
	print_rank_state (state);

	memset (moves, 0, sizeof moves);

	for (shift = 7; shift >= 0; --shift) {
	    if (state & (1 << shift)) {
		moves[members].fields.from = chi_coords2shift (7 - shift, 
							       CHI_RANK_2);
		moves[members].fields.to = chi_coords2shift (7 - shift, 
							     CHI_RANK_4);
		++members;
	    }
	}
	print_escaped_moves (members, moves);
    }

    printf ("};\n");
}

static void
generate_bpawn_dbl_steps ()
{
    int state;
    chi_move moves[8];

    printf ("\n\
/* Pawn non-capturing moves from 7th to 5th rank.  The index into the\n\
   array is the \"state\" of the 5th rank.  This state is calculated by\n\
   advancing the pawns two ranks, and then ANDing it with the empty\n\
   squares.  */\n\
\n\
static const unsigned char* bpawn_dbl_steps[256] = {\n");

    for (state = 0; state < 256; ++state) {
	int shift;
	unsigned int members;

	members = 0;
	
	print_rank_state (state);

	memset (moves, 0, sizeof moves);

	for (shift = 7; shift >= 0; --shift) {
	    if (state & (1 << shift)) {
		moves[members].fields.from = chi_coords2shift (7 - shift, 
							       CHI_RANK_7);
		moves[members].fields.to = chi_coords2shift (7 - shift, 
							     CHI_RANK_5);
		++members;
	    }
	}
	print_escaped_moves (members, moves);
    }

    printf ("};\n");
}

#endif
