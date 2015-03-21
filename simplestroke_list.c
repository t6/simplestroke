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
#include "util.h"

static void
simplestroke_list_usage() {
    fprintf(stderr,
            "usage: simplestroke list [-j]\n"
            "       simplestroke list -h\n");
}


static bool use_json = false;

static void
simplestroke_list_gesture_cb(__attribute__((unused)) stroke_t *stroke,
                             int id,
                             char *description,
                             char *command,
                             __attribute__((unused)) const void *u) {
    if (use_json) {
        static bool first = true;
        printf("%s{\"description\":", first ? "" : ",");
        json_dump_string(description, strlen(description));

        printf(",\"command\":");
        json_dump_string(command, strlen(command));
        printf(",\"id\":%i}", id);

        first = false;
    } else
        printf("%-5i %-36s %-36s\n", id, description, command);

    free(description);
    free(command);
}

int
simplestroke_list(int argc, char **argv) {
    struct option longopts[] = {
        { "help",  no_argument, NULL, 'h' },
        { "id", required_argument, NULL, 'i' },
        { "json", required_argument, NULL, 'j' },
        { NULL, 0, NULL, 0 }
    };

    int ch;
    while ((ch = getopt_long(argc, argv, "hj", longopts, NULL)) != -1) {
        switch (ch) {
        case 'j':
            use_json = true;
            break;
        case 'h':
        case '?':
            simplestroke_list_usage();
            return EXIT_FAILURE;
        default:
            break;
        }
    }

    const char *error = NULL;
    Database *db = database_open(&error);
    if (error) {
        warnx("%s", error);
        return EXIT_FAILURE;
    }

    if (use_json)
        printf("[");
    else
        printf("%-5s %-36s %-36s\n", "ID", "DESCRIPTION", "COMMAND");

    error = database_load_gestures(db, simplestroke_list_gesture_cb, NULL);

    if (use_json)
        printf("]\n");

    int retval = EXIT_SUCCESS;
    if (error) {
        warnx("Problem loading gestures: %s", error);
        retval = EXIT_FAILURE;
    }

    database_close(db);

    return retval;
}
