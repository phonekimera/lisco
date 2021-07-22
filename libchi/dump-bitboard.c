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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "libchi.h"
#include "error.h"
#include "getprogname.h"

static void
dump_bitboard(bitv64 bitboard, unsigned char marker)
{
		int file, rank;

	for (rank = CHI_RANK_8; rank >= CHI_RANK_1 && rank <= CHI_RANK_8; --rank) {
		printf (" %c ", '1' + rank);
		for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
			int shift = chi_coords2shift (file, rank);
  
			if ((1ULL << shift) & bitboard) {
				printf(" %c", marker);
			} else {
				printf(" .");
			}
		}

		printf ("\n");
	}

	printf("   ");
	for (file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
		printf(" %c", file + 'a');
	}
	printf("\n");
}

int
main (int argc, char* argv[])
{
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Usage: %s BITBOARD [MARKER]\n", getprogname());
		return EXIT_FAILURE;
	}

	errno = 0;
	char *endptr;
	bitv64 bitboard = strtoull(argv[1], &endptr, 0);

	if (errno)
		error(EXIT_FAILURE, errno, "Invalid bitboard: %s\n", strerror(errno));

	if (*endptr)
		error(EXIT_FAILURE, 0, "Bitboard contains trailing garbage '%s'.\n", endptr);

	unsigned char marker = argc == 3 ? argv[2][0] : 'X';

	dump_bitboard(bitboard, marker);

	return EXIT_SUCCESS;
}
