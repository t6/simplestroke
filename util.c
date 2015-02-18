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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

void
config_home(char *buf,
            size_t len) {
    // From the XDG Base Directory Specification:
    // If $XDG_CONFIG_HOME is either not set or empty,
    // a default equal to $HOME/.config should be used.

    const char *config_dir = getenv("XDG_CONFIG_HOME");
    if (config_dir && strcmp(config_dir, "") != 0)
        strlcpy(buf, config_dir, len);
    else {
        char *home = getenv("HOME");
        if (!home)
            abort();

        strlcpy(buf, home, len);
        strlcat(buf, "/.config", len);
    }
}

void
config_dir(char *buf,
           size_t len) {
    config_home(buf, len);
    strlcat(buf, "/simplestroke", len);
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
