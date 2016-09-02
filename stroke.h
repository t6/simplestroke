/*
 * Copyright (c) 2015 Tobias Kortkamp <t@tobik.me>
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

#ifndef MAX_STROKE_POINTS
#define MAX_STROKE_POINTS    512
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
	int is_finished;
	point p[MAX_STROKE_POINTS];
} stroke_t;

void stroke_add_point(stroke_t *, double, double);
void stroke_finish(stroke_t *);
double stroke_compare(const stroke_t *, const stroke_t *, int *, int *);

extern const double stroke_infinity;

#endif
