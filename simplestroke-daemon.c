/*
 * Copyright (c) 2016, 2019 Tobias Kortkamp <t@tobik.me>
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

#include <sys/param.h>
#if HAVE_ERR
# include <err.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stroke.h"
#include "tracker.h"

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#if HAVE_EVDEV
struct evdev_mouse_buttons {
	const char *name;
	const char *altname;
	uint16_t value;
};

static struct evdev_mouse_buttons buttons[] = {
	{ "BTN_MOUSE", NULL, 0x110 },
	{ "BTN_LEFT", "left", 0x110 },
	{ "BTN_RIGHT", "right", 0x111 },
	{ "BTN_MIDDLE", "middle", 0x112 },
	{ "BTN_SIDE", "side", 0x113 },
	{ "BTN_EXTRA", "extra", 0x114 },
	{ "BTN_FORWARD", "forward", 0x115 },
	{ "BTN_BACK", "back", 0x116 },
};
#endif

static void
usage()
{
	fprintf(stderr, "usage: simplestroke-daemon -c <command> -b <mouse button>\n");
	exit(1);
}

static uint16_t
parse_mouse_button(const char *arg)
{
#if HAVE_EVDEV
	for (size_t i = 0; i < nitems(buttons); i++) {
		if (strcmp(arg, buttons[i].name) == 0 ||
		    (buttons[i].altname && strcmp(arg, buttons[i].altname) == 0)) {
			return buttons[i].value;
		}
	}
#endif

	const char *errstr;
	int16_t button = (int16_t)strtonum(optarg, 0, INT_MAX, &errstr);
	if (errstr == NULL) {
		return button;
	}

	errx(1, "invalid mouse button");
}

int
main(int argc, char *argv[])
{
	int ch;
	const char *command = NULL;
	uint16_t button;

	while ((ch = getopt(argc, argv, "b:c:")) != -1) {
		switch (ch) {
		case 'b':
			button = parse_mouse_button(optarg);
			break;
		case 'c':
			command = optarg;
			break;
		case '?':
		default:
			usage();
		}
	}

	tracker_init(command);

	while (1) {
		if (!tracker_record_stroke(NULL, button)) {
			continue;
		}
		tracker_run_command();
	}

	return 1;
}
