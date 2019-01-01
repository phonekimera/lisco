/* chitest.c - Functions for board manipulation.
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

#include <sys/time.h>
#include <sys/times.h>

#include <libchi.h>

#define COUNT 100000000

int from = 53;
int to = 13;
chi_piece_t promote = queen;
int material = 8;

int
func_int (int count)
{
    int i;
    int retval = 0;
    unsigned int move = 0;

    for (i = 0; i < count; ++i) {
	move = from & 0xff;
	move |= (to & 0xff) << 8;
	move |= (promote & 0xff) << 16;
	move |= (material & 0xff) << 24;

	retval ^= (move & 0xff) ^
	    ((move & 0xff00) >> 8) ^
	    ((move & 0xff0000) >> 16) ^
	    ((move & 0xff000000) >> 24);
    }

    return retval;
}

int
func_union (int count)
{
    int i;
    int retval = 0;
    chi_move move = 0;

    for (i = 0; i < count; ++i) {
	chi_move_set_packed (move, from);
	chi_move_set_to (move, to);
	chi_move_set_promote (move, promote);
	chi_move_set_material (move, material);

	retval ^= chi_move_from (move) ^ chi_move_to (move) ^ chi_move_promote (move) ^ chi_move_material (move);
    }

    return retval;
}

int
main (int argc, char* argv[])
{
    struct tms start, end;

    times (&start);
    func_union (COUNT);
    times (&end);
    printf ("union version: %ld user, %ld system\n",
	    end.tms_utime - start.tms_utime,
	    end.tms_stime - start.tms_stime);

    times (&start);
    func_int (COUNT);
    times (&end);
    printf ("integer version: %ld user, %ld system\n",
	    end.tms_utime - start.tms_utime,
	    end.tms_stime - start.tms_stime);

    return 0;
}
