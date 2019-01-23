/* This file is part of the chess engine tate.
 *
 * Copyright (C) 2002-2019 cantanea EOOD.
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
#include <string.h>

#include <libchi.h>

int
chi_set_position(chi_pos *argpos, const char *fen)
{
	const char* ptr = fen;
	int ep_file = -1;
	unsigned long num;
	const char* end_ptr;
	char* num_end_ptr;
	chi_pos tmp_pos;
	chi_pos* pos = &tmp_pos;
	int errnum;

	if (!argpos || !fen)
		return CHI_ERR_YOUR_FAULT;

	errnum = chi_parse_fen_position(pos, fen, &end_ptr);

	if (errnum)
		return errnum;

	/* Assume that castling is not possible at all.  */
	pos->lost_wk_castle = 0;
	pos->lost_wq_castle = 0;
	pos->lost_bk_castle = 0;
	pos->lost_bq_castle = 0;

	ptr = end_ptr;

	while (*ptr == ' ' || *ptr == '\t')
		++ptr;

	switch (*ptr) {
		case 'w':
		case 'W':
			chi_on_move (pos) = chi_white;
			break;
		case 'b':
		case 'B':
			chi_on_move (pos) = chi_black;
			break;
		default:
			return CHI_ERR_ILLEGAL_FEN;
	}
	++ptr;

	while (*ptr == ' ' || *ptr == '\t')
		++ptr;

	while (1) {
		switch (*ptr) {
			case 'K':
				chi_wk_castle(pos) = 1;
				pos->lost_wk_castle = 0;
				break;
			case 'Q':
				chi_wq_castle(pos) = 1;
				pos->lost_wq_castle = 0;
				break;
			case 'k':
				chi_bk_castle(pos) = 1;
				pos->lost_bk_castle = 0;
				break;
			case 'q':
				chi_bq_castle(pos) = 1;
				pos->lost_bq_castle = 0;
				break;
			case '-':
				break;
			default:
				return CHI_ERR_ILLEGAL_FEN;
	}

	if (*ptr == '-') {
		++ptr;
		break;
	}
	++ptr;
	if (*ptr == ' ')
		break;
	}

	while (*ptr == ' ' || *ptr == '\t')
		++ptr;

	switch (*ptr) {
		case 'a':
			ep_file = 0;
			break;
		case 'b':
			ep_file = 1;
			break;
		case 'c':
			ep_file = 2;
			break;
		case 'd':
			ep_file = 3;
			break;
		case 'e':
			ep_file = 4;
			break;
		case 'f':
			ep_file = 5;
			break;
		case 'g':
			ep_file = 6;
			break;
		case 'h':
			ep_file = 7;
			break;
		case '-':
			break;
		default:
			return CHI_ERR_ILLEGAL_FEN;
	}

	++ptr;

	if (ep_file >= 0) {
		if (*ptr != '3' && *ptr != '6')
			return CHI_ERR_ILLEGAL_FEN;
		++ptr;

		chi_ep(pos) = 1;
		chi_ep_file(pos) = ep_file;
	}

	while (*ptr == ' ' || *ptr == '\t')
		++ptr;

	num = strtoul (ptr, &num_end_ptr, 10);
	if (num_end_ptr == ptr)
		return CHI_ERR_ILLEGAL_FEN;
	pos->half_move_clock = num;
	ptr = num_end_ptr;

	while (*ptr == ' ' || *ptr == '\t')
		++ptr;

	num = strtoul (ptr, &num_end_ptr, 10);
	if (num_end_ptr == ptr)
		return CHI_ERR_ILLEGAL_FEN;

	if (chi_on_move(pos) == chi_white)
		pos->half_moves = (num - 1) << 1;
	else
		pos->half_moves = ((num - 1) << 1) + 1;

	pos->irreversible[0] = pos->half_moves - pos->half_move_clock;
	pos->irreversible_count = 1;

	if (chi_ep(pos)) {
		pos->ep_files[0] = chi_ep_file(pos);
		pos->double_pawn_moves[0] = pos->half_moves;
		pos->double_pawn_move_count = 1;
	}

	ptr = num_end_ptr;

	while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')
		++ptr;

		/* Trailing garbage? */
	if (*ptr)
		return CHI_ERR_ILLEGAL_FEN;

	/* Side not to move in check? */
	chi_on_move (pos) = !chi_on_move (pos);
	if (chi_check_check (pos))
		return CHI_ERR_IN_CHECK;
	chi_on_move (pos) = !chi_on_move (pos);

	if (chi_wk_castle (pos) || chi_wq_castle (pos)) {
		if (pos->w_kings != (CHI_E_MASK & CHI_1_MASK))
			return CHI_ERR_ILLEGAL_CASTLING_STATE;
		if (chi_wk_castle (pos)) {
			if (!(pos->w_rooks & CHI_H_MASK & CHI_1_MASK))
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
			if (pos->w_bishops & CHI_H_MASK & CHI_1_MASK)
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
		}
		if (chi_wq_castle (pos)) {
			if (!(pos->w_rooks & CHI_A_MASK & CHI_1_MASK))
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
			if (pos->w_bishops & CHI_A_MASK & CHI_1_MASK)
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
		}
	}

	if (chi_bk_castle (pos) || chi_bq_castle (pos)) {
		if (pos->b_kings != (CHI_E_MASK & CHI_8_MASK))
			return CHI_ERR_ILLEGAL_CASTLING_STATE;
		if (chi_bk_castle (pos)) {
			if (!(pos->b_rooks & CHI_H_MASK & CHI_8_MASK))
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
			if (pos->b_bishops & CHI_H_MASK & CHI_8_MASK)
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
		}
		if (chi_bq_castle (pos)) {
			if (!(pos->b_rooks & CHI_A_MASK & CHI_8_MASK))
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
			if (pos->b_bishops & CHI_A_MASK & CHI_8_MASK)
				return CHI_ERR_ILLEGAL_CASTLING_STATE;
		}
	}

	*argpos = *pos;

	return 0;
}
