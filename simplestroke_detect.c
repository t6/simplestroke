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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#ifdef HAVE_BSD_STRING_H
#include <bsd/string.h>
#endif

#include "db.h"
#include "tracker.h"
#include "util.h"

typedef struct {
  stroke_t stroke;
  char *command;
  char *description;
  double score;
} GestureSelectionState;

static void load_gestures_cb(stroke_t *stroke, __unused int id,
                             char *description, char *command,
                             const void *user_data) {
  GestureSelectionState *state = (GestureSelectionState *)user_data;
  double score = stroke_compare(stroke, &state->stroke, NULL, NULL);
  if (score < stroke_infinity) { // state->stroke has similarity with stroke
    if (score < state->score) {  // check if there is a better candidate
      state->score = score;
      if (state->command)
        free(state->command);
      if (state->description)
        free(state->description);
      state->command = strdup(command);
      state->description = strdup(description);
    }
  }
  free(command);
  free(description);
}

static void simplestroke_detect_usage() {
  fprintf(stderr, "usage: simplestroke detect [-n]\n"
                  "       simplestroke detect -h\n");
}

int simplestroke_detect(const int argc, char **argv) {
  bool no_exec = false;
  int ch;
  while ((ch = getopt(argc, argv, "hn")) != -1) {
    switch (ch) {
      case 'h':
      case '?':
      case ':':
        simplestroke_detect_usage();
        return EX_USAGE;
      case 'n':
        no_exec = true;
        break;
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

  GestureSelectionState state = { .command = NULL,
                                  .description = NULL,
                                  .score = stroke_infinity };
  error = record_stroke(&state.stroke);
  if (error) {
    warnx("Failed recording gesture: %s", error);
    return EXIT_FAILURE;
  }

  error = database_load_gestures(db, load_gestures_cb, &state);
  if (error) {
    warnx("%s", error);
    return EXIT_FAILURE;
  }

  database_close(db);

  int retval = EXIT_FAILURE;
  if (state.score < stroke_infinity) { // We found a candidate
    if (no_exec)
      printf("Command: %s\nDescription: %s\n", state.command,
             state.description);
    else
      exec_commandline(state.command);
    retval = EXIT_SUCCESS;
  }

  if (state.command)
    free(state.command);
  if (state.description)
    free(state.description);

  return retval;
}
