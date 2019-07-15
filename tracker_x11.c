/*
 * Copyright (c) 2015-2016, 2019 Tobias Kortkamp <t@tobik.me>
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

#include <assert.h>
#if HAVE_ERR
# include <err.h>
#endif
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/record.h>

#include "stroke.h"
#include "tracker.h"

typedef struct {
	Display *control;
	Display *data;
	XRecordRange *range;
	XRecordContext context;

	uint16_t button;
	int track;
	INT16 x;
	INT16 y;

	struct stroke *stroke;
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
			if (state->button != 0 &&
			    event->u.u.detail == state->button) {
				state->track = 0;
			}
			break;
		case ButtonRelease:
			state->track = 0;
			break;
		case MotionNotify:
			state->x = event->u.keyButtonPointer.rootX;
			state->y = event->u.keyButtonPointer.rootY;
			break;
		default:
			XRecordFreeData(record_data);
			return;
		}

		if (state->stroke != NULL) {
			if (state->track && (state->stroke->n < MAX_STROKE_POINTS) &&
			    !state->stroke->is_finished) {
				stroke_add_point(state->stroke, state->x, state->y);
			} else {
				state->track = 0;
				stroke_finish(state->stroke);
			}
		}
	}

	XRecordFreeData(record_data);
}

static int
x11_init(void)
{
	Display *d = XOpenDisplay(NULL);
	if (d) {
		XCloseDisplay(d);
		return 1;
	}

	return 0;
}

static int
x11_record_stroke(/* out */ struct stroke *stroke, uint16_t button)
{
	RecorderState state = {
		.control = XOpenDisplay(NULL),
		.data = XOpenDisplay(NULL),
		.range = XRecordAllocRange(),
		.track = 1,
		.stroke = stroke,
		.context = 0,
		.button = button,
		.x = 0,
		.y = 0
	};

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

	state.range->device_events.first = ButtonRelease;
	state.range->device_events.last = MotionNotify;

	XRecordClientSpec spec = XRecordAllClients;
	state.context =
	    XRecordCreateContext(state.control, 0, &spec, 1, &state.range, 1);
	if (!state.context) {
		record_cleanup(&state);
		err(1, "create record context");
	}

	XSync(state.control, True);

	if (0 == XRecordEnableContextAsync(state.data, state.context,
	    &record_callback, (XPointer)&state)) {
		record_cleanup(&state);
		err(1, "enable data transfer");
	}

	while (state.track) {
		XRecordProcessReplies(state.data);
		usleep(50);
	}

	record_cleanup(&state);

	return 1;
}
