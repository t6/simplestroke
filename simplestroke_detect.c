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

typedef struct {
    stroke_t stroke;
    char command[2048];
    char description[2048];
    double score;
} GestureSelectionState;

void
load_gestures_cb(stroke_t *stroke,
                 const char* description,
                 const char* command,
                 const void* user_data) {
    GestureSelectionState *state = (GestureSelectionState*)user_data;
    double score = stroke_compare(stroke, &state->stroke, NULL, NULL);
    if(score < stroke_infinity) { // state->stroke has similarity with stroke
        if(score < state->score) { // check if there is a better candidate
            state->score = score;
            // Need to copy command and description (owned by sqlite)
            strlcpy(state->command, command, sizeof(state->command));
            strlcpy(state->description, description, sizeof(state->description));
        }
    }
}

int
simplestroke_detect(const int argc,
                 char** argv) {
    struct option longopts[] = {
        { "help",  no_argument, NULL, 'h' },
        { "no-exec", no_argument, NULL, 'n'},
        { NULL, 0, NULL, 0 }
    };

    bool no_exec = false;
    int ch;
    while((ch = getopt_long(argc, argv, "hn", longopts, NULL)) != -1) {
        switch(ch) {
        case 'h':
            exec_man_for_subcommand(argv[0]);
            break;
        case 'n':
            no_exec = true;
            break;
        case '?':
            return EXIT_FAILURE;
        default:
            break;
        }
    }

    const char* error;
    Database db;
    error = database_open(&db);
    if(error) {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }

    GestureSelectionState state = { .stroke = {},
                                    .command = {},
                                    .description = {},
                                    .score = stroke_infinity };
    error = record_stroke(&state.stroke);
    if(error) {
        fprintf(stderr, "Failed recording gesture: %s\n", error);
        return EXIT_FAILURE;
    }

    error = database_load_gestures(db, load_gestures_cb, &state);
    if(error) {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }

    database_close(db);

    if(state.score < stroke_infinity) { // We found a candidate
        if(no_exec) {
            printf("Command: %s\nDescription: %s\n", state.command, state.description);
        } else {
            exec_commandline(state.command);
        }
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
