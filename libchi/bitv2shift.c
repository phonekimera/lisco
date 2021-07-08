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

#include <string.h>

#define M1 ((bitv64) 0x5555555555555555)
#define M2 ((bitv64) 0x3333333333333333)

unsigned int 
chi_bitv2shift (b)
     bitv64 b;
{
    unsigned int n;

    bitv64 a = b - 1 - (((b - 1) >> 1) & M1);
    bitv64 c = (a & M2) + ((a >> 2) & M2);
    
    n = ((unsigned int) c) + ((unsigned int) (c >> 32));
    n = (n & 0x0f0f0f0f) + ((n >> 4) & 0x0f0f0f0f);
    n = (n & 0xffff) + (n >> 16);
    n = (n & 0xff) + (n >> 8);
    
    return n;    
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
