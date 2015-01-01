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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "recorder-x11.h"

stroke_t record_stroke_or_exit() {
    stroke_t stroke;
    const char *error = record_stroke(&stroke);
    if(error) {
        fprintf(stderr, "record_stroke: %s\n", error);
        exit(EXIT_FAILURE);
    }
    return stroke;
}

int main(const int argc, const char **argv) {
    if(argc == 1) {
        stroke_t stroke = record_stroke_or_exit();
        printf("Stroke with %i points recorded\n", stroke.n);
    } else {
        fprintf(stderr, "Usage: %s <keycode> <mousebutton>\n", argv[0]);
    }
    return EXIT_SUCCESS;
}
