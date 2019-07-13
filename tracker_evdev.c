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

#ifdef USE_EVDEV

#ifdef __FreeBSD__
# include <sys/capsicum.h>
# include <capsicum_helpers.h>
# include <sys/sysctl.h>
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

#include <linux/input.h>

#include "stroke.h"

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

static int props_by_sysctl = 0;

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

#if defined(__FreeBSD__)
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
	int fd = open(buf, O_RDONLY | O_NONBLOCK);
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

#ifdef __FreeBSD__
	cap_rights_t rights;
	cap_rights_init(&rights, CAP_READ, CAP_EVENT);

	if (cap_rights_limit(fd, &rights) < 0 && errno != ENOSYS) {
		err(1, "cap_rights_limit");
	}
#endif

	return fd;
}

static int
handle_events(struct pollfd fds[], size_t nfds, stroke_t *stroke, double *x, double *y)
{
	for (size_t i = 0; i < nfds; i++) {
		struct input_event ev;
		if ((fds[i].revents & POLLHUP) ||
		    (fds[i].revents & POLLIN) == 0) {
			continue;
		}
		while (read(fds[i].fd, &ev, sizeof(struct input_event)) > 0) {
			if (ev.type == EV_REL) {
				switch (ev.code) {
					case REL_X:
						*x += ev.value;
						break;
					case REL_Y:
						*y += ev.value;
						break;
				}
				stroke_add_point(stroke, *x, *y);
			} else if (ev.type == EV_KEY) {
				return 0;
			}
		}
	}

	return 1;
}

int
evdev_record_stroke(/* out */ stroke_t *stroke)
{
#ifdef __FreeBSD__
	if (caph_limit_stdio() < 0) {
		err(1, "caph_limit_stdio");
	}

	close(STDIN_FILENO);

	// Getting device properties via sysctls is not supported on all
	// FreeBSD versions.  As a heuristic just check the 0th
	// device to see if it supported.  Fallback is graceful if
	// this fails at the cost of a bunch of extra open(2) and
	// ioctl(2).
	if (sysctlbyname("kern.evdev.input.0.props", NULL, 0, NULL, 0) >= 0) {
		props_by_sysctl = 1;
	}
#endif

	struct pollfd fds[32] = {};
	size_t nfds = 0;
	int maxfd = 0;
	for (size_t i = 0; i < nitems(fds); i++) {
		int fd = open_device(i);
		if (fd < 0) {
			continue;
		}
		maxfd = MAX(maxfd, fd);
		fds[i].fd = fd;
		fds[i].events = POLLIN;
		fds[i].revents = 0;
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

#ifdef __FreeBSD__
	closefrom(maxfd + 1);

	if (caph_enter() < 0) {
		err(1, "cap_enter");
	}
#endif


	double x = 0.0;
	double y = 0.0;
	while (poll(fds, nfds, -1) > -1) {
		if (handle_events(fds, nfds, stroke, &x, &y) == 0) {
			break;
		}
	}

	stroke_finish(stroke);

	return 1;
}

#endif
