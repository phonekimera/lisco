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

#ifndef _TATEPLAY_OPTION_H
# define _TATEPLAY_OPTION_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libchi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum OptionType {
	option_type_string = 1,
	option_type_check,
	option_type_spin,
	option_type_button,
	option_type_combo,
} OptionType;

typedef struct Option {
	char *name;
	OptionType type;
	char *min;
	char *max;
	char **vars;
	size_t num_vars;
	char *default_value;
	chi_bool default_set;
} Option;

extern Option *option_uci_new(const char *input);
extern Option *option_xboard_new(const char *input);
extern void option_destroy(Option *option);

#ifdef __cplusplus
}
#endif

#endif
