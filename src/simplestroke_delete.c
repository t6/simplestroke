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
#include "db.h"

static void
simplestroke_delete_usage() {
    fprintf(stderr,
            "usage: simplestroke delete -i id\n"
            "       simplestroke delete -h\n");
}

int
simplestroke_delete(int argc, char **argv) {
    int ch;
    int id = -1;
    while ((ch = getopt(argc, argv, "hi:")) != -1) {
        switch (ch) {
        case 'h':
        case '?':
        case ':':
            simplestroke_delete_usage();
            return EXIT_FAILURE;
        case 'i':
            id = (int)strtol(optarg, NULL, 10);
            if (errno == EINVAL || errno == ERANGE || id < 0) {
                warnx("-i must be a positive integer, was: %s", optarg);
                return EXIT_FAILURE;
            }
            break;
        }
    }

    if (id == -1) {
        simplestroke_delete_usage();
        return EXIT_FAILURE;
    }

    const char *error = NULL;
    Database *db = database_open(&error);
    if (error) {
        warnx("%s", error);
        return EXIT_FAILURE;
    }

    int retval;
    error = database_delete_gesture_with_id(db, id);
    if (error) {
        retval = EXIT_FAILURE;
        warnx("Deleting gesture with id %i failed: %s", id, error);
    } else
        retval = EXIT_SUCCESS;

    database_close(db);

    return retval;
}
