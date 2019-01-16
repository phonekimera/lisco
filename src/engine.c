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
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "assure.h"
#include "error.h"
#include "xmalloca.h"

#include "engine.h"
#include "log.h"

Engine *
engine_new()
{
	Engine *self = xmalloc(sizeof (Engine));

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
		self->err = out[1];

		return true;
	}

	/* Child process.  First duplicate the pipes to our standard file
	 * descriptors.
	 */
	if (close(STDIN_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot close child stdin");
	if (close(STDOUT_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot close child stdout");
	if (close(STDERR_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot close child stderr");
	if (dup2(in[0], STDIN_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot dup pipe");
	if (close(in[0]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");
	if (dup2(out[1], STDOUT_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot dup pipe");
	if (close(out[1]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");
	if (dup2(err[1], STDERR_FILENO) != 0)
		error(EXIT_FAILURE, errno, "cannot dup pipe");
	if (close(err[1]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");

	/* Now close the unused pipe ends.  */
	if (close(in[1]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");
	if (close(out[0]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");
	if (close(err[0]) != 0)
		error(EXIT_FAILURE, errno, "cannot close pipe");
	
	execvp(self->argv[0], self->argv);
	error(EXIT_FAILURE, errno, "error starting '%s", self->argv[0]);
	exit(EXIT_SUCCESS);
}
