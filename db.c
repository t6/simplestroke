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
#include <sys/types.h>
#include <sys/param.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sqlite3.h>

#include "util.h"
#include "db.h"

/* Opens the database, creating it and the configuration directory if necessary.
   Returns the database handle and sets error to NULL if successful, otherwise
   error is set to an appropriate error message. */
Database database_open(const char **error) {
    char path[MAXPATHLEN];
    config_dir(path, sizeof(path));
    if(!mkdirs(path)) {
        if(error) *error = "Could not create config directory";
        return (Database){ .db = NULL };
    }
    strlcat(path, "/simplestroke.sqlite", sizeof(path));

    sqlite3 *db;
    if(SQLITE_OK != sqlite3_open_v2(path,
                                    &db,
                                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                    NULL)) {
        if(error) *error = sqlite3_errmsg(db);
        return (Database){ .db = NULL };
    }

    if(error) *error = NULL;

    return (Database){ .db = db };
}

bool database_close(Database db) {
    if(db.db && sqlite3_close(db.db) == SQLITE_OK) {
        db.db = NULL;
        return true;
    }
    return false;
}
