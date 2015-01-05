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
simplestroke_export_as_svg(stroke_t *stroke,
                           char* description,
                           char* command,
                           char* filename,
                           char* color) {
    FILE* f = stdout;
    if(filename) {
        f = fopen(filename, "w");
        if(!f) {
            perror("fopen");
            return EXIT_FAILURE;
        }
    }

    // TODO: need to escape ]]> in description and command (end of CDATA)
    //       by expanding ]]> to ]]]]><![CDATA[>
    const int width = 250;
    const int height = 250;
    fprintf(f,
            "<svg xmlns='http://www.w3.org/2000/svg'\n"
            "     width='%ipx' height='%ipx'>\n"
            "  <!-- the gesture's description: -->\n"
            "  <title><![CDATA[%s]]></title>\n"
            "  <!-- command to execute after recognizing this gesture: -->\n"
            "  <desc><![CDATA[%s]]></desc>\n"
            "  <svg viewBox=\"0 0 1 1\" width='%ipx' height='%ipx'>\n"
            "    <!-- points that make up this gesture -->\n"
            "    <polyline fill='none'\n"
            "              stroke-linejoin='round'\n"
            "              stroke-linecap='round'\n"
            "              stroke='%s'\n"
            "              stroke-width='1%%'\n"
            "              points='",
            width, height,
            description,
            command,
            width, height,
            color ? color : "black");

    for(int i = 0; i < stroke->n; i++) {
        fprintf(f, "%f,%f ", stroke->p[i].x, stroke->p[i].y);
    }
    fprintf(f, "'/>\n  </svg>\n</svg>\n");

    fclose(f);

    return EXIT_SUCCESS;
}

int
simplestroke_export(const int argc,
                    char** argv) {
    struct option longopts[] = {
        { "help",  no_argument, NULL, 'h' },
        { "id", required_argument, NULL, 'i' },
        { "file", required_argument, NULL, 'f'},
        { "color", required_argument, NULL, 'c' },
        { NULL, 0, NULL, 0 }
    };

    int ch;
    int id = -1;
    char *filename = NULL;
    char *color = NULL;
    while((ch = getopt_long(argc, argv, "hc:i:f:", longopts, NULL)) != -1) {
        switch(ch) {
        case 'h':
            exec_man_for_subcommand(argv[0]);
            break;
        case 'i':
            id = (int)strtol(optarg, NULL, 10);
            if(errno == EINVAL || errno == ERANGE || id < 0) {
                fprintf(stderr, "--id must be a positive integer!\n");
                return EXIT_FAILURE;
            }
            break;
        case 'f':
            filename = optarg;
            break;
        case 'c':
            color = optarg;
            break;
        case '?':
            return EXIT_FAILURE;
        default:
            break;
        }
    }

    if(id == -1) {
        exec_man_for_subcommand(argv[0]);
    }

    const char* error;
    Database db;
    error = database_open(&db);
    if(error) {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }

    stroke_t stroke = {};
    char* command = NULL;
    char* description = NULL;
    error = database_load_gesture_with_id(db, id, &stroke,
                                          &description, &command);
    if(error) {
        fprintf(stderr, "Problem loading gesture with id %i: %s\n", id, error);
        return EXIT_FAILURE;
    }

    int retval = simplestroke_export_as_svg(&stroke, description, command, filename, color);

    free(command);
    free(description);
    database_close(db);

    return retval;
}
