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

#include <libchi.h>

/* Well-known small primes.  You probably know them by heart.  */
static const unsigned long int small_primes[] = {
	2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61,
	67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137,
	139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
	223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283,
	293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
	383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461,
	463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563,
	569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643,
	647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739,
	743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
	839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937,
	941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021,
	1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093,
	1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181,
	1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259,
	1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
	1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433,
	1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493,
	1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579,
	1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657,
	1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741,
	1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831,
	1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913,
	1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999
};

static const unsigned long int num_small_primes = 303;

static int is_prime(unsigned long int p);

unsigned long int
chi_closest_prime (base)
     unsigned long base;
{
    	int offset;

	if (base == 2)
		return 2;

	/* Make sure, number is odd.  */
    	base |= 1;

	if (is_prime (base))
		return base;

	for (offset = 2; offset < base; offset += 2) {
		if ((base + offset > base) && is_prime (base + offset))
			return base + offset;
		if (is_prime (base - offset))
			return base - offset;
	}

	return 1;	
}

static int
is_prime (p)
     unsigned long p;
{
	unsigned long int i;
	unsigned long int p1;
	unsigned long int b;
	unsigned long int m;

	if (p <= 1)
		return 1;

	for (i = 0; i < num_small_primes; ++i) {
		if (p == small_primes[i])
			return 1;
		if (p % small_primes[i] == 0) {
			return 0;
		}
	}

	/* For the Rabin-Miller test, we have to find b such that 
	   2**b is the largets power of 2 that divides p - 1.  That
	   happens to be the number of bits not set in p - 1 (counted
	   from the least significant side).
	*/
	p1 = p - 1;
	p1 &= -p1;
	--p1;

	for (b = 0; p1 != 0; ++b, p1 &= (p1 - 1));
	
	/* We also need m with p = 1 + 2**b * m.  */
	m = (p - 1) >> b;

	/* Now perform the Rabin-Miller test several times.  See
	   Applied Cryptography by Bruce Schneier, chapter 11.  */
	for (i = 0; i < 100; ++i) {
		unsigned long int a = 0xffff & random ();
		bitv64 z = 1;
		bitv64 t = a;
		unsigned long int u = m;
		unsigned long int j = 0;

		/* z = a**m mod p */
		while (u) {
		        if (u & 1)
				z = ((z % p) * (t % p)) % p;
	        	u >>= 1;
			t = ((t % p) * (t % p)) % p;
		}

		if (z == 1 || z == p - 1)
			continue;    /* Test passed.  */

		while (1) {
			if (j > 0 && z == 1)
				return 0;

			++j;
			if (j < b && z != p - 1) {
				z = (z * z) % p;
				continue;
			}

			if (z == p - 1)
				break;  /* Test passed.  */

			if (j == b && z != p - 1)
				return 0;  /* So sorry, not a prime.  */
		}
	}
	
	return 1;
}

/*
Local Variables:
mode: c
c-style: K&R
c-basic-shift: 8
End:
*/
