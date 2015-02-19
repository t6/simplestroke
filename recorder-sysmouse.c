/*
 * Copyright (c) 2015 Tobias Kortkamp <tobias.kortkamp@gmail.com>
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

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mouse.h>
#include <unistd.h>

#include "recorder.h"

/* sys/mouse.h is missing a constant for the 9th mouse button
 * If it is pressed down, the 9th bit is set.
 */
#define MOUSE_BUTTON9DOWN 0x100000000

/* Returns true if any mouse button was released, which
 * is the signal to stop polling /dev/sysmouse.
 */
static bool
button_released(mousestatus_t *status) {
#define RELEASED(n)                                     \
    ((status->obutton & MOUSE_BUTTON##n##DOWN)          \
     && !(status->button & MOUSE_BUTTON##n##DOWN))

    return RELEASED(1) || RELEASED(2) || RELEASED(3) || RELEASED(4) ||
           RELEASED(5) || RELEASED(6) || RELEASED(7) || RELEASED(8) ||
           RELEASED(9);

#undef RELEASED
}

const char *
record_stroke(/* out */ stroke_t *stroke) {
    mousestatus_t mouse_status;
    int x = 0;
    int y = 0;
    int status = 0;

    int mouse = open("/dev/sysmouse", O_RDONLY);
    if (mouse == -1)
        return strerror(mouse);

    while (true) {
        usleep(50);

        if ((status = ioctl(mouse, MOUSE_GETSTATUS, &mouse_status)) == -1) {
            close(mouse);
            return strerror(status);
        }

        if (button_released(&mouse_status)) {
            stroke_finish(stroke);
            break;
        }

        if (mouse_status.flags & MOUSE_POSCHANGED
                && (mouse_status.dx != 0 || mouse_status.dy != 0)) {
            x += mouse_status.dx;
            y += mouse_status.dy;

            if (stroke->n < MAX_STROKE_POINTS &&
                    !stroke->is_finished)
                stroke_add_point(stroke, x, y);
            else {
                stroke_finish(stroke);
                break;
            }
        }
    }

    close(mouse);

    return NULL;
}
