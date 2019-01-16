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

#include <libchi.h>

bitv64
chi_zk_update_signature (chi_zk_handle zk_handle, bitv64 signature,
                         chi_move move, chi_color_t color)
{
    chi_piece_t attacker = chi_move_attacker (move);
    chi_piece_t victim = chi_move_victim (move);
    chi_piece_t promote = chi_move_promote (move);
    int is_ep = chi_move_is_ep (move);
    int from = chi_move_from (move);
    int to = chi_move_to (move);

    if (color != chi_white)
	color = chi_black;

    signature ^= chi_zk_lookup (zk_handle, attacker, color, from) ^
	chi_zk_lookup (zk_handle, attacker, color, to);

    if (promote) {
	signature ^= chi_zk_lookup (zk_handle, attacker, color, to) ^
	    chi_zk_lookup (zk_handle, promote, color, to);
    }

    if (victim) {
	if (is_ep) {
	    if (color == chi_white)
		to -= 8;
	    else
		to += 8;
	}
	signature ^= chi_zk_lookup (zk_handle, victim, !color, to);
    } else if (attacker == king) {
	if (color == chi_white && from == 3) {
	    if (to == 1) {
		signature ^= chi_zk_lookup (zk_handle, rook, color, 0) ^
		    chi_zk_lookup (zk_handle, rook, color, 2);
	    } else if (to == 5) {
		signature ^= chi_zk_lookup (zk_handle, rook, color, 7) ^
		    chi_zk_lookup (zk_handle, rook, color, 4);
	    }
	} else if (color == chi_black && from == 59) {
	    if (to == 57) {
		signature ^= chi_zk_lookup (zk_handle, rook, color, 56) ^
		    chi_zk_lookup (zk_handle, rook, color, 58);
	    } else if (to == 61) {
		signature ^= chi_zk_lookup (zk_handle, rook, color, 63) ^
		    chi_zk_lookup (zk_handle, rook, color, 60);
	    }
	}
    }

#define ZK_ARRAY_SIZE (((king + 1) * 2 * 64) + 1)
    signature ^= zk_handle[ZK_ARRAY_SIZE - 1];

    return signature;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
