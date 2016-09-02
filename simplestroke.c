/*
 * Copyright (c) 2016 Tobias Kortkamp <t@tobik.me>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "stroke.h"
#include "tracker.h"

enum Gesture {
  // straight line gestures
  TopDown,
  DownTop,
  LeftRight,
  RightLeft,

  // Diagonal line gestures
  TopLeftDown,
  TopRightDown,
  DownLeftTop,
  DownRightTop,

  // 'z' line gestures
  LeftZ,  // a z starting from the left
  RightZ, // a mirrored z starting from the right

  SquareLeft,
  SquareRight,

  NoGesture
};

struct {
  const char* name;
  size_t n;
  struct {
    double x;
    double y;
  } p[MAX_STROKE_POINTS];
} default_gestures[] = {
      [TopDown] = { "TopDown",
                    3,
                    { { 0.5, 0.0 }, { 0.5, 0.5 }, { 0.5, 1.0 } } },
      [DownTop] = { "DownTop",
                    3,
                    { { 0.5, 1.0 }, { 0.5, 0.5 }, { 0.5, 0.0 } } },
      [LeftRight] = { "LeftRight",
                      3,
                      { { 0.0, 0.5 }, { 0.5, 0.5 }, { 1.0, 0.5 } } },
      [RightLeft] = { "RightLeft",
                      3,
                      { { 1.0, 0.5 }, { 0.5, 0.5 }, { 0.0, 0.5 } } },

      [TopLeftDown] = { "TopLeftDown",
                        3,
                        { { 0.0, 0.0 }, { 0.5, 0.5 }, { 1.0, 1.0 } } },
      [TopRightDown] = { "TopRightDown",
                         3,
                         { { 1.0, 0.0 }, { 0.5, 0.5 }, { 0.0, 1.0 } } },
      [DownLeftTop] = { "DownLeftTop",
                        3,
                        { { 1.0, 1.0 }, { 0.5, 0.5 }, { 0.0, 0.0 } } },
      [DownRightTop] = { "DownRightTop",
                         3,
                         { { 0.0, 1.0 }, { 0.5, 0.5 }, { 1.0, 0.0 } } },

      [LeftZ] = { "LeftZ",
                  9,
                  { { 0.0, 0.0 },
                    { 0.5, 0.0 },
                    { 1.0, 0.0 },
                    { 0.75, 0.25 },
                    { 0.5, 0.5 },
                    { 0.25, 0.75 },
                    { 0.0, 1.0 },
                    { 0.5, 1.0 },
                    { 1.0, 1.0 } } },
      [RightZ] = { "RightZ",
                   9,
                   { { 1.0, 1.0 },
                     { 0.5, 1.0 },
                     { 0.0, 1.0 },
                     { 0.25, 0.75 },
                     { 0.5, 0.5 },
                     { 0.75, 0.25 },
                     { 1.0, 0.0 },
                     { 0.5, 0.0 },
                     { 0.0, 0.0 } } },

      [SquareLeft] = { "SquareLeft",
                       9,
                       { { 0.0, 0.0 },
                         { 0.5, 0.0 },
                         { 1.0, 0.0 },
                         { 1.0, 0.5 },
                         { 1.0, 1.0 },
                         { 0.5, 1.0 },
                         { 0.0, 1.0 },
                         { 0.0, 0.5 },
                         { 0.0, 0.0 } } },
      [SquareRight] = { "SquareRight",
                        9,
                        { { 0.0, 0.0 },
                          { 0.0, 0.5 },
                          { 0.0, 1.0 },
                          { 0.5, 1.0 },
                          { 1.0, 1.0 },
                          { 1.0, 0.5 },
                          { 1.0, 0.0 },
                          { 0.5, 0.0 },
                          { 0.0, 0.0 } } },
};

stroke_t strokes[NoGesture];

extern int simplestroke_detect(const int argc, const char** argv);

static void init_gestures() {
  for (size_t i = 0; i < NoGesture; i++) {
    for (size_t j = 0; j < default_gestures[i].n; j++) {
      double x = default_gestures[i].p[j].x;
      double y = default_gestures[i].p[j].y;
      stroke_add_point(&strokes[i], x, y);
    }
    stroke_finish(&strokes[i]);
  }
}

int main(__unused int argc, __unused char* argv[]) {
  init_gestures();

  stroke_t stroke = {};
  record_stroke(&stroke);

  enum Gesture gesture = NoGesture;
  double best_score = stroke_infinity;
  for (size_t i = 0; i < NoGesture; i++) {
    stroke_t candidate = strokes[i];
    double score = stroke_compare(&candidate, &stroke, NULL, NULL);
    if (score < stroke_infinity) { // candidate has similarity with stroke
      if (score < best_score) {    // check if there is a better candidate
        best_score = score;
        gesture = i;
      }
    }
  }

  if (gesture != NoGesture) {
    printf("%s\n", default_gestures[gesture].name);
    return 0;
  }

  return 1;
}
