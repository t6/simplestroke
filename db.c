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
#include "stroke.h"

static const char* schema =
    "CREATE TABLE IF NOT EXISTS gesture (                               \
         id INTEGER PRIMARY KEY,                                        \
         time INTEGER NOT NULL,                                         \
         description TEXT NOT NULL,                                     \
         command TEXT NOT NULL,                                         \
         npoints INTEGER NOT NULL,                                      \
         points BLOB NOT NULL                                           \
    );";

static const char* insert_gesture_sql =
    "INSERT INTO gesture (time, description, command, npoints, points)  \
        VALUES (?, ?, ?, ?, ?);";

/* Opens the database, creating it and the configuration directory if necessary.
   Returns the database handle and sets error to NULL if successful, otherwise
   error is set to an appropriate error message. */
const char*
database_open(/* out */ Database* db) {
    if(!db) {
        return "db was null";
    }

    char path[MAXPATHLEN];
    config_dir(path, sizeof(path));
    if(!mkdirs(path)) {
        return "Could not create config directory";
    }
    strlcat(path, "/simplestroke.sqlite", sizeof(path));

    const int status = sqlite3_open_v2(path,
                                       &db->db,
                                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                       NULL);
    if(SQLITE_OK != status) {
        return sqlite3_errstr(status);
    }

    if(SQLITE_OK != sqlite3_exec(db->db, schema, NULL, NULL, NULL)) {
        sqlite3_close(db->db);
        return "could not create schema";
    }

    if(SQLITE_OK != sqlite3_prepare_v2(db->db,
                                       insert_gesture_sql,
                                       -1,
                                       &db->insert_gesture_stmt,
                                       NULL)) {
        const char* error = sqlite3_errstr(sqlite3_extended_errcode(db->db));
        sqlite3_close(db->db);
        return error;
    }

    return NULL;
}

const char*
database_close(Database db) {
    sqlite3_finalize(db.insert_gesture_stmt);
    const int status = sqlite3_close(db.db);
    if(SQLITE_OK == status) {
        db.db = NULL;
        return NULL;
    }

    return sqlite3_errstr(status);
}

const char *
database_add_gesture(Database db,
                     stroke_t* stroke,
                     const char* description,
                     const char* command) {
    sqlite3_reset(db.insert_gesture_stmt);

    if(SQLITE_OK != sqlite3_bind_int(db.insert_gesture_stmt,
                                     1, time(NULL))) {
        return sqlite3_errstr(sqlite3_extended_errcode(db.db));
    }
    if(SQLITE_OK != sqlite3_bind_text(db.insert_gesture_stmt,
                                      2, description, -1,
                                      SQLITE_STATIC)) {
        return sqlite3_errstr(sqlite3_extended_errcode(db.db));
    }
    if(SQLITE_OK != sqlite3_bind_text(db.insert_gesture_stmt,
                                       3, command, -1,
                                       SQLITE_STATIC)) {
        return sqlite3_errstr(sqlite3_extended_errcode(db.db));
    }
    if(SQLITE_OK != sqlite3_bind_int(db.insert_gesture_stmt,
                                      4, stroke->n)) {
        return sqlite3_errstr(sqlite3_extended_errcode(db.db));
    }
    if(SQLITE_OK != sqlite3_bind_blob(db.insert_gesture_stmt,
                                      5, stroke->p, sizeof(double)*stroke->n,
                                      SQLITE_STATIC)) {
        return sqlite3_errstr(sqlite3_extended_errcode(db.db));
    }

    if(SQLITE_DONE != sqlite3_step(db.insert_gesture_stmt)) {
        return "step was not done?";
    }
    return NULL;
}
