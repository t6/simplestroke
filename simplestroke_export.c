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

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "db.h"
#include "lib/xo/xo.h"

static int
simplestroke_export_xo(stroke_t *stroke,
                       int id,
                       char *description,
                       char *command) {
    xo_open_container("gesture");
    xo_emit("{e:id/%i}"
            "{Lwc:Description}{:description/%s}\n"
            "{Lwc:Command}{:command/%s}\n",
            id, description, command);

    xo_open_list("points");
    for (int i = 0; i < stroke->n; i++) {
        xo_open_instance("point");
        xo_emit("{e:x/%f}{e:y/%f}", stroke->p[i].x, stroke->p[i].y);
        xo_close_instance("point");
    }
    xo_close_list("points");
    xo_close_container("gesture");

    return EXIT_SUCCESS;
}

static int
simplestroke_export_as_svg(stroke_t *stroke,
                           char *description,
                           char *command,
                           char *color) {
    // TODO: need to escape ]]> in description and command (end of CDATA)
    //       by expanding ]]> to ]]]]><![CDATA[>
    const int width = 250;
    const int height = 250;
    printf("<svg xmlns='http://www.w3.org/2000/svg'\n"
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

    for (int i = 0; i < stroke->n; i++)
        printf("%f,%f ", stroke->p[i].x, stroke->p[i].y);
    printf("'/>\n  </svg>\n</svg>\n");

    return EXIT_SUCCESS;
}

static void
simplestroke_export_usage() {
    fprintf(stderr,
            "usage: simplestroke export -i id [-c color] [-s] [--libxo xoargs]\n"
            "       simplestroke export -h\n");
}

int
simplestroke_export(int argc, char **argv) {
    struct option longopts[] = {
        { "help",  no_argument, NULL, 'h' },
        { "id", required_argument, NULL, 'i' },
        { "color", required_argument, NULL, 'c' },
        { "svg", no_argument, NULL, 's' },
        { NULL, 0, NULL, 0 }
    };

    int ch;
    int id = -1;
    bool svg_export = false;
    char *color = NULL;
    while ((ch = getopt_long(argc, argv, "hc:i:s", longopts, NULL)) != -1) {
        switch (ch) {
        case 's':
            svg_export = true;
            break;
        case 'h':
            simplestroke_export_usage();
            return EXIT_FAILURE;
        case 'i':
            id = (int)strtol(optarg, NULL, 10);
            if (errno == EINVAL || errno == ERANGE || id < 0) {
                fprintf(stderr, "--id must be a positive integer!\n");
                return EXIT_FAILURE;
            }
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

    if (id == -1) {
        simplestroke_export_usage();
        return EXIT_FAILURE;
    }

    const char *error = NULL;
    Database *db = database_open(&error);
    if (error) {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }

    stroke_t stroke = {};
    char *command = NULL;
    char *description = NULL;
    error = database_load_gesture_with_id(db, id, &stroke,
                                          &description, &command);
    if (error) {
        fprintf(stderr, "Problem loading gesture with id %i: %s\n", id, error);
        return EXIT_FAILURE;
    }

    int retval = svg_export
                 ? simplestroke_export_as_svg(&stroke, description, command, color)
                 : simplestroke_export_xo(&stroke, id, description, command);

    xo_finish();
    free(command);
    free(description);
    database_close(db);

    return retval;
}
