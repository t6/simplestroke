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

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "db.h"
#include "recorder.h"

static void
simplestroke_new_usage() {
    fprintf(stderr,
            "usage: simplestroke new -c command -d description [-w seconds]\n"
            "       simplestroke new -h\n");
}

int
simplestroke_new(const int argc,
                 char **argv) {
    if (argc == 1) {
        simplestroke_new_usage();
        return EXIT_FAILURE;
    }

    char *description = NULL;
    char *command = NULL;
    // wait for 2 seconds before recording by default
    const long default_wait = 2;
    long wait = default_wait;
    int ch;
    while ((ch = getopt(argc, argv, "hw:d:c:")) != -1) {
        switch (ch) {
        case 'w':
            wait = strtol(optarg, NULL, 10);
            if (errno == EINVAL || errno == ERANGE || wait < 0) {
                warnx("-w expects a positive integer, falling back to default of %li seconds!\n",
                      default_wait);
                wait = default_wait;
            }
            break;
        case 'd':
            description = optarg;
            break;
        case 'c':
            command = optarg;
            break;
        case 'h':
        case '?':
        case ':':
            simplestroke_new_usage();
            return EXIT_FAILURE;
        default:
            break;
        }
    }

    if (command == NULL || description == NULL) {
        simplestroke_new_usage();
        return EXIT_FAILURE;
    }

    const char *error = NULL;
    Database *db = database_open(&error);
    if (error) {
        warnx("%s", error);
        return EXIT_FAILURE;
    }

    // wait for X seconds, then record
    if (wait > 0) {
        const char *msg = "Recording gesture in %i seconds (press C-c to abort)...";
        printf(msg, wait);
        fflush(stdout);
        for (long i = wait - 1; i >= 0; i--) {
            sleep(1);
            printf("\r");
            printf(msg, i);
            fflush(stdout);
        }
        printf("\n");
        printf("Draw your gesture now and click a mouse button when you are finished. Press C-c to abort.\n");
    }

    stroke_t stroke = {};
    error = record_stroke(&stroke);
    if (error) {
        warnx("Failed recording gesture: %s", error);
        database_close(db);
        free((void *)error);
        return EXIT_FAILURE;
    }

    error = database_add_gesture(db, &stroke, description, command);
    if (error) {
        warnx("Could not store gesture in database: %s", error);
        database_close(db);
        return EXIT_FAILURE;
    }

    database_close(db);

    printf("Success!\n");

    return EXIT_SUCCESS;
}
