/*
 * Copyright (c) 2016 Tobias Kortkamp <t@tobik.me>
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

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/record.h>
#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
	Display *control;
	Display *data;
	XRecordRange *range;
	XRecordContext context;

	char *command;
	int button;
	int mod;
} RecorderState;

static void
record_cleanup(const RecorderState *state)
{
	if (!state) {
		return;
	}

	if ((state->context != 0) && state->control) {
		XRecordFreeContext(state->control, state->context);
	}

	if (state->control) {
		XCloseDisplay(state->control);
	}

	if (state->data) {
		XCloseDisplay(state->data);
	}

	if (state->range) {
		XFree(state->range);
	}
}

static void
record_callback(XPointer closure, XRecordInterceptData *record_data)
{
	RecorderState *state = (RecorderState *)closure;
	// the data field can be treated as an xEvent as defined in X11/Xproto.h
	const xEvent *event = (xEvent *)record_data->data;

	if (record_data->category == XRecordFromServer) {
		switch (event->u.u.type) {
		case ButtonPress:
			if (event->u.u.detail == state->button
			    && (state->mod == -1
				|| (event->u.keyButtonPointer.state & state->mod))) {
				pid_t child = fork();
				if (child == 0) {
					execlp(state->command, state->command, NULL);
				} else if (child == -1) {
					err(1, "fork");
				}

				int status;
				if (waitpid(child, &status, 0) == -1) {
					err(1, "waitpid");
				}
			}
			break;
		default:
			XRecordFreeData(record_data);
			return;
		}
	}

	XRecordFreeData(record_data);
}

void
usage() {
	fprintf(stderr,
		"usage: simplestroke-daemon -c <command> [-b <mouse button>] [-m mod[1-5]|control|shift|lock]\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	int ch;
	const char *errstr;
	RecorderState state = { .command = NULL, .button = 3, .mod = -1 };

	while ((ch = getopt(argc, argv, "b:c:m:")) != -1) {
		switch (ch) {
		case 'b':
			state.button = (int)strtonum(optarg, 1, 16, &errstr);
			if (errstr != NULL) {
				errx(1, "mouse button: %s\n", errstr);
			}
			break;
		case 'c':
			state.command = optarg;
			break;
		case 'm':
			if (strcasecmp(optarg, "mod1") == 0) {
				state.mod = Mod1Mask;
			} else if (strcasecmp(optarg, "mod2") == 0) {
				state.mod = Mod2Mask;
			} else if (strcasecmp(optarg, "mod3") == 0) {
				state.mod = Mod3Mask;
			} else if (strcasecmp(optarg, "mod4") == 0) {
				state.mod = Mod4Mask;
			} else if (strcasecmp(optarg, "mod5") == 0) {
				state.mod = Mod5Mask;
			} else if (strcasecmp(optarg, "control") == 0) {
				state.mod = ControlMask;
			} else if (strcasecmp(optarg, "shift") == 0) {
				state.mod = ShiftMask;
			} else if (strcasecmp(optarg, "lock") == 0) {
				state.mod = LockMask;
			} else {
				usage();
			}
			break;
		case '?':
		default:
			usage();
		}
	}

	if (!state.command)
		usage();

	state.control = XOpenDisplay(NULL);
	state.data = XOpenDisplay(NULL);
	state.range = XRecordAllocRange();

	// See http://www.x.org/docs/Xext/recordlib.pdf
	if (!state.control) {
		record_cleanup(&state);
		err(1, "open control display");
	}
	if (!state.data) {
		record_cleanup(&state);
		err(1, "open data display");
	}

	if (!state.range) {
		record_cleanup(&state);
		err(1, "create record range");
	}

	state.range->device_events.first = ButtonPress;
	state.range->device_events.last = ButtonPress;

	XRecordClientSpec spec = XRecordAllClients;
	state.context =
	    XRecordCreateContext(state.data, 0, &spec, 1, &state.range, 1);
	if (!state.context) {
		record_cleanup(&state);
		err(1, "create record context");
	}

	XSync(state.control, True);

	if (daemon(0, 0) < 0) {
		err(1, "daemon");
	}

	XRecordEnableContext(
		state.data, state.context, &record_callback, (XPointer)&state);
	record_cleanup(&state);

	return 1;
}
