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
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "assure.h"
#include "error.h"

#include "libchi.h"
#include "display-board.h"
#include "log.h"
#include "stringbuf.h"
#include "tateplay-engine.h"
#include "tateplay-game.h"
#include "util.h"
#include "xboard-feature.h"
#include "xmalloca-debug.h"

static void engine_spool_output(Engine *self, const char *buf,
                                void (*callback) (Engine *self));
static bool engine_handle_input(Engine *self, char *buf, ssize_t nbytes);
static bool engine_process_input(Engine *self, const char *line);
static bool engine_process_input_negotiating(Engine *self, const char *line);
static bool engine_process_input_acknowledged(Engine *self, const char *line);
static bool engine_process_input_configuring(Engine *self, const char *line);
static bool engine_process_input_thinking(Engine *self, const char *line);
static bool engine_process_input_pondering(Engine *self, const char *line);

static bool engine_process_xboard_features(Engine *self, const char *line);
static bool engine_process_uci_option(Engine *self, const char *line);
static bool engine_process_uci_id(Engine *self, const char *line);
static bool engine_configure(Engine *self);
static void engine_configure_option(Engine *self, chi_stringbuf *sb,
                                    const UserOption *option);

static void engine_check_string_option(Engine *self, Option *option,
                                       const UserOption *user_option);
static void engine_check_spin_option(Engine *self, Option *option,
                                     const UserOption *user_option);
static void engine_check_check_option(Engine *self, Option *option,
                                      const UserOption *user_option);
static void engine_check_combo_option(Engine *self, Option *option,
                                      const UserOption *user_option);

/* Callbacks.  */
static void engine_start_initial_timeout(Engine *self);
static void engine_start_clock(Engine *self);
static void engine_state_ready(Engine *self);

Engine *
engine_new(Game *game)
{
	Engine *self = xmalloc(sizeof *self);
	memset(self, 0, sizeof *self);

	self->_argv_size = 1;
	self->argv = xmalloc(sizeof self->argv[0]);
	self->argv[0] = NULL;

	self->tc.moves_per_time_control = 40;
	self->tc.seconds_per_time_control = 15 * 60;
	self->tc.increment = 0;

	self->game = game;

	return self;
}

void
engine_destroy(Engine *self)
{
	if (self == NULL) return;

	while (self->pid) {
		int status;

		log_realm(self->nick, "waiting for engine to terminate");
		kill(self->pid, SIGTERM);
		sleep(1);
		status = waitpid(self->pid, NULL, WNOHANG);
		if (status > 0) {
			log_realm(self->nick, "terminated");
			break;
		}
		if (status < 0) {
			error(EXIT_SUCCESS, errno, "waitpid error");
			break;
		}

		kill(self->pid, SIGQUIT);
		sleep(1);
		status = waitpid(self->pid, NULL, WNOHANG);
		if (status > 0) {
			log_realm(self->nick, "terminated");
			break;
		}
		if (status < 0) {
			error(EXIT_SUCCESS, errno, "waitpid error");
			break;
		}

		log_realm(self->nick, "giving up waiting, sending SIGKILL");
		kill(self->pid, SIGKILL);
		(void) waitpid(self->pid, NULL, WNOHANG);
	}

	if (self->user_options) {
		size_t i;
		for (i = 0; i < self->num_user_options; ++i) {
			UserOption option = self->user_options[i];
			free(option.name);
			free(option.value);
		}
		free(self->user_options);
	}

	if (self->options) {
		size_t i;
		for (i = 0; i < self->num_options; ++i) {
			Option *option = self->options[i];
			option_destroy(option);
		}
		free(self->options);
	}

	if (self->argv) free(self->argv);
	if (self->nick) free(self->nick);
	if (self->outbuf) free(self->outbuf);
	if (self->inbuf) free(self->inbuf);

	free(self);
}

