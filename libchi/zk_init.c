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
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <libchi.h>

#define ZK_ARRAY_SIZE (((king + 1) * 2 * 64) + 1)

int
chi_zk_init (result)
     chi_zk_handle* result;
{
    int fd;
    int seeded = 0;
    int i;
    int dev_urandom = 0;

    chi_zk_handle zk_handle = 
	malloc (ZK_ARRAY_SIZE * sizeof *zk_handle);

    *result = zk_handle;

    if (!zk_handle)
	return CHI_ERR_ENOMEM;

    /* Fill the array with random bits.  FIXME: Why does this has to
       be random?  It should be sufficient to generate the random data
       once at compile-time and reuse it every time the library is
       invoked? */
    fd = open ("/dev/urandom", O_RDONLY);
    if (fd != -1) {
	char* ptr = (char*) zk_handle;
	size_t bytes_read = 0;
	size_t wanted = ZK_ARRAY_SIZE * sizeof *zk_handle;

	while (bytes_read < wanted) {
	    int bytes_here = read (fd, ptr + bytes_read, wanted - bytes_read);
	    
	    if (bytes_here == -1)
		break;

	    bytes_read += bytes_here;
	}

	dev_urandom = 1;
	close (fd);
    }

    if (dev_urandom)
	return 0;

    if (!seeded) {
	srandom (time (NULL));
	seeded = 1;
    }
    
    for (i = 0; i < ZK_ARRAY_SIZE; ++i) {
	bitv64 number = (bitv64) random ();
	
	/* Not perfectly random, since we will never generate 
	   numbers that have the upper 8 bits unset but this
	       takes into account that random () might return only
	       16 bits or whatever.  */
#define ZK_UPPER_8 (((bitv64) 0xff) << 56)
	
	while (!(number & ZK_UPPER_8)) {
	    number <<= 8;
	    number ^= random ();
	}
	zk_handle[i] = number;
    }

    return 0;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
