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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#include <stdio.h>

#include <string.h>

static void print_rank_state(unsigned char state);

static void bitv64_dump(bitv64, bitv64, int, int);
static const char* shift2label(unsigned int);
static void generate_knight_masks(void);
static void generate_king_masks(void);
static void generate_rank_masks(void);
static void generate_file_masks(void);
static void generate_rook_hor_slide_masks(void);
static void generate_rook_hor_attack_masks(void);
static void generate_rook_ver_slide_masks(void);
static void generate_rook_ver_attack_masks(void);

static void generate_rook_king_attacks(void);
static void generate_rook_king_intermediates(void);

static void generate_bishop_king_attacks(void);
static void generate_bishop_king_intermediates(void);

#ifdef PAWN_LOOKUP
static void generate_wpawn_sg_steps(void);
static void generate_bpawn_sg_steps(void);
static void generate_wpawn_dbl_steps(void);
static void generate_bpawn_dbl_steps(void);

static void print_escaped_moves(unsigned int, const chi_move*);
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
    generate_rank_masks ();
    generate_file_masks ();
    generate_rook_hor_slide_masks ();
    generate_rook_hor_attack_masks ();
    generate_rook_ver_slide_masks ();
    generate_rook_ver_attack_masks ();

    generate_bishop_king_attacks ();
    generate_bishop_king_intermediates ();

    generate_rook_king_attacks ();
    generate_rook_king_intermediates ();

#ifdef PAWN_LOOKUP
    generate_wpawn_sg_steps ();
    generate_bpawn_sg_steps ();
    generate_wpawn_dbl_steps ();
    generate_bpawn_dbl_steps ();
#endif

    return 0;
}

static void
print_rank_state(unsigned char state)
{
    int i;

    printf ("\t/* Rank state: 0x%02x (", state);
    for (i = 7; i >= 0; --i) {
	if (state & (1 << i)) {
	    fputc ('1', stdout);
	} else {
	    fputc ('0', stdout);
	}
    }
    printf (").  */\n");
}

static void
bitv64_dump (from_mask, to_mask, piece_char, tabs)
     bitv64 from_mask;
     bitv64 to_mask;
     int piece_char; 
     int tabs;
{
    int file, rank;
    int tab;

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1 && rank <= CHI_RANK_8; --rank) {
	for (tab = 0; tab < tabs; ++tab)
	    fputc ('\t', stdout);
	
	printf ("   ");
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
	bitv64_dump ((bitv64) 1 << i, to_mask[i], 'K', 1);
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

    for (rank = CHI_RANK_2; rank <= CHI_RANK_8; ++rank) {
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
	bitv64_dump ((bitv64) 1 << i, to_mask[i], 'N', 1);
	printf ("\t*/\n");
	printf ("\t(bitv64) 0x%016llx,\n", to_mask[i]);
    }

    printf ("};\n");
}

static void
generate_rank_masks ()
{
    bitv64 to_mask[64];
    int file, rank, i;

    memset (to_mask, 0, sizeof to_mask);

    for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
	bitv64 mask = ((bitv64) 0xff) << (rank * 8);

	for (file = CHI_FILE_H; file >= CHI_FILE_A; --file) {
	    to_mask[chi_coords2shift (file, rank)] = mask;
	}
    }

    printf ("\n/* Rank masks.  */\n");
    printf ("static const bitv64 rank_masks[64] = {\n");

    for (i = 0; i < 64; ++i) {
	printf ("\t/* (0x%016llx) %s ->\n", 
		(bitv64) 1 << i, shift2label (i));
	bitv64_dump ((bitv64) 1 << i, to_mask[i], 'O', 1);
	printf ("\t*/\n");
	printf ("\t(bitv64) 0x%016llx,\n", to_mask[i]);
    }

    printf ("};\n");
}

static void
generate_file_masks ()
{
    bitv64 to_mask[64];
    int file, rank, i;

    memset (to_mask, 0, sizeof to_mask);

    for (rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
	bitv64 mask = ((bitv64) 0xff) << ((7 - rank) * 8);

	for (file = CHI_FILE_H; file >= CHI_FILE_A; --file) {
	    to_mask[chi_coords2shift90 (file, rank)] = mask;
	}
    }

    printf ("\n/* File masks.  */\n");
    printf ("static const bitv64 file_masks[64] = {\n");

    for (i = 0; i < 64; ++i) {
	printf ("\t/* (0x%016llx) %s ->\n", 
		(bitv64) 1 << i, shift2label (i));
	bitv64_dump ((bitv64) 1 << i, to_mask[i], 'O', 1);
	printf ("\t*/\n");
	printf ("\t(bitv64) 0x%016llx,\n", to_mask[i]);
    }

    printf ("};\n");
}

