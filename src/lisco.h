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

#ifndef LISCO_H
# define LISCO_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libchi.h>

#define LISCO_DEFAULT_TT_SIZE 16

#define HASH_UNKNOWN ((unsigned int) 0)
#define HASH_ALPHA   ((unsigned int) 1)
#define HASH_BETA    ((unsigned int) 2)
#define HASH_EXACT   ((unsigned int) 3)

#ifdef __cplusplus
extern "C" {
#endif

extern void init_tt_hashs(unsigned long int memuse);
extern void clear_tt_hashs(void);

#ifdef __cplusplus
extern }
#endif

#endif
