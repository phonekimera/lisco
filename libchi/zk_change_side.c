/* zk_change_side.c - change the side on move in a signature
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

bitv64
chi_zk_change_side (zk_handle, signature)
     chi_zk_handle zk_handle;
     bitv64 signature;
{
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