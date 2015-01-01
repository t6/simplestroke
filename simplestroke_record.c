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
#include <sys/param.h>
#include <getopt.h>

#include "recorder-x11.h"
#include "db.h"
#include "util.h"

/* Returns a stroke. When there was a problem print an error message and exit */
stroke_t record_stroke_or_exit() {
    stroke_t stroke;
    const char *error = record_stroke(&stroke);
    if(error) {
        fprintf(stderr, "record_stroke: %s\n", error);
        exit(EXIT_FAILURE);
    }
    return stroke;
}

int simplestroke_record(const int argc, char** argv) {
    struct option longopts[] = {
        { "help",  no_argument, NULL, 'h' },
        { "title", required_argument, NULL, 't' },
        { NULL, 0, NULL, 0 }
    };

    if(argc == 1)
        exec_man_for_subcommand(argv[0]);

    char *title = NULL;
    int ch;
    while((ch = getopt_long(argc, argv, "ht:", longopts, NULL)) != -1) {
        switch(ch) {
        case 't':
            title = optarg;
            break;
        case 'h':
            exec_man_for_subcommand(argv[0]);
            break;
        case '?':
            return EXIT_FAILURE;
        default:
            break;
        }
    }

    if(title == NULL) {
        fprintf(stderr, "WARNING: Using title <unnamed>\n");
        title = "<unnamed>";
    }

    const char *error;
    Database db = database_open(&error);
    if(error != NULL) {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }

    database_close(db);

    return EXIT_SUCCESS;
}
