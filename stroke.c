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

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "stroke.h"
#include "util.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double stroke_infinity = 0.2;
static const double epsilon = 0.000001;

void
stroke_add_point(stroke_t *s,
                 const double x,
                 const double y) {
    assert(MAX_STROKE_POINTS > s->n);
    assert(!s->is_finished);
    s->p[s->n].x = x;
    s->p[s->n].y = y;
    s->n++;
}

void
stroke_finish(stroke_t *s) {
    if(s->is_finished) return;

    s->is_finished = true;

    const int n = s->n - 1;
    double total = 0.0;
    s->p[0].t = 0.0;

    for(int i = 0; i < n; i++) {
        total += hypot(s->p[i+1].x - s->p[i].x, s->p[i+1].y - s->p[i].y);
        s->p[i+1].t = total;
    }

    for(int i = 0; i <= n; i++)
        s->p[i].t /= total;

    double minX = s->p[0].x, minY = s->p[0].y, maxX = minX, maxY = minY;
    for(int i = 1; i <= n; i++) {
        minX = min(s->p[i].x, minX); // if (s->p[i].x < minX) minX = s->p[i].x;
        maxX = max(s->p[i].x, maxX); // if (s->p[i].x > maxX) maxX = s->p[i].x;
        minY = min(s->p[i].y, minY); // if (s->p[i].y < minY) minY = s->p[i].y;
        maxY = max(s->p[i].y, maxY); // if (s->p[i].y > maxY) maxY = s->p[i].y;
    }

    const double scaleX = maxX - minX;
    const double scaleY = maxY - minY;
    double scale = (scaleX > scaleY) ? scaleX : scaleY;
    if(scale < 0.001) scale = 1;
    for(int i = 0; i <= n; i++) {
        s->p[i].x = (s->p[i].x-(minX+maxX)/2)/scale + 0.5;
        s->p[i].y = (s->p[i].y-(minY+maxY)/2)/scale + 0.5;
    }

    for(int i = 0; i < n; i++) {
        s->p[i].dt = s->p[i+1].t - s->p[i].t;
        s->p[i].alpha = atan2(s->p[i+1].y - s->p[i].y, s->p[i+1].x - s->p[i].x)/M_PI;
    }
}

double
angle_difference(const double alpha,
                 const double beta) {
    double d = alpha - beta;
    if(d < -1.0) {
        return d + 2.0;
    } else if(d > 1.0) {
        return d - 2.0;
    } else {
        return d;
    }
}

double
stroke_angle_difference(const stroke_t *a,
                        const stroke_t *b,
                        const int i,
                        const int j) {
    assert(a);
    assert(b);
    return fabs(angle_difference(a->p[i].alpha, b->p[j].alpha));
}

void
step(const stroke_t *a,
     const stroke_t *b,
     const int N,
     double *dist,
     int *prev_x,
     int *prev_y,
     const int x,
     const int y,
     const double tx,
     const double ty,
     int *k,
     const int x2,
     const int y2) {
    const double dtx = a->p[x2].t - tx;
    const double dty = b->p[y2].t - ty;
    if(dtx >= dty * 2.2 || dty >= dtx * 2.2 || dtx < epsilon || dty < epsilon) {
        return;
    }
    (*k)++;

    double d = 0.0;
    int i = x, j = y;
    double next_tx = (a->p[i+1].t - tx) / dtx;
    double next_ty = (b->p[j+1].t - ty) / dty;
    double cur_t = 0.0;

    while(true) {
        const double ad = pow(angle_difference(a->p[i].alpha, b->p[j].alpha), 2);
        double next_t = next_tx < next_ty ? next_tx : next_ty;
        const bool done = next_t >= 1.0 - epsilon;
        if(done) {
            next_t = 1.0;
        }
        d += (next_t - cur_t)*ad;
        if(done) {
            break;
        }
        cur_t = next_t;
        if(next_tx < next_ty) {
            next_tx = (a->p[++i+1].t - tx) / dtx;
        } else {
            next_ty = (b->p[++j+1].t - ty) / dty;
        }
    }

    const double new_dist = dist[x*N+y] + d * (dtx + dty);
    if(new_dist >= dist[x2*N+y2]) {
        return;
    }

    prev_x[x2*N+y2] = x;
    prev_y[x2*N+y2] = y;
    dist[x2*N+y2] = new_dist;
}

/* To compare two gestures, we use dynamic programming to minimize (an
 * approximation) of the integral over square of the angle difference among
 * (roughly) all reparametrizations whose slope is always between 1/2 and 2.
 */
double
stroke_compare(const stroke_t *a,
               const stroke_t *b,
               int *path_x,
               int *path_y) {
    assert(a);
    assert(b);

    const int M = a->n;
    const int N = b->n;
    const int m = M - 1;
    const int n = N - 1;

    double dist[MAX_STROKE_POINTS * MAX_STROKE_POINTS];
    int prev_x[MAX_STROKE_POINTS * MAX_STROKE_POINTS];
    int prev_y[MAX_STROKE_POINTS * MAX_STROKE_POINTS];

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            dist[i*N + j] = stroke_infinity;
        }
    }
    dist[M*N - 1] = stroke_infinity;
    dist[0] = 0.0;

    for(int x = 0; x < m; x++) {
        for(int y = 0; y < n; y++) {
            if(dist[x*N + y] >= stroke_infinity) {
                continue;
            }
            const double tx  = a->p[x].t;
            const double ty  = b->p[y].t;
            int max_x = x;
            int max_y = y;
            int k = 0;

            while(k < 4) {
                if(a->p[max_x + 1].t - tx > b->p[max_y + 1].t - ty) {
                    max_y++;
                    if(max_y == n) {
                        step(a, b, N, dist, prev_x, prev_y, x, y, tx, ty, &k, m, n);
                        break;
                    }
                    for(int x2 = x + 1; x2 <= max_x; x2++) {
                        step(a, b, N, dist, prev_x, prev_y, x, y, tx, ty, &k, x2, max_y);
                    }
                } else {
                    max_x++;
                    if(max_x == m) {
                        step(a, b, N, dist, prev_x, prev_y, x, y, tx, ty, &k, m, n);
                        break;
                    }
                    for(int y2 = y + 1; y2 <= max_y; y2++) {
                        step(a, b, N, dist, prev_x, prev_y, x, y, tx, ty, &k, max_x, y2);
                    }
                }
            }
        }
    }
    const double cost = dist[M*N - 1];
    if(path_x && path_y) {
        if(cost < stroke_infinity) {
            int x = m;
            int y = n;
            int k = 0;
            while(x || y) {
                const int old_x = x;
                x = prev_x[x*N + y];
                y = prev_y[old_x*N + y];
                path_x[k] = x;
                path_y[k] = y;
                k++;
            }
        } else {
            path_x[0] = 0;
            path_y[0] = 0;
        }
    }

    return cost;
}