void
engine_add_argv(Engine *self, const char *arg)
{
	if (!self->nick && self->_argv_size < 2)
		self->nick = xstrdup(arg);
	self->argv = xrealloc(self->argv,
	                      ++self->_argv_size * sizeof self->argv[0]);
	self->argv[self->_argv_size - 2] = xstrdup(arg);
	self->argv[self->_argv_size - 1] = NULL;
}

void
engine_set_protocol(Engine *self, EngineProtocol protocol)
{
	self->protocol = protocol;
}

bool
engine_start(Engine *self)
{
	pid_t pid;
	int in[2], out[2], err[2];
	int flags;

	assure(self);
	assure(!self->state);

	if (pipe(in) < 0 || pipe(out) < 0 || pipe(err) < 0)
		error(EXIT_FAILURE, errno, "cannot create pipe");

	pid = fork();
	if (pid < 0) return false;
	if (pid > 0) {
		self->pid = pid;
		if (close(in[0]) != 0)
			error(EXIT_FAILURE, errno, "cannot close pipe");
		self->in = in[1];
		if (close(out[1]) != 0)
			error(EXIT_FAILURE, errno, "cannot close pipe");
		self->out = out[0];
		if (close(err[1]) != 0)
			error(EXIT_FAILURE, errno, "cannot close pipe");
		self->err = err[0];

		/* Make I/O non-blocking.  */
		flags = fcntl(self->in, F_GETFL, 0);
		fcntl(self->in, F_SETFL, flags | O_NONBLOCK);
		flags = fcntl(self->out, F_GETFL, 0);
		fcntl(self->out, F_SETFL, flags | O_NONBLOCK);
		flags = fcntl(self->err, F_GETFL, 0);
		fcntl(self->err, F_SETFL, flags | O_NONBLOCK);

		self->state = started;

		return true;
	}

	/* Child process.  First duplicate the pipes to our standard file
	 * descriptors.
	 */
	if (close(STDIN_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot close child stdin");
	if (dup2(in[0], STDIN_FILENO) == -1)
		error(EXIT_FAILURE, errno, "cannot dup pipe");
	if (close(in[0]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");

	if (close(STDOUT_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot close child stdout");
	if (dup2(out[1], STDOUT_FILENO) == -1)
		error(errno, errno, "cannot dup pipe");
	if (close(out[1]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");

	if (close(STDERR_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot close child stderr");
	if (dup2(err[1], STDERR_FILENO) == -1)
		exit(1);
	if (close(err[1]) != 0)
		exit(1);

	/* Now close the unused pipe ends.  */
	if (close(in[1]) != 0)
		exit(1);
	if (close(out[0]) != 0)
		exit(1);
	if (close(err[0]) != 0)
		exit(1);
	
	execvp(self->argv[0], self->argv);
	error(EXIT_FAILURE, errno, "error starting '%s", self->argv[0]);

	exit(EXIT_FAILURE); /* NOTREACHED! Shut up compiler warning.  */
}

void
engine_negotiate(Engine *self)
{
	switch (self->protocol) {
		case xboard:
			engine_spool_output(self, "xboard\nprotover 2\n",
			                    engine_start_initial_timeout);
			self->state = negotiating;
			break;
		case uci:
			engine_spool_output(self, "uci\n", NULL);

			/* Xboard waits for one hour for the done=0 feature.  Do the same
			 * for UCI.
			 */
			self->max_waiting_time = 3600UL * 1000000UL;
			self->state = acknowledged;
			break;
	}
}

bool
engine_read_stdout(Engine *self)
{
#define BUFSIZE 4096
	char buf[BUFSIZE + 1];
	ssize_t nbytes;

	nbytes = read(self->out, buf, BUFSIZE);
	if (nbytes < 0) {
		error(EXIT_SUCCESS, errno,
		      "error reading from standard output of engine '%s'",
		      self->nick);
		return false;
	} else if (nbytes == 0) {
		error(EXIT_SUCCESS, 0,
		      "unexpected end of file while reading from engine '%s'",
			  self->nick);
		return false;
	}

	/* Terminate the buffer.  */
	buf[nbytes] = '\0';

	return engine_handle_input(self, buf, nbytes);
}

bool
engine_read_stderr(Engine *self)
{
#define BUFSIZE 4096
	char buf[BUFSIZE + 1];
	ssize_t nbytes;
	char *start;
	char *end;

	nbytes = read(self->err, buf, BUFSIZE);
	if (nbytes < 0) {
		error(EXIT_SUCCESS, errno,
		      "error reading stderr from engine '%s'",
		      self->nick);
		return false;
	} else if (nbytes == 0) {
		error(EXIT_SUCCESS, 0,
		      "unexpected end of file reading stderr from engine '%s'",
			  self->nick);
		return false;
	}

	/* Make sure that buf is null-terminated!  */
	buf[nbytes] = '\0';

	/* Now handle lines, one by one.  */
	start = end = buf;

	/* We accept LF, CRLF, LFCR, and CR as line endings.  */
	while (*end) {
		char *line = NULL;
		if (*end == '\012') {
			*end = '\0';
			if (end[1] == '\015') ++end;
			line = start;
			start = end + 1;
		} else if (*end == '\015') {
			*end = '\0';
			if (end[1] == '\012') ++end;
			line = start;
			start = end + 1;
		}

		++end;

		if (line) {
			log_engine_error(self->nick, "%s", line);
		}
	}

	if (*start) {
		log_engine_error(self->nick, "%s", start);
	}

	return true;
}

bool
engine_write_stdin(Engine *self)
{
	char *start;
	char *end;

	ssize_t nbytes = write(self->in, self->outbuf, strlen(self->outbuf));
	if (nbytes < 0) {
		error(EXIT_SUCCESS, errno,
		      "error writing to stdin of engine '%s'",
		      self->nick);
		return false;
	} else if (nbytes == 0) {
		error(EXIT_SUCCESS, 0,
		      "unexpected end of file writing to stdin of engine '%s'",
			  self->nick);
		return false;
	}

	/* Now handle lines, one by one.  */
	start = end = self->outbuf;

	while (*end) {
		char *line = NULL;
		if (*end == '\n') {
			*end = '\0';
			line = start;
			start = end + 1;
		}

		++end;

		if (line) {
			log_engine_in(self->nick, "%s", line);
		}
	}

	if (*start) {
		log_engine_error(self->nick, "%s [...]", start);
	}

	if (*start) {
		memmove(self->outbuf, start, strlen(start));
	} else {
		self->outbuf[0] = '\0';
		if (self->out_callback) {
			self->out_callback(self);
			self->out_callback = NULL;
		}
	}

	return true;
}

void
engine_think(Engine *self, chi_pos *pos, chi_move move)
{
	char *movestr = NULL;
	unsigned int buflen;
	int errnum;
	char *fen;
	chi_stringbuf* sb = _chi_stringbuf_new(200);

	self->state = thinking;

	if (verbose && !move) {
		fen = chi_fen(pos);
		log_engine_error(self->nick, "FEN: %s\n", fen);
		display_board(stderr, pos);
		chi_free(fen);
	}

	if (self->protocol == xboard) {
		if (move) {
			if (self->san) {
				errnum = chi_print_move(pos, move, &movestr, &buflen, 1);
			} else {
				errnum = chi_coordinate_notation(move, chi_on_move(pos),
												&movestr, &buflen);
			}
			if (errnum) {
				log_engine_fatal(self->nick, 
								"error generating move string: %s",
								chi_strerror(errnum));
			}
			if (self->xboard_usermove)
				_chi_stringbuf_append(sb, "usermove ");
			
			_chi_stringbuf_append(sb, movestr);
			_chi_stringbuf_append_char(sb, '\n');
		} else {
			_chi_stringbuf_append(sb, "go\n");
		}
	} else {
		/* UCI.  First apply the move to the position.  */
		if (move) {
			errnum = chi_apply_move(pos, move);
			if (errnum) {
				log_engine_fatal(self->nick, 
				                 "error applying move: %s",
				                 chi_strerror(errnum));
			}
		}
		fen = chi_fen(pos);
		_chi_stringbuf_append(sb, "position fen ");
		_chi_stringbuf_append(sb, fen);
		_chi_stringbuf_append_char(sb, '\n');
		_chi_stringbuf_append(sb, "go");

		if (self->depth) {
			_chi_stringbuf_append(sb, " depth ");
			_chi_stringbuf_append_unsigned(sb, self->depth, 10);
			_chi_stringbuf_append_char(sb, '\n');
		}

		_chi_stringbuf_append_char(sb, '\n');
		chi_free(fen);
	}

	engine_spool_output(self, _chi_stringbuf_get_string(sb),
	                    engine_start_clock);
	_chi_stringbuf_destroy(sb);
}

void
engine_ponder(Engine *self, chi_pos *pos)
{
	self->state = pondering;
}

void
engine_set_option(Engine *self, char *name, char *value)
{
	UserOption option;

	++self->num_user_options;
	self->user_options = xrealloc(self->user_options,
	                              self->num_user_options
	                              * sizeof self->user_options[0]);
	option.name = name;
	option.value = value;
	self->user_options[self->num_user_options - 1] = option;
}

chi_bool
engine_stop_clock(Engine *self)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	return time_control_stop_thinking(&self->tc, &now);
}

static void
engine_spool_output(Engine *self, const char *cmd,
                    void (*callback)(Engine * self))
{
	size_t len = strlen(cmd);
	size_t required = 1 + len;

	assure(self->outbuf == NULL || !(*self->outbuf));

	if (required > self->outbuf_size) {
		self->outbuf_size = required;
		self->outbuf = xrealloc(self->outbuf, self->outbuf_size);
	}
	
	strcpy(self->outbuf, cmd);

	self->out_callback = callback;
}

static bool
engine_handle_input(Engine *self, char *buf, ssize_t nbytes)
{
	char *start;
	char *end;
	chi_bool status = true;
	size_t consumed = 0;
	size_t required;

	/* Copy it into the input buffer.  */
	required = self->inbuf_length + nbytes + 1;
	if (required > self->inbuf_size) {
		self->inbuf_size = required;
		self->inbuf = xrealloc(self->inbuf, self->inbuf_size);
	}
	
	strcpy(self->inbuf + self->inbuf_length, buf);
	self->inbuf_length += nbytes;

	/* Now handle lines, one by one.  */
	start = end = self->inbuf;

	/* We accept LF, CRLF, LFCR, and CR as line endings.  */
	char *eob = self->inbuf + self->inbuf_length;
	assure(!*eob);
	while (end < eob) {
		char *line = NULL;
		if (*end == '\012') {
			*end = '\0';
			if (end[1] == '\015') ++end;
			line = start;
			start = end + 1;
		} else if (*end == '\015') {
			*end = '\0';
			if (end[1] == '\012') ++end;
			line = start;
			start = end + 1;
		}

		++end;

		if (line) {
			consumed += strlen(line) + 1;
			log_engine_out(self->nick, "%s", line);
			if (!engine_process_input(self, line)) {
				status = false;
				break;
			}
			line = NULL;
		}
	}

	if (consumed < nbytes) {
		self->inbuf_length = nbytes - consumed;
		memmove(self->inbuf, start, self->inbuf_length);
	} else {
		self->inbuf_length = 0;
	}

	return status;
}

static bool
engine_process_input(Engine *self, const char *cmd)
{
	chi_bool status;

	assure(self->state > started);

	cmd = ltrim(cmd);

	switch (self->state) {
		case negotiating:
			status = engine_process_input_negotiating(self, cmd);
			break;
		case acknowledged:
			status = engine_process_input_acknowledged(self, cmd);
			break;
		case configuring:
			status = engine_process_input_configuring(self, cmd);
			break;
		case ready:
			status = true;
			break;
		case thinking:
			status = engine_process_input_thinking(self, cmd);
			break;
		case pondering:
			status = engine_process_input_pondering(self, cmd);
			break;
		default:
			error(EXIT_SUCCESS, 0,
			      "engine '%s' in unhandled state %d.\n",
			      self->nick, self->state);
			status = false;
			break;
	}

	return status;
}

static bool
engine_process_input_negotiating(Engine *self, const char *cmd)
{
	if (self->protocol == xboard) {
		if (strncmp("feature", cmd, 7) == 0 && isspace(cmd[7])) {
				self->state = acknowledged;
			/* We consider ourselves a server and set the default
			 * to true.
			 */
			self->xboard_name = chi_true;

			return engine_process_input_acknowledged(self, cmd);
		}
	}

	return true;
}

static bool
engine_process_input_acknowledged(Engine *self, const char *cmd)
{
	if (self->protocol == xboard) {
		if (!(strncmp("feature", cmd, 7) == 0 && isspace(cmd[7]))) {
			return true;
		}

		return engine_process_xboard_features(self, cmd + 7);
	} else if (self->protocol == uci) {
		if (strncmp("option", cmd, 6) == 0 && isspace(cmd[6])) {
			return engine_process_uci_option(self, cmd + 6);
		} else if (strncmp("id", cmd, 2) == 0 && isspace(cmd[2])) {
			return engine_process_uci_id(self, cmd + 2);
		} else if (strncmp("uciok", cmd, 5) == 0
		           && ('\0' == cmd[5] || isspace(cmd[5]))) {
			self->max_waiting_time = 0UL;
			engine_configure(self);
			return true;
		}
	}

	return true;
}

static bool
engine_process_input_configuring(Engine *self, const char *cmd)
{
	if (self->protocol == uci) {
		if (strncmp("readyok", cmd, 7) == 0
		           && ('\0' == cmd[7] || isspace(cmd[7]))) {
			self->state = ready;
		}
	}

	return true;
}

static bool
engine_process_input_thinking(Engine *self, const char *cmd)
{
	const char *token;
	/* This cast saves an unnecessary strdup.  The command buffer is
	 * already dynamically allocated.
	 */
	char *endptr = (char *) cmd;

	token = strsep(&endptr, " \t\v\f");
	if (!token)
		return true;

	if (self->protocol == xboard) {
		if (strcmp("move", token) == 0) {
			token = strsep(&endptr, " \t\v\f");
			if (!token)
				return true;
			
			return game_do_move(self->game, token);
		}
	} else {
		if (strcmp("bestmove", token) == 0) {
			token = strsep(&endptr, " \t\v\f");
			if (!token)
				return true;
			
			return game_do_move(self->game, token);
		}
	}

	return true;
}

static bool
engine_process_input_pondering(Engine *self, const char *cmd)
{
	return true;
}

static bool
engine_process_xboard_features(Engine *self, const char *cmd)
{
	const char *endptr = cmd;
	XboardFeature *feature;

	while (*endptr) {
		feature = xboard_feature_new(endptr, &endptr);
		if (!feature)
			continue;
		if (strcmp("myname", feature->name) == 0) {
			log_realm(self->nick, "engine now known as '%s'",
			          feature->value);
			free(self->nick);
			self->nick = xstrdup(feature->value);
		} else if (strcmp("done", feature->name) == 0) {
			if (strcmp("0", feature->value) == 0) {
				gettimeofday(&self->waiting_since, NULL);
				log_realm(self->nick, "now waiting up to one"
				                      " one hour for engine"
				                      " to be ready");
				self->max_waiting_time = 3600UL * 1000000UL;
			} else {
				self->max_waiting_time = 0UL;
				engine_configure(self);
				return true;
			}
		} else if (strcmp("usermove", feature->name) == 0) {
			if (strcmp("0", feature->value) == 0) {
				self->xboard_usermove = chi_false;
			} else {
				self->xboard_usermove = chi_true;
			}
		}

		xboard_feature_destroy(feature);
	}

	return true;
}

static bool
engine_process_uci_option(Engine *self, const char *line)
{
	Option *option = option_uci_new(line);

	if (!option) {
		log_engine_error(self->nick, "unable to parse UCI option: %s",
		                 line);
		return true;
	}

	self->options = xrealloc(self->options,
	                         (self->num_options + 1)
	                          * sizeof self->options[0]);
	self->options[self->num_options++] = option;

	return true;
}

static bool
engine_process_uci_id(Engine *self, const char *line)
{
	const char *start = ltrim(line);
	char *name;

	/* We are only interested in the engine name, not the author.  */
	if (!(strncmp("name", start, 4) == 0 && isspace(start[4])))
		return true;

	name = xstrdup(ltrim(start + 5));
	name = trim(name);

	log_realm(self->nick, "engine now known as '%s'", name);
	free(self->nick);
	self->nick = name;

	return true;
}

static void
engine_start_initial_timeout(Engine *self)
{
	gettimeofday(&self->waiting_since, NULL);
	self->max_waiting_time = 2UL * 1000000;

	log_realm(self->nick,
	          "wait at most two seconds for engine being ready.");
}

static void
engine_start_clock(Engine *self)
{
	struct timeval now;

	gettimeofday(&now, NULL);

	time_control_start_thinking(&self->tc, &now);
}

static void
engine_state_ready(Engine *self)
{
	self->state = ready;
}

static bool
engine_configure(Engine *self)
{
	chi_stringbuf *sb = _chi_stringbuf_new(128);
	const char *commands;
	size_t i;

	log_realm(self->nick, "starting configuration");
	self->state = configuring;

	if (self->protocol == xboard) {
		if (self->depth) {
			_chi_stringbuf_append(sb, "sd ");
			_chi_stringbuf_append_unsigned(sb, self->depth, 10);
			_chi_stringbuf_append_char(sb, '\n');
		}

		if (self->user_options) {
			error(EXIT_FAILURE, 0, "%s: xboard engines do not support\
  options.  Use --config-COLOR instead?\n", self->nick);
		}

		commands = _chi_stringbuf_get_string(sb);
		if (*commands) {
			engine_spool_output(self, _chi_stringbuf_get_string(sb),
			                    engine_state_ready);			
		} else {
			self->state = ready;
		}
		_chi_stringbuf_destroy(sb);
	} else if (self->protocol == uci) {
		for (i = 0; self->user_options && i < self->num_user_options; ++i) {
			engine_configure_option(self, sb, self->user_options + i);
		}
		_chi_stringbuf_append(sb, "ucinewgame\nisready\n");
		engine_spool_output(self, _chi_stringbuf_get_string(sb), NULL);
		_chi_stringbuf_destroy(sb);
	}

	return true;
}

static void
engine_configure_option(Engine *self, chi_stringbuf *sb,
                        const UserOption *user_option)
{
	size_t i;
	Option *option = NULL;

	for (i = 0; i < self->num_options; ++i) {
		if (strcmp(user_option->name, self->options[i]->name) == 0) {
			option = self->options[i];
			break;
		}
	}

	if (option == NULL) {
		error(EXIT_FAILURE, 0, "%s: engine does not support option '%s'.",
		      self->nick, user_option->name);
	}

	switch(option->type) {
		case option_type_button:
			_chi_stringbuf_append(sb, "setoption name ");
			_chi_stringbuf_append(sb, option->name);
			_chi_stringbuf_append_char(sb, '\n');
			break;
		case option_type_string:
			engine_check_string_option(self, option, user_option);
			_chi_stringbuf_append(sb, "setoption name ");
			_chi_stringbuf_append(sb, user_option->name);
			_chi_stringbuf_append_char(sb, ' ');
			_chi_stringbuf_append(sb, user_option->value);
			_chi_stringbuf_append_char(sb, '\n');
			break;
		case option_type_spin:
			engine_check_spin_option(self, option, user_option);
			_chi_stringbuf_append(sb, "setoption name ");
			_chi_stringbuf_append(sb, user_option->name);
			_chi_stringbuf_append_char(sb, ' ');
			_chi_stringbuf_append(sb, user_option->value);
			_chi_stringbuf_append_char(sb, '\n');
			break;
		case option_type_check:
			engine_check_check_option(self, option, user_option);
			_chi_stringbuf_append(sb, "setoption name ");
			_chi_stringbuf_append(sb, user_option->name);
			_chi_stringbuf_append_char(sb, ' ');
			_chi_stringbuf_append(sb, user_option->value);
			_chi_stringbuf_append_char(sb, '\n');
			break;
		case option_type_combo:
			engine_check_combo_option(self, option, user_option);
			_chi_stringbuf_append(sb, "setoption name ");
			_chi_stringbuf_append(sb, user_option->name);
			_chi_stringbuf_append_char(sb, ' ');
			_chi_stringbuf_append(sb, user_option->value);
			_chi_stringbuf_append_char(sb, '\n');
			break;
	}
}

static void
engine_check_string_option(Engine *self, Option *option,
                           const UserOption *user_option)
{
	double value, min, max;

	if (!(option->min || option->max))
		return;

	if (!parse_double(&value, user_option->value)) {
		error(EXIT_FAILURE, errno,
				"%s: invalid value '%s' for option '%s'",
				self->nick, user_option->value, user_option->name);
	}

	if (option->min) {
		if (!parse_double(&min, option->min)) {
			error(EXIT_FAILURE, errno,
			      "%s: engine gave invalid minimum value '%s' for option '%s'",
			      self->nick, option->min, user_option->name);
		}
		if (value < min) {
			error(EXIT_FAILURE, errno,
					"%s: minimum value for option '%s' is %s.",
					self->nick, user_option->name, option->min);
		}
	}

	if (option->max) {
		if (!parse_double(&max, option->max)) {
			error(EXIT_FAILURE, errno,
			      "%s: engine gave invalid maximum value '%s' for option '%s'",
			      self->nick, option->min, user_option->name);
		}
		if (value > max) {
			error(EXIT_FAILURE, errno,
					"%s: maximum value for option '%s' is %s.",
					self->nick, user_option->name, option->max);
		}
	}
}

static void
engine_check_spin_option(Engine *self, Option *option,
                         const UserOption *user_option)
{
	long value, min, max;

	if (!(option->min || option->max))
		return;

	if (!parse_integer(&value, user_option->value)) {
		error(EXIT_FAILURE, errno,
				"%s: invalid value '%s' for option '%s'",
				self->nick, user_option->value, user_option->name);
	}

	if (option->min) {
		if (!parse_integer(&min, option->min)) {
			error(EXIT_FAILURE, errno,
			      "%s: engine gave invalid minimum value '%s' for option '%s'",
			      self->nick, option->min, user_option->name);
		}
		if (value < min) {
			error(EXIT_FAILURE, errno,
					"%s: minimum value for option '%s' is %s.",
					self->nick, user_option->name, option->min);
		}
	}

	if (option->max) {
		if (!parse_integer(&max, option->max)) {
			error(EXIT_FAILURE, errno,
			      "%s: engine gave invalid maximum value '%s' for option '%s'",
			      self->nick, option->min, user_option->name);
		}
		if (value > max) {
			error(EXIT_FAILURE, errno,
					"%s: maximum value for option '%s' is %s.",
					self->nick, user_option->name, option->max);
		}
	}
}

static void
engine_check_check_option(Engine *self, Option *option,
                          const UserOption *user_option)
{
	if (strcmp(user_option->value, "true") == 0
	    || strcmp(user_option->value, "false") == 0)
		return;

	error(EXIT_FAILURE, 0, "%s: value for option '%s' must be 'true' or 'false'",
	      self->nick, user_option->name);
}

static void
engine_check_combo_option(Engine *self, Option *option,
                          const UserOption *user_option)
{
	size_t i;

	for (i = 0; i < option->num_vars; ++i) {
		if (strcmp(user_option->value, option->vars[i]) == 0)
			return;
	}

	error(EXIT_SUCCESS, 0, "%s: value for option '%s' must be one of:",
	      self->nick, user_option->name);

	for (i = 0; i < option->num_vars; ++i) {
		fprintf(stderr, "\t%s\n", option->vars[i]);
	}

	exit(EXIT_FAILURE);
}