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
#include <string.h>

#include "util.h"

extern int simplestroke_new(const int argc, const char** argv);
extern int simplestroke_detect(const int argc, const char** argv);

typedef struct {
    char* name;
    int (*handler)(const int, const char**);
} Subcommand;

static Subcommand subcommands[] = {
    { "new", simplestroke_new },
    { "detect", simplestroke_detect },
};

int
main(const int argc,
     const char** argv) {
    if(argc > 1) {
        const int n = sizeof(subcommands)/sizeof(Subcommand);
        for(int i = 0; i < n; i++) {
            const Subcommand subcmd = subcommands[i];
            if(!strcmp(argv[1], subcmd.name)) {
                return subcmd.handler(argc - 1, argv + 1);
            }
        }
    }

    exec_man_for_subcommand(NULL);

    return EXIT_FAILURE;
}
