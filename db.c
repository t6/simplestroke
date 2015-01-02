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
    "CREATE TABLE IF NOT EXISTS stroke (                                \
         id INTEGER PRIMARY KEY,                                        \
         time INTEGER NOT NULL,                                         \
         description TEXT NOT NULL,                                     \
         command TEXT NOT NULL,                                         \
         npoints INTEGER NOT NULL,                                      \
         points BLOB NOT NULL                                           \
    );";

static const char* insert_stroke_sql =
    "INSERT INTO stroke (time, description, command, npoints, points)   \
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

    if(SQLITE_OK != sqlite3_open_v2(path,
                                    &db->db,
                                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                    NULL)) {
        return sqlite3_errmsg(db->db);
    }

    char* error;
    if(SQLITE_OK != sqlite3_exec(db->db, schema, NULL, NULL, &error)) {
        sqlite3_close(db->db);
        return error;
    }

    if(SQLITE_OK != sqlite3_prepare_v2(db->db,
                                       insert_stroke_sql,
                                       -1,
                                       &db->insert_stroke_stmt,
                                       NULL)) {
        const char *error = sqlite3_errmsg(db->db);
        sqlite3_close(db->db);
        return error;
    }

    return NULL;
}

const char*
database_close(Database db) {
    sqlite3_finalize(db.insert_stroke_stmt);
    if(sqlite3_close(db.db) == SQLITE_OK) {
        db.db = NULL;
        return NULL;
    }

    return sqlite3_errmsg(db.db);
}

const char *
database_add_stroke(Database db,
                    stroke_t* stroke,
                    const char* description,
                    const char* command) {
    sqlite3_reset(db.insert_stroke_stmt);

    if(SQLITE_OK != sqlite3_bind_int(db.insert_stroke_stmt,
                                      1, time(NULL))) {
        return sqlite3_errmsg(db.db);
    }
    if(SQLITE_OK != sqlite3_bind_text(db.insert_stroke_stmt,
                                      2, description, -1,
                                      SQLITE_STATIC)) {
        return sqlite3_errmsg(db.db);
    }
    if(SQLITE_OK != sqlite3_bind_text(db.insert_stroke_stmt,
                                       3, command, -1,
                                       SQLITE_STATIC)) {
        return sqlite3_errmsg(db.db);
    }
    if(SQLITE_OK != sqlite3_bind_int(db.insert_stroke_stmt,
                                      4, stroke->n)) {
        return sqlite3_errmsg(db.db);
    }
    if(SQLITE_OK != sqlite3_bind_blob(db.insert_stroke_stmt,
                                      5, stroke->p, sizeof(double)*stroke->n,
                                      SQLITE_STATIC)) {
        return sqlite3_errmsg(db.db);
    }

    if(SQLITE_DONE != sqlite3_step(db.insert_stroke_stmt)) {
        return "step was not done?";
    }
    return NULL;
}
