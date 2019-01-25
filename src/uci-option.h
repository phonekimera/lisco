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

#ifndef _UCI_OPTION_H
# define _UCI_OPTION_H        /* Allow multiple inclusion.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libchi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum UCIOptionType {
	uci_option_type_string,
	uci_option_type_check,
	uci_option_type_spin,
	uci_option_type_button,
	uci_option_type_combo,
} UCIOptionType;

typedef struct UCIOption {
	char *name;
	long long min;
	chi_bool min_set;
	long long max;
	chi_bool max_set;
	const char *vars;
	size_t num_vars;
	union {
		chi_bool boolean_default;
		const char *string_default;
		long long integer_default;
	} defaults;
	chi_bool default_set;
} UCIOption;

extern UCIOption *uci_option_new(const char *input);
extern void uci_option_destroy(UCIOption *option);

#ifdef __cplusplus
}
#endif

#endif
