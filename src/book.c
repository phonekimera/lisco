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

#include <stdio.h>
#include <gdbm.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "book.h"

#define BOOK_NAME "book.gdbm"

static void fatal_func(char* error_msg);

void
fatal_func (error_msg)
     char* error_msg;
{
    error (EXIT_FAILURE, 0, "%s", error_msg);
}

void
create_book (filename, maxply, minapp, wpc)
     const char* filename;
     unsigned int maxply;
     unsigned int minapp;
     unsigned int wpc;
{
    GDBM_FILE dbf;

    dbf = gdbm_open (BOOK_NAME, 0, GDBM_WRCREAT | GDBM_NOLOCK, 0, fatal_func);
    if (dbf == NULL) {
	const char* error_msg = gdbm_errno ? gdbm_strerror (gdbm_errno)
	    : strerror (errno);

	fprintf (stdout, "Error (creating database): %s: %s\n",
		 BOOK_NAME, error_msg);
	return;
    }



    gdbm_close (dbf);
}
