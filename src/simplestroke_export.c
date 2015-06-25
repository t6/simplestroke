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
#include <sysexits.h>

#include "db.h"
#include "util.h"

static int simplestroke_export_svg(stroke_t *stroke, int id, char *description,
                                   char *command, char *color) {
  // XXX: strictly speakin we would need to escape ]]> in description
  //      and command (end of CDATA) by expanding ]]> to ]]]]><![CDATA[>
  const int width = 250;
  const int height = 250;
  printf("<svg xmlns='http://www.w3.org/2000/svg'\n"
         "     width='%ipx' height='%ipx'>\n"
         "  <!-- the gesture's description: -->\n"
         "  <title><![CDATA[%s]]></title>\n"
         "  <!-- command to execute after recognizing this gesture: -->\n"
         "  <desc><![CDATA[%s]]></desc>\n"
         "  <svg id=\"gesture-%i\" <!-- the id of the gesture -->\n"
         "       viewBox=\"0 0 1 1\" width='%ipx' height='%ipx'>\n"
         "    <!-- points that make up this gesture -->\n"
         "    <polyline fill='none'\n"
         "              stroke-linejoin='round'\n"
         "              stroke-linecap='round'\n"
         "              stroke='%s'\n"
         "              stroke-width='1%%'\n"
         "              points='",
         width, height, description, command, id, width, height,
         color ? color : "black");

  for (int i = 0; i < stroke->n; i++)
    printf("%f,%f ", stroke->p[i].x, stroke->p[i].y);
  printf("'/>\n  </svg>\n</svg>\n");

  return EXIT_SUCCESS;
}

static void simplestroke_export_usage() {
  fprintf(stderr, "usage: simplestroke export -i id [-c color] [-s] [-j]\n"
                  "       simplestroke export -h\n");
}

int simplestroke_export(int argc, char **argv) {
  int ch;
  int id = -1;
  char *color = NULL;
  while ((ch = getopt(argc, argv, "hc:i:")) != -1) {
    switch (ch) {
      case 'h':
      case '?':
      case ':':
        simplestroke_export_usage();
        return EX_USAGE;
      case 'i':
        id = (int)strtol(optarg, NULL, 10);
        if (errno == EINVAL || errno == ERANGE || id < 0) {
          warnx("-i must be a positive integer, was: %s", optarg);
          return EXIT_FAILURE;
        }
        break;
      case 'c':
        color = optarg;
        break;
      default:
        break;
    }
  }

  if (id == -1) {
    simplestroke_export_usage();
    return EX_USAGE;
  }

  const char *error = NULL;
  Database *db = database_open(&error);
  if (error) {
    warnx("%s", error);
    return EXIT_FAILURE;
  }

  int retval = EXIT_FAILURE;
  stroke_t stroke;
  char *command = NULL;
  char *description = NULL;
  error =
      database_load_gesture_with_id(db, id, &stroke, &description, &command);
  if (error)
    warnx("Problem loading gesture with id %i: %s", id, error);
  else {
    retval = simplestroke_export_svg(&stroke, id, description, command, color);

    free(command);
    free(description);
  }
  database_close(db);

  return retval;
}
