/*
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

#include <getopt.h>

#include "closeout.h"
#include "error.h"
#include "progname.h"

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' }
};

int
main(int argc, char const *argv[])
{
	set_program_name(argv[0]);

	atexit(close_stdout);
	
	return 0;
}
