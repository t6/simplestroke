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

int
simplestroke_record(const int argc,
                    char** argv) {
    struct option longopts[] = {
        { "help",  no_argument, NULL, 'h' },
        { "description", required_argument, NULL, 'd' },
        { "command", required_argument, NULL, 'c' },
        { NULL, 0, NULL, 0 }
    };

    if(argc == 1)
        exec_man_for_subcommand(argv[0]);

    char* description = NULL;
    char* command = NULL;
    int ch;
    while((ch = getopt_long(argc, argv, "hd:c:", longopts, NULL)) != -1) {
        switch(ch) {
        case 'd':
            description = optarg;
            break;
        case 'c':
            command = optarg;
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

    if(command == NULL) {
        fprintf(stderr, "Missing argument --command!\n");
        return EXIT_FAILURE;
    }

    if(description == NULL) {
        fprintf(stderr, "WARNING: Using description <no description>\n");
        description = "<no description>";
        return EXIT_FAILURE;
    }

    const char* error;
    Database db;
    error = database_open(&db);
    if(error) {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }

    printf("Draw your stroke now and click a mouse button when you are finished. Press C-c to abort.\n");
    stroke_t stroke;
    error = record_stroke(&stroke);
    if(error) {
        fprintf(stderr, "record_stroke: %s\n", error);
        return EXIT_FAILURE;
    }

    error = database_add_stroke(db, &stroke, description, command);
    if(error) {
        fprintf(stderr, "Could not store stroke in database: %s\n", error);
        database_close(db);
        return EXIT_FAILURE;
    }

    database_close(db);

    printf("Success!\n");

    return EXIT_SUCCESS;
}
