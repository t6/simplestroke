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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

char *
config_dir() {
    char *dir = NULL;

    // From the XDG Base Directory Specification:
    // If $XDG_CONFIG_HOME is either not set or empty,
    // a default equal to $HOME/.config should be used.

    const char *config_dir = getenv("XDG_CONFIG_HOME");
    if (config_dir && strcmp(config_dir, "") != 0) {
        if (asprintf(&dir, "%s/%s", config_dir, "simplestroke") < 0)
            err(EX_OSERR, "asprintf");
    } else {
        char *home = getenv("HOME");
        if (!home)
            err(EXIT_FAILURE, "$HOME not set?");

        if (asprintf(&dir, "%s/.config/%s", home, "simplestroke") < 0)
            err(EX_OSERR, "asprintf");
    }

    return dir;
}

/* Uses mkdir -p (the system command) to recursively create
   the directory `dir`, if it does not exist.
   Returns true if/after the dir exists, false otherwise.
   Also returns false if `dir` points to a file. */
bool
mkdirs(char *dir) {
    struct stat s;
    if (stat(dir, &s) == 0)
        return S_ISDIR(s.st_mode);
    else if (errno != ENOENT) {
        perror("stat");
        return false;
    }

    char *argv[] = { "mkdir", "-p", dir, NULL };

    pid_t child = fork();
    if (child == 0)
        execvp("mkdir", argv);
    else if (child == -1) {
        perror("fork");
        return false;
    }

    int status;
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return false;
    }
    if (status != 0)
        return false;

    return true;
}

void
exec_commandline(const char *commandline) {
    char *argv[] = { "/bin/sh", "-c", (char *)commandline, NULL };
    execv("/bin/sh", argv);

    // execv returned => an error occurred...
    perror("execvp");
    abort();
}

/* The following four functions are liberated from Jansson
   Copyright (c) 2009-2014 Petri Lehtinen <petri@digip.org>
   Extracted from commit 9b435d
   src/dump.c: dump_string
   src/utf.c: utf8_iterate, utf8_check_first, utf8_check_full
*/
static size_t
utf8_check_first(char byte) {
    unsigned char u = (unsigned char)byte;
    if (u < 0x80)
        return 1;
    if (0x80 <= u && u <= 0xBF) {
        /* second, third or fourth byte of a multi-byte
           sequence, i.e. a "continuation byte" */
        return 0;
    } else if (u == 0xC0 || u == 0xC1) {
        /* overlong encoding of an ASCII byte */
        return 0;
    } else if (0xC2 <= u && u <= 0xDF) {
        /* 2-byte sequence */
        return 2;
    } else if (0xE0 <= u && u <= 0xEF) {
        /* 3-byte sequence */
        return 3;
    } else if (0xF0 <= u && u <= 0xF4) {
        /* 4-byte sequence */
        return 4;
    } else { /* u >= 0xF5 */
        /* Restricted (start of 4-, 5- or 6-byte sequence) or invalid
           UTF-8 */
        return 0;
    }
}

static size_t
utf8_check_full(const char *buffer, size_t size, int32_t *codepoint) {
    size_t i;
    int32_t value = 0;
    unsigned char u = (unsigned char)buffer[0];
    if (size == 2)
        value = u & 0x1F;
    else if (size == 3)
        value = u & 0xF;
    else if (size == 4)
        value = u & 0x7;
    else
        return 0;
    for (i = 1; i < size; i++) {
        u = (unsigned char)buffer[i];
        if (u < 0x80 || u > 0xBF) {
            /* not a continuation byte */
            return 0;
        }
        value = (value << 6) + (u & 0x3F);
    }
    if (value > 0x10FFFF) {
        /* not in Unicode range */
        return 0;
    } else if (0xD800 <= value && value <= 0xDFFF) {
        /* invalid code point (UTF-16 surrogate halves) */
        return 0;
    } else if ((size == 2 && value < 0x80) ||
               (size == 3 && value < 0x800) ||
               (size == 4 && value < 0x10000)) {
        /* overlong encoding */
        return 0;
    }
    if (codepoint)
        *codepoint = value;
    return 1;
}

static const char *
utf8_iterate(const char *buffer, size_t bufsize,
             int32_t *codepoint) {
    size_t count;
    int32_t value;
    if (!bufsize)
        return buffer;
    count = utf8_check_first(buffer[0]);
    if (count <= 0)
        return NULL;
    if (count == 1)
        value = (unsigned char)buffer[0];
    else {
        if (count > bufsize || !utf8_check_full(buffer, count, &value))
            return NULL;
    }
    if (codepoint)
        *codepoint = value;
    return buffer + count;
}

static int
dump_string(FILE *file, const char *str, size_t len) {
    const char *pos, *end, *lim;
    int32_t codepoint;
    end = pos = str;
    lim = str + len;
    while (1) {
        const char *text;
        char seq[13];
        size_t length;
        while (end < lim) {
            end = utf8_iterate(pos, lim - pos, &codepoint);
            if (!end)
                return -1;
            /* mandatory escape or control char */
            if (codepoint == '\\' || codepoint == '"' || codepoint < 0x20)
                break;
            pos = end;
        }
        if (pos != str) {
            if (fwrite(str, pos - str, 1, file) != (size_t)(pos - str))
                return -1;
        }
        if (end == pos)
            break;
        /* handle \, /, ", and control codes */
        length = 2;
        switch (codepoint) {
        case '\\':
            text = "\\\\";
            break;
        case '\"':
            text = "\\\"";
            break;
        case '\b':
            text = "\\b";
            break;
        case '\f':
            text = "\\f";
            break;
        case '\n':
            text = "\\n";
            break;
        case '\r':
            text = "\\r";
            break;
        case '\t':
            text = "\\t";
            break;
        case '/':
            text = "\\/";
            break;
        default: {
            /* codepoint is in BMP */
            if (codepoint < 0x10000) {
                snprintf(seq, sizeof(seq), "\\u%04X", codepoint);
                length = 6;
            }
            /* not in BMP -> construct a UTF-16 surrogate pair */
            else {
                int32_t first, last;
                codepoint -= 0x10000;
                first = 0xD800 | ((codepoint & 0xffc00) >> 10);
                last = 0xDC00 | (codepoint & 0x003ff);
                snprintf(seq, sizeof(seq), "\\u%04X\\u%04X", first, last);
                length = 12;
            }
            text = seq;
            break;
        }
        }
        if (fwrite(text, length, 1, file) != length)
            return -1;
        str = pos = end;
    }
    return 0;
}

void
json_dump_string(const char *s, size_t n) {
    fwrite("\"", 1, 1, stdout);
    dump_string(stdout, s, n);
    fwrite("\"", 1, 1, stdout);
}
