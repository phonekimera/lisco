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
#include <stdlib.h>

#include <libchi.h>

int
chi_parse_epd (epd, epdstr)
     chi_epd_pos* epd;
     const char* epdstr;
{
    const char* ptr = epdstr;
    int ep_file = -1;
    const char* end_ptr;
    chi_pos tmp_pos;
    chi_pos* pos = &tmp_pos;
    int errnum;
    int avoid;
    chi_move solution;
    char* id = NULL;

    if (!epd || !epdstr)
	return CHI_ERR_YOUR_FAULT;

    memset (epd, 0, sizeof *epd);

    errnum = chi_parse_fen_position (pos, epdstr, &end_ptr);

    if (errnum)
	return errnum;

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
		chi_wk_castle (pos) = 1;
		break;
	    case 'Q':
		chi_wq_castle (pos) = 1;
		break;
	    case 'k':
		chi_bk_castle (pos) = 1;
		break;
	    case 'q':
		chi_bq_castle (pos) = 1;
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

	chi_ep (pos) = 1;
	chi_ep_file (pos) = ep_file;
    }

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;

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

    /* Best move or avoid move?  */
    if (*ptr == 'b' || *ptr == 'B')
	avoid = 0;
    else if (*ptr == 'a' || *ptr == 'A')
	avoid = 1;
    else
	return CHI_ERR_ILLEGAL_EPD;
    ++ptr;

    if (*ptr != 'm' && *ptr != 'M')
	return CHI_ERR_ILLEGAL_EPD;
    ++ptr;

    /* Parse solution.  */
    while (*ptr == ' ' || *ptr == '\t')
	++ptr;
    errnum = chi_parse_move (pos, &solution, ptr);
    if (errnum)
	return errnum;

    while (*ptr) {
	int is_id_string;
	const char* qstart;

	while ((*ptr != ';') && *ptr)
	    ++ptr;
	if (!*ptr)
	    break;
	++ptr;

	while (*ptr == ' ' || *ptr == '\t')
	    ++ptr;
	if (!*ptr)
	    break;

	is_id_string = 0;

	if ((ptr[0] && (ptr[0] == 'i' || ptr[0] == 'I')) && 
	    (ptr[1] && (ptr[1] == 'd' || ptr[1] == 'D')))
	    is_id_string = 1;

	while (*ptr && (*ptr != ' ') && (*ptr != '\t'))
	    ++ptr;
	if (!*ptr)
	    break;

	while (*ptr && (*ptr == ' ' || *ptr == '\t'))
	    ++ptr;
	if (!*ptr)
	    break;

	if (*ptr != '\"')
	    continue;
	++ptr;

	qstart = ptr;

	while (*ptr && *ptr != '"')
	    ++ptr;
	if (!*ptr)
	    break;

	if (is_id_string && !id) {
	    id = strdup (qstart);
	    if (!id)
		return CHI_ERR_ENOMEM;
	    id[ptr - qstart] = '\0';
	}
    }

    epd->id = id;
    epd->pos = *pos;
    epd->avoid = avoid;
    epd->solution = solution;

    return 0;
}

void
chi_free_epd (epd)
     chi_epd_pos* epd;
{
    if (epd && epd->id) {
	free (epd->id);
    }
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