static void
generate_rook_hor_slide_masks ()
{
    unsigned int to_masks[8][256];
    int state;
    int rank, file;

    for (state = 0; state < 256; ++state) {
	for (file = CHI_FILE_H; file >= CHI_FILE_A; --file) {
	    int i;
	    int shift = chi_coords2shift (file, CHI_RANK_1);
	    unsigned int from_mask = 1 << shift;

	    to_masks[file][state] = 0;

	    for (i = 1; i < 8; ++i) {
		if ((from_mask >> i) & state)
		    break;
		to_masks[file][state] |= 0xff & (from_mask >> i);
	    }
	    for (i = 1; i < 8; ++i) {
		if ((from_mask << i) & state)
		    break;
		to_masks[file][state] |= 0xff & (from_mask << i);
	    }
	}
    }

    printf ("\n/* Rook masks.  */\n");
    printf ("static const bitv64 rook_hor_slide_masks[64][256] = {\n");

    for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_H; file >= CHI_FILE_A; --file) {
	    int i = chi_coords2shift (file, rank);
	    printf ("\t/* (0x%016llx) R%s -> (%d)\n", 
		    (bitv64) 1 << i, shift2label (i), i);
	    bitv64_dump (((bitv64) 1) << i, 0, 'R', 1);
	    printf ("\t*/\n");
	    printf ("\t{\n");

	    for (state = 0; state < 256; ++state) {
		bitv64 mask = ((bitv64) to_masks[file][state]) << (8 * rank);

		printf ("\t");
		print_rank_state ((unsigned char) state);
		printf ("\t\t(bitv64) 0x%016llx,\n", mask);
	    }
	    printf ("\t},\n");
	}
    }

    printf ("};\n");
}

static void
generate_rook_hor_attack_masks ()
{
    unsigned int to_masks[8][256];
    int state;
    int rank, file;

    for (state = 0; state < 256; ++state) {
	for (file = CHI_FILE_H; file >= CHI_FILE_A; --file) {
	    int i;
	    int shift = chi_coords2shift (file, CHI_RANK_1);
	    unsigned int from_mask = 1 << shift;

	    to_masks[file][state] = 0;

	    for (i = 1; i < 8; ++i) {
		if ((from_mask >> i) & state) {
		    to_masks[file][state] |= 0xff & (from_mask >> i);
		    break;
		}
	    }
	    for (i = 1; i < 8; ++i) {
		if ((from_mask << i) & state) {
		    to_masks[file][state] |= 0xff & (from_mask << i);
		    break;
		}
	    }
	}
    }

    printf ("\n/* Rook masks.  */\n");
    printf ("static const bitv64 rook_hor_attack_masks[64][256] = {\n");

    for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
	for (file = CHI_FILE_H; file >= CHI_FILE_A; --file) {
	    int i = chi_coords2shift (file, rank);
	    printf ("\t/* (0x%016llx) R%s -> (%d)\n", 
		    (bitv64) 1 << i, shift2label (i), i);
	    bitv64_dump (((bitv64) 1) << i, 0, 'R', 1);
	    printf ("\t*/\n");
	    printf ("\t{\n");

	    for (state = 0; state < 256; ++state) {
		bitv64 mask = ((bitv64) to_masks[file][state]) << (8 * rank);

		printf ("\t");
		print_rank_state ((unsigned char) state);
		printf ("\t\t/*\n");
		bitv64_dump (((bitv64) 1) << i, mask, 'R', 2);
		printf ("\t\t*/\n");
		printf ("\t\t(bitv64) 0x%016llx,\n", mask);
	    }
	    printf ("\t},\n");
	}
    }

    printf ("};\n");
}

