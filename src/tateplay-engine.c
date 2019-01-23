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
#include "xmalloca.h"

#include "tateplay-engine.h"
#include "log.h"

static void engine_spool_output(Engine *self, const char *buf);
static bool engine_handle_input(Engine *self, char *buf, ssize_t nbytes);

Engine *
engine_new()
{
	Engine *self = xmalloc(sizeof *self);
	memset(self, 0, sizeof *self);

	self->_argv_size = 1;
	self->argv = xmalloc(sizeof self->argv[0]);
	self->argv[0] = NULL;

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
engine_init_protocol(Engine *self)
{
	switch (self->protocol) {
		case xboard:
			engine_spool_output(self, "xboard\nprotover 2\n");
			break;
		case uci:
			engine_spool_output(self, "uci\n");
			break;
	}

	self->state = initialize_protocol;
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
		      "error reading from standard input of engine '%s'",
		      self->nick);
		return false;
	} else if (nbytes == 0) {
		error(EXIT_SUCCESS, 0,
		      "unexpected end of file while reading from engine '%s'",
			  self->nick);
		return false;
	}

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

	ssize_t nbytes = write(self->in, self->outbuf, self->outbuf_length);
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

	self->outbuf_length = strlen(start);
	memmove(self->outbuf, start, self->outbuf_length);

	return true;
}

static void
engine_spool_output(Engine *self, const char *cmd)
{
	/* FIXME! Maybe it's a better strategy to only allow one command (or
	 * sequence of commands like "xboard/protover 2/") so that we can
	 * register a callback, when the output buffer is emptied.
	 */
	size_t len = strlen(cmd);
	size_t required = 1 + self->outbuf_length + len;
	if (required > self->outbuf_size) {
		self->outbuf_size = required;
		self->outbuf = xrealloc(self->outbuf, self->outbuf_size);
	}
	
	strcpy(self->outbuf + self->outbuf_length, cmd);
	self->outbuf_length += len;
	gettimeofday(&self->waiting_since, NULL);
	self->max_waiting_time = 2000000;
}

static bool
engine_handle_input(Engine *self, char *buf, ssize_t nbytes)
{
	size_t len;
	size_t required;
	char *start;
	char *end;
	/* Null-terminate the string.  */
	buf[nbytes] = '\0';
	len = strlen(buf);

	/* Copy it into the input buffer.  */
	required = self->inbuf_length + len + 1;
	if (required > self->inbuf_size) {
		self->inbuf_size = required;
		self->inbuf = xrealloc(self->inbuf, self->inbuf_size);
	}
	
	strcpy(self->inbuf + self->inbuf_length, buf);

	/* Now handle lines, one by one.  */
	start = end = self->inbuf;

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
			log_engine_out(self->nick, "%s", line);
		}
	}

	memmove(self->inbuf, start, strlen(start));
	self->inbuf_length = strlen(self->inbuf);

	return true;
}