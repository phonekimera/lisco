/* This file is part of the chess engine lisco.
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
#include <sys/types.h>

static void bitv64_dump(bitv64, bitv64, int, int);
static void obscured_dump(int from, int to, bitv64 mask,
	unsigned char direction, size_t tabs);
static const char* shift2label(unsigned int);

static void generate_obscured_masks(void);

static void generate_reverse_pawn_masks(void);

static void generate_knight_masks(void);
static void generate_king_masks(void);

static void generate_rook_king_attacks(void);
/* FIXME! Get rid of this? */
static void generate_rook_king_intermediates(void);

static void generate_bishop_king_attacks(void);
/* FIXME! Get rid of this? */
static void generate_bishop_king_intermediates(void);

#ifdef PAWN_LOOKUP
static void generate_wpawn_sg_steps(void);
static void generate_bpawn_sg_steps(void);
static void generate_wpawn_dbl_steps(void);
static void generate_bpawn_dbl_steps(void);

static void print_escaped_moves(unsigned int, const chi_move*);

static void print_rank_state(unsigned char state);
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

	generate_obscured_masks();

	generate_reverse_pawn_masks();

	generate_knight_masks ();
	generate_king_masks ();

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

#ifdef PAWN_LOOKUP
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
#endif

static void
bitv64_dump (bitv64 from_mask, bitv64 to_mask, int piece_char, int tabs)
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

