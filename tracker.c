/*
 * Copyright (c) 2015-2016 Tobias Kortkamp <t@tobik.me>
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
#include <string.h>
#include <unistd.h>

#include "stroke.h"

typedef struct {
	Display *control;
	Display *data;
	XRecordRange *range;
	XRecordContext context;

	int track;
	INT16 x;
	INT16 y;

	stroke_t *stroke;
} RecorderState;

/******************************************************************************
 * Track the mouse
 */
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
		case ButtonRelease:
			state->track = False;
			break;
		case MotionNotify:
			state->x = event->u.keyButtonPointer.rootX;
			state->y = event->u.keyButtonPointer.rootY;
			break;
		default:
			XRecordFreeData(record_data);
			return;
		}

		if (state->track && (state->stroke->n < MAX_STROKE_POINTS) &&
		    !state->stroke->is_finished) {
			stroke_add_point(state->stroke, state->x, state->y);
		} else {
			state->track = 0;
			stroke_finish(state->stroke);
		}
	}

	XRecordFreeData(record_data);
}

void
record_stroke(/* out */ stroke_t *stroke)
{
	assert(stroke);

	RecorderState state = {
		.control = XOpenDisplay(NULL),
		.data = XOpenDisplay(NULL),
		.range = XRecordAllocRange(),
		.track = 1,
		.stroke = stroke,
		.context = 0,
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

	state.range->device_events.first = KeyPress;
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
}