static void
generate_rook_ver_slide_masks ()
{
    bitv64 to_masks[64][256];
    int file, rank;
    int shift;
    int state;

    memset (to_masks, 0, sizeof to_masks);

    for (state = 0; state < 256; ++state) {
	for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
	    int r;
	    bitv64 target_mask = 0;

	    for (r = rank + 1; r <= CHI_RANK_8; ++r) {
		int block_mask = 1 << (CHI_RANK_8 - r);

		if (block_mask & state)
		    break;

		target_mask |= (((bitv64) 1) << 
				(chi_coords2shift (CHI_FILE_A, r)));
	    }
	    for (r = rank - 1; r >= CHI_RANK_1; --r) {
		int block_mask = 1 << (CHI_RANK_8 - r);

		if (block_mask & state)
		    break;

		target_mask |= (((bitv64) 1) << 
				(chi_coords2shift (CHI_FILE_A, r)));
	    }

	    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
		shift = chi_coords2shift (file, rank);

		to_masks[shift][state] |= target_mask >> file;
	    }
	}
    }

    printf ("\n/* Rook masks.  */\n");
    printf ("static const bitv64 rook_ver_slide_masks[64][256] = {\n");

    for (shift = 0; shift < 64; ++shift) {
	printf ("\t/* (0x%016llx) R%s -> (%d) */\n", 
		(bitv64) 1 << shift, shift2label (shift), shift);
	printf ("\t{\n");
	for (state = 0; state < 256; ++state) {
	    printf ("\t");
	    print_rank_state ((unsigned char) state);
	    printf ("\t\t(bitv64) 0x%016llx,\n", to_masks[shift][state]);
	}
	printf ("\t},\n");
    }

    printf ("};\n");
}

static void
generate_rook_ver_attack_masks ()
{
    bitv64 to_masks[64][256];
    int file, rank;
    int shift;
    int state;

    memset (to_masks, 0, sizeof to_masks);

    for (state = 0; state < 256; ++state) {
	for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
	    int r;
	    bitv64 target_mask = 0;

	    for (r = rank + 1; r <= CHI_RANK_8; ++r) {
		int block_mask = 1 << (CHI_RANK_8 - r);

		if (block_mask & state) {
		    target_mask |= (((bitv64) 1) << 
				    (chi_coords2shift (CHI_FILE_A, r)));
		    break;
		}
	    }
	    for (r = rank - 1; r >= CHI_RANK_1; --r) {
		int block_mask = 1 << (CHI_RANK_8 - r);

		if (block_mask & state) {
		    target_mask |= (((bitv64) 1) << 
				    (chi_coords2shift (CHI_FILE_A, r)));
		    break;
		}
	    }

	    for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
		shift = chi_coords2shift (file, rank);

		to_masks[shift][state] |= target_mask >> file;
	    }
	}
    }

    printf ("\n/* Rook masks.  */\n");
    printf ("static const bitv64 rook_ver_attack_masks[64][256] = {\n");

    for (shift = 0; shift < 64; ++shift) {
	printf ("\t/* (0x%016llx) R%s -> (%d) */\n", 
		(bitv64) 1 << shift, shift2label (shift), shift);
	printf ("\t{\n");
	for (state = 0; state < 256; ++state) {
	    printf ("\t");
	    print_rank_state ((unsigned char) state);
	    printf ("\t\t(bitv64) 0x%016llx,\n", to_masks[shift][state]);
	}
	printf ("\t},\n");
    }

    printf ("};\n");
}

static void
generate_bishop_king_attacks ()
{
    int king_rank, king_file;
    int shift = 0;

    printf ("\n/* Bishop king attacks.  */\n");
    printf ("static const bitv64 bishop_king_attacks[64] = {\n");

    for (king_rank = CHI_RANK_1; king_rank <= CHI_RANK_8; ++king_rank) {
	for (king_file = CHI_FILE_H; king_file >= CHI_FILE_A; --king_file) {
	    int bishop_file, bishop_rank;
	    bitv64 mask = 0;

	    for (bishop_file = king_file - 1, bishop_rank = king_rank + 1;
		 bishop_file >= CHI_FILE_A && bishop_rank <= CHI_RANK_8;
		 --bishop_file, ++bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);
	    for (bishop_file = king_file + 1, bishop_rank = king_rank - 1;
		 bishop_file <= CHI_FILE_H && bishop_rank >= CHI_RANK_1;
		 ++bishop_file, --bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);
	    for (bishop_file = king_file + 1, bishop_rank = king_rank + 1;
		 bishop_file <= CHI_FILE_H && bishop_rank <= CHI_RANK_8;
		 ++bishop_file, ++bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);
	    for (bishop_file = king_file - 1, bishop_rank = king_rank - 1;
		 bishop_file >= CHI_FILE_A && bishop_rank >= CHI_RANK_1;
		 --bishop_file, --bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);

	    printf ("\t/* (0x%016llx) K%s ->\n", 
		    (bitv64) 1 << shift, shift2label (shift));
	    bitv64_dump ((bitv64) 1 << shift, mask, 'K', 1);
	    printf ("\t*/\n\t(bitv64) 0x%016llx,\n", mask);
	    
	    ++shift;
	}
    }

    printf ("};\n");
}

