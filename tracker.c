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

#if HAVE_ERR
# include <err.h>
#endif
#include <sys/wait.h>
#include <unistd.h>

#include "tracker.h"

static const char *command;
static void tracker_run_command_internal(void);

#if HAVE_EVDEV
#include "tracker_evdev.c"
#endif

#if HAVE_X11
#include "tracker_x11.c"
#endif

void
tracker_init(const char *command_)
{
	command = command_;

#if HAVE_EVDEV
	if (!evdev_init())
#endif
#if HAVE_X11
	if (!x11_init())
#endif
		errx(1, "failed to initialize mouse tracker");

#if 0
	if (command_ != NULL) {
		if (daemon(0, 0) < 0) {
			err(1, "daemon");
		}
	}
#endif

}

int
tracker_record_stroke(struct stroke *stroke, uint16_t code)
{
	if (stroke != NULL) {
		memset(stroke, 0, sizeof(struct stroke));
	}

#if HAVE_EVDEV
	if (!evdev_record_stroke(stroke, code))
#endif
#if HAVE_X11
	if (!x11_record_stroke(stroke, code))
#endif
		return 0;

	return 1;
}

void
tracker_run_command()
{
#if HAVE_CAPSICUM && HAVE_EVDEV
	evdev_run_command();
#else
	tracker_run_command_internal();
#endif
}

static
void tracker_run_command_internal()
{
	pid_t child = fork();
	if (child == 0) {
		execlp(command, command, NULL);
		err(1, "execlp");
	} else if (child == -1) {
		err(1, "fork");
	}
	int status;
	if (waitpid(child, &status, 0) == -1) {
		err(1, "waitpid");
	}
}
