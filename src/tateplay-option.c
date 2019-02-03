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
#include "tateplay-option.h"

static size_t option_consume_tokens(Option *self, char **tokens,
                                    const char *original);
static void option_add_var(Option *self, char *var);

Option *
option_uci_new(const char *input)
{
	Option *self = xmalloc(sizeof *self);
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

		size_t skip = option_consume_tokens(self, tokens + i,
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
				self->type = option_type_check;
			} else if (strcmp("spin", value) == 0) {
				self->type = option_type_spin;
			} else if (strcmp("combo", value) == 0) {
				self->type = option_type_combo;
			} else if (strcmp("button", value) == 0) {
				self->type = option_type_button;
			} else if (strcmp("string", value) == 0) {
				self->type = option_type_string;
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
			option_add_var(self, value);
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

	option_destroy(self);

	return NULL;
}

Option *
option_xboard_new(const char *input)
{
	Option *self = xmalloc(sizeof *self);
	const char *original = ltrim(input);
	char *copy = xstrdup(original);
	char *start = copy;
	char *end;

	memset(self, 0, sizeof *self);

	start = trim(start);
	end = start + 1;
	while (*end) {
		if (isspace(*end)) {
			if (strcmp("-button", end + 1) == 0) {
				self->type = option_type_button;
				*end = '\0';
				self->name = xstrdup(trim(start));
				break;
			} else if (strcmp("-save", end + 1) == 0) {
				self->type = option_type_button;
				*end = '\0';
				self->name = xstrdup(trim(start));
				break;
			} else if (strcmp("-reset", end + 1) == 0) {
				self->type = option_type_button;
				*end = '\0';
				self->name = xstrdup(trim(start));
				break;
			} else if (strncmp("-check", end + 1, 6) == 0
			           && isspace(end[7])) {
				self->type = option_type_check;
				*end = '\0';
				self->name = xstrdup(trim(start));
				end += 8;
				while (isspace(*end)) { end++; }
				if (end[0] == '0')
					self->default_value = xstrdup("0");
				else if (end[0] == '1')
					self->default_value = xstrdup("1");
				if (end[1] != '\0')
					goto bail_out;
				break;
			} else if (strncmp("-string", end + 1, 7) == 0
			           && isspace(end[8])) {
				self->type = option_type_string;
				*end = '\0';
				self->name = xstrdup(trim(start));
				start = end + 9;
				self->default_value = xstrdup(trim(start));
				break;
			} else if (strncmp("-spin", end + 1, 5) == 0
			           && isspace(end[6])) {
				self->type = option_type_spin;
				*end = '\0';
				self->name = xstrdup(trim(start));

				start = (char *) ltrim(end + 7);
				end = start + 1;
				if (*end == '\0')
					goto bail_out;
				while (*end && !isspace(*end)) { ++end; }
				*end = '\0';
				self->default_value = xstrdup(start);

				start = (char *) ltrim(end + 1);
				end = start + 1;
				while (*end && !isspace(*end)) { ++end; }
				*end = '\0';
				self->min = xstrdup(start);

				start = (char *) ltrim(end + 1);
				end = start + 1;
				while (*end && !isspace(*end)) { ++end; }
				*end = '\0';
				self->max = xstrdup(start);

				if (!*self->default_value || !*self->min || !*self->max)
					goto bail_out;

				break;
			}
		}
		++end;
	}

	if (!self->name || !*self->name)
		goto bail_out;

	free(copy);

	return self;

bail_out:
	free(copy);

	option_destroy(self);

	return NULL;
}

void
option_destroy(Option *self)
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
option_consume_tokens(Option *self, char **tokens, const char *original)
{
	size_t i, j;

	for (i = 0; tokens[i] != NULL; ++i) {
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
option_add_var(Option *self, char *var)
{
	++self->num_vars;
	self->vars = xrealloc(self->vars,
	                      self->num_vars * sizeof self->vars[0]);
	self->vars[self->num_vars - 1] = xstrdup(var);
}
