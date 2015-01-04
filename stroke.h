/*
 * Copyright (c) 2015 Tobias Kortkamp <tobias.kortkamp@gmail.com>
 * Copyright (c) 2009 Thomas Jaeger <ThJaeger@gmail.com>
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
#ifndef __STROKE_H__
#define __STROKE_H__

#include <stdbool.h>

#ifndef MAX_STROKE_POINTS
#define MAX_STROKE_POINTS 512
#endif

typedef struct {
    double x;
    double y;
    double t;
    double dt;
    double alpha;
} point;

typedef struct {
    int n;
    bool is_finished;
    point p[MAX_STROKE_POINTS];
} stroke_t;

void stroke_add_point(stroke_t *stroke, double x, double y);
void stroke_finish(stroke_t *stroke);
double stroke_compare(const stroke_t *a, const stroke_t *b, int *path_x, int *path_y);

extern const double stroke_infinity;

#endif