static void
obscured_dump (int from, int to, bitv64 obscured_mask,
		unsigned char direction, size_t tabs)
{
	int file, rank;

	for (rank = CHI_RANK_8; rank >= CHI_RANK_1 && rank <= CHI_RANK_8; --rank) {
		for (size_t tab = 0; tab < tabs; ++tab) {
			fputc ('\t', stdout);
		}

		printf ("   ");
		for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
			int shift = chi_coords2shift (file, rank);

			if ((1ULL << shift) & obscured_mask) {
				printf(" %c", direction);
			} else if (shift == from) {
				if (from == to) {
					printf(" 0");
				} else {
					printf(" F");
				}
			} else if (shift == to) {
				printf(" T");
			} else {
				printf(" .");
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
print_binary(unsigned long long n)
{
	for (int i = 0; i < 64; ++i) {
			putc(n & 1ULL << i ? '1' : '0', stderr);
	}
	putc('\n', stderr);
}

static void
generate_obscured_masks()
{
	bitv64 obscured_masks[64][64];
	bitv64 mask;
	off_t condvar;
	unsigned char directions[64][64];
	
	memset(obscured_masks, 0, sizeof(obscured_masks[0][0]) * 64 * 64);
	memset(directions, '?', sizeof(directions[0][0]) * 64 * 64);

	for (off_t from = 0; from < 64; ++from) {
		/* North. */
		for (off_t to = from + 8; to < 64; to += 8) {
			off_t obscured;
			mask = 0ULL;
			for (obscured = to + 8; obscured < 64; obscured += 8) {
				mask |= (1ULL << obscured);
			}
			obscured_masks[from][to] = mask;
			directions[from][to] = '|';
		}

		/* North-east. */
		condvar = from % 8;
		for (off_t to = from + 7; to % 8 < condvar && to < 64; to += 7) {
			mask = 0ULL;
			for (off_t obscured = to + 7;
			     obscured % 8 < condvar && obscured < 64;
			     obscured += 7) {
				mask |= (1ULL << obscured);
			}
			obscured_masks[from][to] = mask;
			directions[from][to] = '/';
		}

		/* East. */
		condvar = from / 8;
		for (off_t to = from - 1; to / 8 == condvar && to >= 0; to -= 1) {
			mask = 0ULL;
			for (off_t obscured = to - 1;
			     obscured / 8 == condvar && obscured >= 0;
			     obscured -= 1) {
				mask |= (1ULL << obscured);
			}
			obscured_masks[from][to] = mask;
			directions[from][to] = '-';
		}

		/* South. */
		for (off_t to = from - 8; to >= 0; to -= 8) {
			mask = 0ULL;
			for (off_t obscured = to - 8; obscured >= 0; obscured -= 8) {
				mask |= (1ULL << obscured);
			}
			obscured_masks[from][to] = mask;
			directions[from][to] = '|';
		}
	}

	printf("\n/* Obscured masks (fields obscured by a move from F to T).  */\n");
	printf("static const bitv64 obscured_masks[64][64] = {\n");

	for (off_t from = 0; from < 64; ++from) {
		printf("\t/* From %lld (%s).  */\n", from, shift2label(from));
		printf("\t{\n");
		for (off_t to = 0; to < 64; ++to) {
			printf("\t\t/* From %lld (%s) to %lld (%s)\n",
					from, shift2label(from), to, shift2label(to));
			obscured_dump(from, to, obscured_masks[from][to],
					directions[from][to], 2);
			printf("\t\t*/\n");
			if (to != 63) {
				printf("\t\t0x%016llx,\n", obscured_masks[from][to]);
			} else {
				printf("\t\t0x%016llx\n", obscured_masks[from][to]);
			}
		}
		if (from != 63) {
			printf("\t},\n");
		} else {
			printf("\t}\n");
		}
	}

	printf("};\n");
}

static void
generate_reverse_pawn_masks()
{
	bitv64 white_to_mask[64], black_to_mask[64];

	for (int i = 0; i < 64; ++i) {
		if (i < 8 || i >= 56) {
			white_to_mask[i] = black_to_mask[i] = 0ULL;
		} else {
			bitv64 from_mask = 1ULL << i;
			white_to_mask[i] = black_to_mask[i] = 0ULL;
			if (from_mask & ~CHI_H_MASK) {
				white_to_mask[i] |= 1ULL << (i - 9);
				black_to_mask[i] |= 1ULL << (i + 7);
			}
			if (from_mask & ~CHI_A_MASK) {
				white_to_mask[i] |= 1ULL << (i - 7);
				black_to_mask[i] |= 1ULL << (i + 9);
			}
		}
	}

	printf ("\n/* Reverse pawn attack masks.  */\n");
	printf ("static const bitv64 reverse_pawn_attacks[2][64] = {\n");

	printf("\t/* White pawns.  */\n");
	printf("\t{\n");
	for (int i = 0; i < 64; ++i) {
		printf("\t/* (0x%016llx) %s(%d) ->\n",
				(bitv64) 1 << i, shift2label (i), i);
		bitv64_dump((bitv64) 1 << i, white_to_mask[i], 'O', 1);
		printf("\t*/\n");
		printf("\t(bitv64) 0x%016llxULL,\n", white_to_mask[i]);
	}
	printf("\t},\n");

	printf("\t/* Black pawns.  */\n");
	printf("\t{\n");
	for (int i = 0; i < 64; ++i) {
		printf("\t/* (0x%016llx) %s ->\n",  (bitv64) 1 << i, shift2label (i));
		bitv64_dump((bitv64) 1 << i, black_to_mask[i], 'O', 1);
		printf("\t*/\n");
		printf("\t(bitv64) 0x%016llxULL,\n", black_to_mask[i]);
	}
	printf("\t},\n");
	printf("};\n");
}

static void
generate_king_masks ()
{
	bitv64 to_mask[64];
	int file, rank, i;

	memset(to_mask, 0, sizeof to_mask);

	for (rank = CHI_RANK_1; rank <= CHI_RANK_8; ++rank) {
		for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
			int from_shift = chi_coords2shift(file, rank);

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

	printf("\n/* King attack masks.  */\n");
	printf("static const bitv64 king_attacks[64] = {\n");

	for (i = 0; i < 64; ++i) {
		printf("\t/* (0x%016llx) K%s ->\n",  (bitv64) 1 << i, shift2label (i));
		bitv64_dump((bitv64) 1 << i, to_mask[i], 'K', 1);
		printf("\t*/\n");
		printf("\t(bitv64) 0x%016llx,\n", to_mask[i]);
	}

	printf("};\n");
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
