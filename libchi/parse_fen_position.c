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

#include <string.h>

#include <libchi.h>

int
chi_parse_fen_position (chi_pos *argpos, const char *fen, const char **end_ptr)
{
	int file = 0;
	int rank = 0;
	int shift = 63;
	bitv64 mask = ((bitv64) 1) << shift;
	const char* ptr = fen;
	int material = 0;
	chi_pos tmp_pos;
	chi_pos* pos = &tmp_pos;

	if (!argpos || !fen)
		return CHI_ERR_YOUR_FAULT;

	chi_clear_position (pos);

	while (*ptr == ' ' || *ptr == '\t')
		++ptr;

	while (shift >= 0) {
		switch (*ptr) {
		case 'P':
			if (rank == 0 || rank == 7)
				return CHI_ERR_ILLEGAL_FEN;
			pos->w_pawns |= mask;
			material += 1;
			break;
		case 'p':
			if (rank == 0 || rank == 7)
				return CHI_ERR_ILLEGAL_FEN;
			pos->b_pawns |= mask;
			material -= 1;
			break;
		case 'N':
			material += 3;
			pos->w_knights |= mask;
			break;
		case 'n':
			material -= 3;
			pos->b_knights |= mask;
			break;
		case 'B':
			material += 3;
			pos->w_bishops |= mask;
			break;
		case 'b':
			material -= 3;
			pos->b_bishops |= mask;
			break;
		case 'R':
			material += 5;
			pos->w_rooks |= mask;
			break;
		case 'r':
			material -= 5;
			pos->b_rooks |= mask;
			break;
		case 'Q':
			material += 9;
			pos->w_bishops |= mask;
			pos->w_rooks |= mask;
			break;
		case 'q':
			material -= 9;
			pos->b_bishops |= mask;
			pos->b_rooks |= mask;
			break;
		case 'K':
			if (pos->w_kings)
			return CHI_ERR_TWO_KINGS;
			pos->w_kings |= mask;
			break;
		case 'k':
			if (pos->b_kings)
				return CHI_ERR_TWO_KINGS;
			pos->b_kings |= mask;
			break;
		case '1':
			break;
		case '2':
			file += 1;
			break;
		case '3':
			file += 2;
			break;
		case '4':
			file += 3;
			break;
		case '5':
			file += 4;
			break;
		case '6':
			file += 5;
			break;
		case '7':
			file += 6;
			break;
		case '8':
			file += 7;
			break;
		case '/':
			++rank;
			file = -1;
			break;
		case ' ':
		case '\t':
			break;
		default:
			return CHI_ERR_ILLEGAL_FEN;
		}

		++file;

		if (file > 8 || rank > 8)
			return CHI_ERR_ILLEGAL_FEN;

		if (*ptr == ' ' || *ptr == '\t') {
			if (rank != 7)
				return CHI_ERR_ILLEGAL_FEN;
			break;
		}

		++ptr;
		shift = 63 - (rank * 8 + file);
		mask = ((bitv64) 1) << shift;
	}

	if (!pos->w_kings || !pos->b_kings)
		return CHI_ERR_NO_KING;

	chi_material (pos) = material;

	pos->w_pieces = pos->w_pawns | pos->w_knights | pos->w_bishops | 
	pos->w_rooks | pos->w_kings;
	pos->b_pieces = pos->b_pawns | pos->b_knights | pos->b_bishops | 
	pos->b_rooks | pos->b_kings;

	*end_ptr = ptr;

	*argpos = *pos;

	return 0;
}
