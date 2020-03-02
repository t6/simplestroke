/*
 * Copyright (c) 2019 Tobias Kortkamp <t@tobik.me>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "config.h"

#if HAVE_CAPSICUM
# include <sys/capsicum.h>
# include "capsicum_helpers.h"
#endif
#ifdef __FreeBSD__
# include <sys/sysctl.h>
#endif
#include <assert.h>
#if HAVE_ERR
# include <err.h>
#endif
#include <sys/param.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __FreeBSD__
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

#include "stroke.h"

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

static struct pollfd fds[32];
static size_t nfds;
static int props_by_sysctl = 0;
#if HAVE_CAPSICUM
static int command_runner_fd[2] = { -1, -1 };
#endif

// From libudev-devd's udev-utils.c
#define	LONG_BITS	(sizeof(long) * 8)
#define	NLONGS(x)	(((x) + LONG_BITS - 1) / LONG_BITS)

static inline bool
bit_is_set(const unsigned long *array, int bit)
{
	return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}

static int
open_device(size_t i)
{
	char buf[PATH_MAX];
	unsigned long prp_bits[NLONGS(INPUT_PROP_CNT)];
	size_t len = sizeof(prp_bits);

#ifdef __FreeBSD__
	if (props_by_sysctl) {
		snprintf(buf, sizeof(buf), "kern.evdev.input.%zu.props", i);
		if (sysctlbyname(buf, prp_bits, &len, NULL, 0) < 0) {
			return -1;
		}
		if (!bit_is_set(prp_bits, INPUT_PROP_POINTER)) {
			return -1;
		}
	}
#endif

	snprintf(buf, sizeof(buf), "/dev/input/event%zu", i);
	int fd = open(buf, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd == -1) {
		return -1;
	}
	if (!props_by_sysctl) {
		if (ioctl(fd, EVIOCGPROP(len), prp_bits) < 0) {
			close(fd);
			return -1;
		}
		if (!bit_is_set(prp_bits, INPUT_PROP_POINTER)) {
			close(fd);
			return -1;
		}
	}

#if HAVE_CAPSICUM
	cap_rights_t rights;
	cap_rights_init(&rights, CAP_READ, CAP_EVENT);

	if (cap_rights_limit(fd, &rights) < 0 && errno != ENOSYS) {
		err(1, "cap_rights_limit");
	}
#endif

	return fd;
}

#if HAVE_CAPSICUM
static void
evdev_create_command_runner(void)
{
	int read_pipe[2];
	int write_pipe[2];

	if (pipe2(read_pipe, O_CLOEXEC) == -1) {
		err(1, "pipe2");
	}
	if (pipe2(write_pipe, O_CLOEXEC) == -1) {
		err(1, "pipe2");
	}

	cap_rights_t rights;
	cap_rights_init(&rights, CAP_WRITE);
	command_runner_fd[0] = read_pipe[1];
	if (cap_rights_limit(command_runner_fd[0], &rights) < 0 && errno != ENOSYS) {
		err(1, "cap_rights_limit");
	}
	if (cap_rights_limit(write_pipe[1], &rights) < 0 && errno != ENOSYS) {
		err(1, "cap_rights_limit");
	}

	cap_rights_init(&rights, CAP_READ, CAP_EVENT);
	command_runner_fd[1] = write_pipe[0];
	if (cap_rights_limit(read_pipe[0], &rights) < 0 && errno != ENOSYS) {
		err(1, "cap_rights_limit");
	}
	if (cap_rights_limit(write_pipe[0], &rights) < 0 && errno != ENOSYS) {
		err(1, "cap_rights_limit");
	}

	pid_t child = fork();
	if (child > 0) {
		return;
	} else if (child == -1) {
		err(1, "fork");
	}

	// Drop suid privileges now
	if (getuid() != geteuid() || getgid() != getegid()) {
		if (setuid(getuid()) != 0 || setgid(getgid()) != 0) {
			err(1, "setuid");
		}
	}
	if (setuid(0) != -1) {
		errx(1, "still root?");
	}

	setproctitle("command runner: %s", command);

	close(read_pipe[1]);
	close(write_pipe[0]);
	closefrom(MAX(read_pipe[0], MAX(write_pipe[1], STDERR_FILENO)) + 1);

	if (write(write_pipe[1], "done", 5) != 5) {
		err(1, "write");
	}
	char buf[4];
	ssize_t n;
	while ((n = read(read_pipe[0], buf, 4)) > -1) {
		if (n == 4 && strncmp(buf, "run", 3) == 0) {
			tracker_run_command_internal();
			if (write(write_pipe[1], "done", 5) != 5) {
				err(1, "write");
			}
		}
	}

	err(1, "read");
}

static void
evdev_run_command(void)
{
	assert(command != NULL);

	char buf[5];
	if (read(command_runner_fd[1], buf, 5) != 5) {
		err(1, "read");
	}
	if (strncmp(buf, "done", 4) != 0) {
		errx(1, "invalid response");
	}

	if (write(command_runner_fd[0], "run", 4) != 4) {
		err(1, "write");
	}
}
#endif


static int
evdev_init(void)
{
#if HAVE_CAPSICUM
	close(STDIN_FILENO);
	closefrom(STDERR_FILENO + 1);

	if (command != NULL) {
		evdev_create_command_runner();
	}

	if (caph_limit_stderr() < 0) {
		err(1, "caph_limit_stderr");
	}
	if (caph_limit_stdout() < 0) {
		err(1, "caph_limit_stdout");
	}

#endif

#ifdef __FreeBSD__
	// Getting device properties via sysctls is not supported on all
	// FreeBSD versions.  As a heuristic just check the 0th
	// device to see if it supported.  Fallback is graceful if
	// this fails at the cost of a bunch of extra open(2) and
	// ioctl(2).
	if (sysctlbyname("kern.evdev.input.0.props", NULL, 0, NULL, 0) >= 0) {
		props_by_sysctl = 1;
	}
#endif

	int maxfd = 0;
	memset(fds, 0, sizeof(fds));
	for (size_t i = 0; i < nitems(fds); i++) {
		int fd = open_device(i);
		if (fd < 0) {
			continue;
		}
		maxfd = MAX(maxfd, fd);
		fds[nfds].fd = fd;
		fds[nfds].events = POLLIN;
		fds[nfds].revents = 0;
		nfds++;
	}

	// Drop suid privileges now
	if (getuid() != geteuid() || getgid() != getegid()) {
		if (setuid(getuid()) != 0 || setgid(getgid()) != 0) {
			err(1, "setuid");
		}
	}
	if (setuid(0) != -1) {
		errx(1, "still root?");
	}

	if (nfds == 0) {
		warnx("evdev: no mouse found");
		return 0;
	}

#if HAVE_CAPSICUM
	closefrom(maxfd + 1);

	if (caph_enter() < 0) {
		err(1, "cap_enter");
	}
#endif

	return 1;
}

static int
evdev_record_stroke(/* out */ struct stroke *stroke, uint16_t code)
{
	double x = 0.0;
	double y = 0.0;
	while (poll(fds, nfds, -1) > -1) {
		for (size_t i = 0; i < nfds; i++) {
			struct input_event ev;
			if ((fds[i].revents & POLLHUP) ||
			    (fds[i].revents & POLLIN) == 0) {
				continue;
			}
			while (read(fds[i].fd, &ev, sizeof(struct input_event)) > 0) {
				if (stroke != NULL && ev.type == EV_REL) {
					switch (ev.code) {
						case REL_X:
							x += ev.value;
							break;
						case REL_Y:
							y += ev.value;
							break;
					}
					stroke_add_point(stroke, x, y);
				} else if (ev.type == EV_KEY) {
					if (code != 0) {
						if (ev.code == code && ev.value == 1) {
							goto end;
						}
					} else {
						goto end;
					}
				}
			}
		}
	}

end:
	if (stroke != NULL) {
		stroke_finish(stroke);
	}

	return 1;
}