static void
generate_bishop_king_intermediates ()
{
    int king_shift, bishop_shift;
    bitv64 bishop_king_attacks[64];

    int king_rank, king_file;
    int shift = 0;

    for (king_rank = CHI_RANK_1; king_rank <= CHI_RANK_8; ++king_rank) {
	for (king_file = CHI_FILE_H; king_file >= CHI_FILE_A; --king_file) {
	    int bishop_file, bishop_rank;
	    bitv64 mask = 0;

	    for (bishop_file = king_file - 1, bishop_rank = king_rank + 1;
		 bishop_file >= CHI_FILE_A && bishop_rank <= CHI_RANK_8;
		 --bishop_file, ++bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);
	    for (bishop_file = king_file + 1, bishop_rank = king_rank - 1;
		 bishop_file <= CHI_FILE_H && bishop_rank >= CHI_RANK_1;
		 ++bishop_file, --bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);
	    for (bishop_file = king_file + 1, bishop_rank = king_rank + 1;
		 bishop_file <= CHI_FILE_H && bishop_rank <= CHI_RANK_8;
		 ++bishop_file, ++bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);
	    for (bishop_file = king_file - 1, bishop_rank = king_rank - 1;
		 bishop_file >= CHI_FILE_A && bishop_rank >= CHI_RANK_1;
		 --bishop_file, --bishop_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (bishop_file, 
							  bishop_rank);

	    bishop_king_attacks[shift] = mask;
	    ++shift;
	}
    }

    printf ("\n/* Bishop king intermediate squares.  */\n");
    printf ("static const bitv64 bishop_king_intermediates[64][64] = {\n");

    for (king_shift = 0; king_shift < 64; ++king_shift) {
	printf ("\t/* (0x%016llx) K%s.  */\n",
		(bitv64) 1 << king_shift, shift2label (king_shift));
	printf ("\t{\n");
	for (bishop_shift = 0; bishop_shift < 64; ++bishop_shift) {
	    bitv64 bishop_mask = (bitv64) 1 << bishop_shift;
	    bitv64 mask = (bitv64) -1;

	    if (bishop_king_attacks[king_shift] & bishop_mask) {
		int upper, lower;
		mask = 0;

		if (bishop_shift > king_shift) {
		    upper = bishop_shift;
		    lower = king_shift;
		} else {
		    upper = king_shift;
		    lower = bishop_shift;
		}

		if ((upper - lower) % 9 == 0) {
		    int i;

		    for (i = lower + 9; i < upper; i += 9)
			mask |= (bitv64) 1 << i;
		} else {
		    int i;

		    for (i = lower + 7; i < upper; i += 7)
			mask |= (bitv64) 1 << i;
		}
	    }

	    printf ("\t\t/* (0x%016llx) B%s -> K%s\n",
		    (bitv64) 1 << bishop_shift, 
		    shift2label (bishop_shift),
		    shift2label (king_shift));
	    bitv64_dump ((bitv64) 1 << bishop_shift, mask, 'B', 2);
	    printf ("\t\t*/\n\t\t(bitv64) 0x%016llx,\n", mask);
	}
	printf ("\t},\n");
    }

    printf ("};\n");
}

static void
generate_rook_king_attacks ()
{
    int king_rank, king_file;
    int shift = 0;

    printf ("\n/* Rook king attacks.  */\n");
    printf ("static const bitv64 rook_king_attacks[64] = {\n");

    for (king_rank = CHI_RANK_1; king_rank <= CHI_RANK_8; ++king_rank) {
	for (king_file = CHI_FILE_H; king_file >= CHI_FILE_A; --king_file) {
	    int rook_file, rook_rank;
	    bitv64 mask = 0;

	    for (rook_rank = king_rank + 1; rook_rank <= CHI_RANK_8;
		 ++rook_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (king_file,
							  rook_rank);
	    for (rook_rank = king_rank - 1; rook_rank >= CHI_RANK_1;
		 --rook_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (king_file,
							  rook_rank);
	    for (rook_file = king_file - 1; rook_file >= CHI_FILE_A;
		 --rook_file)
		mask |= ((bitv64) 1) << chi_coords2shift (rook_file,
							  king_rank);
	    for (rook_file = king_file + 1; rook_file <= CHI_FILE_H;
		 ++rook_file)
		mask |= ((bitv64) 1) << chi_coords2shift (rook_file,
							  king_rank);

	    printf ("\t/* (0x%016llx) K%s ->\n", 
		    (bitv64) 1 << shift, shift2label (shift));
	    bitv64_dump ((bitv64) 1 << shift, mask, 'K', 1);
	    printf ("\t*/\n\t(bitv64) 0x%016llx,\n", mask);
	    
	    ++shift;
	}
    }

    printf ("};\n");
}

static void
generate_rook_king_intermediates ()
{
    int king_shift, rook_shift;
    bitv64 rook_king_attacks[64];

    int king_rank, king_file;
    int shift = 0;

    for (king_rank = CHI_RANK_1; king_rank <= CHI_RANK_8; ++king_rank) {
	for (king_file = CHI_FILE_H; king_file >= CHI_FILE_A; --king_file) {
	    int rook_file, rook_rank;
	    bitv64 mask = 0;

	    for (rook_rank = king_rank + 1; rook_rank <= CHI_RANK_8;
		 ++rook_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (king_file,
							  rook_rank);
	    for (rook_rank = king_rank - 1; rook_rank >= CHI_RANK_1;
		 --rook_rank)
		mask |= ((bitv64) 1) << chi_coords2shift (king_file,
							  rook_rank);
	    for (rook_file = king_file - 1; rook_file >= CHI_FILE_A;
		 --rook_file)
		mask |= ((bitv64) 1) << chi_coords2shift (rook_file,
							  king_rank);
	    for (rook_file = king_file + 1; rook_file <= CHI_FILE_H;
		 ++rook_file)
		mask |= ((bitv64) 1) << chi_coords2shift (rook_file,
							  king_rank);

	    rook_king_attacks[shift] = mask;
	    ++shift;
	}
    }

    printf ("\n/* Rook king intermediate squares.  */\n");
    printf ("static const bitv64 rook_king_intermediates[64][64] = {\n");

    for (king_shift = 0; king_shift < 64; ++king_shift) {
	printf ("\t/* (0x%016llx) K%s.  */\n",
		(bitv64) 1 << king_shift, shift2label (king_shift));
	printf ("\t{\n");
	for (rook_shift = 0; rook_shift < 64; ++rook_shift) {
	    bitv64 rook_mask = (bitv64) 1 << rook_shift;
	    bitv64 mask = (bitv64) -1;

	    if (rook_king_attacks[king_shift] & rook_mask) {
		int upper, lower;
		mask = 0;

		if (rook_shift > king_shift) {
		    upper = rook_shift;
		    lower = king_shift;
		} else {
		    upper = king_shift;
		    lower = rook_shift;
		}

		if ((upper / 8) == (lower / 8)) {
		    int i;

		    for (i = lower + 1; i < upper; ++i)
			mask |= (bitv64) 1 << i;
		} else {
		    int i;

		    for (i = lower + 8; i < upper; i += 8)
			mask |= (bitv64) 1 << i;
		}
	    }

	    printf ("\t\t/* (0x%016llx) R%s -> K%s\n",
		    (bitv64) 1 << rook_shift, 
		    shift2label (rook_shift),
		    shift2label (king_shift));
	    bitv64_dump ((bitv64) 1 << rook_shift, mask, 'R', 2);
	    printf ("\t\t*/\n\t\t(bitv64) 0x%016llx,\n", mask);
	}
	printf ("\t},\n");
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
