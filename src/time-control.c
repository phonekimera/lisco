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

#include "lisco.h"

int
process_search_params(Tree *tree, SearchParams *params)
{
	/* Initial value for calibration.  */
	tree->nodes_to_tc = 10000;

	if (params->movetime) {
		tree->fixed_time = params->movetime;
	} else if (params->nodes) {
		tree->nodes_to_tc = params->nodes;
	}

	return 1;
}