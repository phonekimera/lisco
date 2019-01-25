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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "assure.h"
#include "xmalloca.h"
#include "xstrndup.h"

#include "util.h"
#include "uci-option.h"

static size_t uci_option_consume_tokens(UCIOption *self, char **tokens,
                                        const char *original);

UCIOption *
uci_option_new(const char *input)
{
	UCIOption *self = xmalloc(sizeof *self);
	const char *original = ltrim(input);
	char *copy = xstrdup(original);
	char *string = copy;
	char *token;
	char **tokens = NULL;
	size_t num_tokens = 0;
	size_t i;

	memset(self, 0, sizeof *self);

	string = trim(string);
	while ((token = strsep(&string, " \t\v\f"))) {
		tokens = xrealloc(tokens, ++num_tokens * sizeof *tokens);
		tokens[num_tokens - 1] = token;
	}
	tokens = xrealloc(tokens, (num_tokens + 1) * sizeof *tokens);
	tokens[num_tokens] = NULL;

	i = 0;
	while (i < num_tokens) {
		char *keyword = tokens[i++];
		char *value;

		size_t skip = uci_option_consume_tokens(self, tokens + i,
		                                        original
		                                        + (&tokens[i][0]
		                                           - &tokens[0][0]));
		value = tokens[i];

		i += skip;
	}

	if (!self->name)
		goto bail_out;

	free(copy);
	if (tokens)
		free(tokens);

	return self;

bail_out:
	free(copy);
	if (tokens)
		free(tokens);

	uci_option_destroy(self);

	return NULL;
}

void
uci_option_destroy(UCIOption *self)
{
	if (!self) return;

	if (self->name) free(self->name);

	free(self);
}

static size_t
uci_option_consume_tokens(UCIOption *self, char **tokens, const char *original)
{
	size_t i;

	for (i = 0; tokens[i]; ++i) {
		if (tokens[i][0] == '\0') {
			/* Restore the original character overwriting the
			 * terminating NIL.  This will extend the value.
			 */
			assure(&tokens[i][-1] >= original);
			tokens[i][-1] = original[&tokens[i][-1] - &tokens[0][0]];
			tokens[i][0]  = original[&tokens[i][0] - &tokens[0][0]];
			continue;
		}
		if (strcmp("name", tokens[i]) == 0 && !self->name)
			break;
		if (strcmp("type", tokens[i]) == 0 && !self->type)
			break;
		if (strcmp("var", tokens[i]) == 0 && !self->vars)
			break;
		if (strcmp("min", tokens[i]) == 0 && !self->min_set)
			break;
		if (strcmp("max", tokens[i]) == 0 && !self->max_set)
			break;
	}

	return i;
}
