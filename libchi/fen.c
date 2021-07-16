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

#include <stdio.h>
#include <string.h>

#include "libchi.h"
#include "stringbuf.h"

char *
chi_fen(const chi_pos *pos)
{
	chi_stringbuf* sb = _chi_stringbuf_new(2);
	bitv64 pieces = pos->w_pieces | pos->b_pieces;
	char *retval;

	for (int rank = CHI_RANK_8; rank >= CHI_RANK_1; --rank) {
		int empty = 0;
		for (int file = CHI_FILE_A; file <= CHI_FILE_H; ++file) {
			int shift = chi_coords2shift(file, rank);
			bitv64 mask = ((bitv64) 1) << shift;

			if (pieces & mask) {
				char p;

				if (empty) {
					_chi_stringbuf_append_char(sb, '0' + empty);
					empty = 0;
				}
				if (mask & pos->w_pawns) {
					p = 'P';
				} else if (mask & pos->b_pawns) {
					p = 'p';
				} else if (mask & pos->w_knights) {
					p = 'N';
				} else if (mask & pos->b_knights) {
					p = 'n';
				} else if (mask & pos->w_bishops) {
					if (mask & pos->w_rooks)
						p = 'Q';
					else
						p = 'B';
				} else if (mask & pos->b_bishops) {
					if (mask & pos->b_rooks)
						p = 'q';
					else
						p = 'b';
				} else if ((mask & pos->w_rooks)
				           && !(mask & pos->w_bishops)) {
					p = 'R';
				} else if ((mask & pos->b_rooks)
				           && !(mask & pos->b_bishops)) {
					p = 'r';
				} else if (mask & pos->w_kings) {
					p = 'K';
				} else {
					p = 'k';
				}
				_chi_stringbuf_append_char(sb, p);
			} else {
				++empty;
			}

			if (file == CHI_FILE_H) {
				if (empty) {
					_chi_stringbuf_append_char(sb, '0' + empty);
				}
				if (rank != CHI_RANK_1) {
					_chi_stringbuf_append_char(sb, '/');
				}
			}
		}
	}

	_chi_stringbuf_append_char(sb, ' ');
	if (chi_white == chi_on_move(pos))
		_chi_stringbuf_append_char(sb, 'w');
	else
		_chi_stringbuf_append_char(sb, 'b');

	_chi_stringbuf_append_char(sb, ' ');
	if (!(chi_wk_castle(pos) || chi_wq_castle(pos)
	      || chi_bk_castle(pos) || chi_bq_castle(pos))) {
		_chi_stringbuf_append_char(sb, '-');
	} else {
		if (chi_wk_castle(pos))
			_chi_stringbuf_append_char(sb, 'K');
		if (chi_wq_castle(pos))
			_chi_stringbuf_append_char(sb, 'Q');
		if (chi_bk_castle(pos))
			_chi_stringbuf_append_char(sb, 'k');
		if (chi_bq_castle(pos))
			_chi_stringbuf_append_char(sb, 'q');
	}

	_chi_stringbuf_append_char(sb, ' ');
	if (chi_ep(pos)) {
		_chi_stringbuf_append_char(sb, chi_file2char(chi_ep_file(pos)));
		_chi_stringbuf_append_char(sb, chi_rank2char(chi_ep_rank(pos)));
	} else {
		_chi_stringbuf_append_char(sb, '-');
	}

	_chi_stringbuf_append_char(sb, ' ');
	_chi_stringbuf_append_unsigned(sb, pos->half_move_clock, 10);

	_chi_stringbuf_append_char(sb, ' ');
	_chi_stringbuf_append_unsigned(sb, 1 + (pos->half_moves >> 1), 10);

	retval = strdup(_chi_stringbuf_get_string(sb));
	_chi_stringbuf_destroy(sb);

	if (!retval) {
		perror("fatal error");
		exit(255);
	}

	return retval;
}
