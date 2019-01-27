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
#include "xmalloca-debug.h"
#include "xstrndup.h"

#include "util.h"
#include "uci-option.h"

static size_t uci_option_consume_tokens(UCIOption *self, char **tokens,
                                        const char *original);
static void uci_option_add_var(UCIOption *self, char *var);

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
		if (skip) {
			value = tokens[i];
			i += skip;
		} else {
			++i;
			value = "";
		}

		if (strcmp("name", keyword) == 0) {
			if (self->name)
				goto bail_out;
			self->name = xstrdup(value);
		} else if (strcmp("type", keyword) == 0) {
			if (self->type)
				goto bail_out;
			if (strcmp("check", value) == 0) {
				self->type = uci_option_type_check;
			} else if (strcmp("spin", value) == 0) {
				self->type = uci_option_type_spin;
			} else if (strcmp("combo", value) == 0) {
				self->type = uci_option_type_combo;
			} else if (strcmp("button", value) == 0) {
				self->type = uci_option_type_button;
			} else if (strcmp("string", value) == 0) {
				self->type = uci_option_type_string;
			} else {
				goto bail_out;
			}
		} else if (strcmp("default", keyword) == 0) {
			if (self->default_value)
				goto bail_out;
			self->default_value = xstrdup(value);
		} else if (strcmp("min", keyword) == 0) {
			if (self->min)
				goto bail_out;
			self->min = xstrdup(value);
		} else if (strcmp("max", keyword) == 0) {
			if (self->max)
				goto bail_out;
			self->max = xstrdup(value);
		} else if (strcmp("var", keyword) == 0) {
			uci_option_add_var(self, value);
		} else {
			goto bail_out;
		}
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
	size_t i;

	if (!self) return;

	if (self->name) free(self->name);
	if (self->default_value) free(self->default_value);
	if (self->min) free(self->min);
	if (self->max) free(self->max);

	if (self->vars) {
		for (i = 0; i < self->num_vars; ++i)
			free(self->vars[i]);
		free(self->max);
	}
	free(self);
}

static size_t
uci_option_consume_tokens(UCIOption *self, char **tokens, const char *original)
{
	size_t i, j;

	for (i = 0; tokens[i] != NULL; ++i) {
		// if (tokens[i][0] == '\0') {
		// 	/* Restore the original character overwriting the
		// 	 * terminating NIL.  This will extend the value.
		// 	 */
		// 	assure(&tokens[i][-1] >= original);
		// 	tokens[i][-1] = original[&tokens[i][-1] - &tokens[0][0]];
		// 	tokens[i][0]  = original[&tokens[i][0] - &tokens[0][0]];
		// 	continue;
		// } else

		if (strcmp("name", tokens[i]) == 0) {
			break;
		} else if (strcmp("type", tokens[i]) == 0) {
			break;
		} else if (strcmp("default", tokens[i]) == 0) {
			break;
		} else if (strcmp("var", tokens[i]) == 0) {
			break;
		} else if (strcmp("min", tokens[i]) == 0) {
			break;
		} else if (strcmp("max", tokens[i]) == 0) {
			break;
		}
	}

	if (i > 1) {
		for (j = 0; j < i - 1; ++j) {
			/* Remove the terminating NIL.  */
			size_t l = strlen(tokens[j]);
			off_t offset = &tokens[j][l] - &tokens[0][0];

			tokens[j][l] = original[offset];
		}
	}

	return i;
}

static void
uci_option_add_var(UCIOption *self, char *var)
{
	++self->num_vars;
	self->vars = xrealloc(self->vars,
	                      self->num_vars * sizeof self->vars[0]);
	self->vars[self->num_vars - 1] = xstrdup(var);
}
