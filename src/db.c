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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sysexits.h>
#include <time.h>

#include "db.h"
#include <sqlite3.h>
#include "util.h"

struct _Database {
    sqlite3 *db;
    sqlite3_stmt *insert_gesture_stmt;
    sqlite3_stmt *load_gestures_stmt;
    sqlite3_stmt *load_gesture_with_id_stmt;
    sqlite3_stmt *delete_gesture_with_id_stmt;
};

static const char *schema =
    "CREATE TABLE IF NOT EXISTS gesture (                               \
         id INTEGER PRIMARY KEY,                                        \
         time INTEGER NOT NULL,                                         \
         description TEXT NOT NULL,                                     \
         command TEXT NOT NULL,                                         \
         points BLOB NOT NULL                                           \
    );";

static const char *insert_gesture_sql =
    "INSERT INTO gesture (time, description, command, points)  \
        VALUES (?, ?, ?, ?);";

static const char *load_gestures_sql =
    "SELECT id, points, description, command FROM gesture ORDER BY id ASC;";

static const char *load_gesture_with_id_sql =
    "SELECT id, points, description, command FROM gesture WHERE id = ? LIMIT 1;";

static const char *delete_gesture_with_id_sql =
    "DELETE FROM gesture WHERE id = ?;";


/* Opens the database, creating it and the configuration directory if necessary.
   Returns the database handle and sets error to NULL if successful, otherwise
   error is set to an appropriate error message. */
Database *
database_open(/* out */ const char **error) {
    assert(error);
    Database *db = malloc(sizeof(Database));
    if (db == NULL)
        err(EX_OSERR, "malloc");

    *error = NULL;

    char *confdir = config_dir();
    if (!mkdirs(confdir)) {
        *error = "Could not create config directory";
        goto error;
    }

    char *dbpath;
    if (asprintf(&dbpath, "%s/simplestroke.sqlite", confdir) < 0) {
        *error = strerror(errno);
        free(confdir);
        goto error;
    }

    free(confdir);

    const int status
        = sqlite3_open_v2(dbpath, &db->db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                          NULL);
    free(dbpath);
    if (SQLITE_OK != status) {
        *error = sqlite3_errstr(status);
        goto error;
    }

    if (SQLITE_OK != sqlite3_exec(db->db, schema, NULL, NULL, NULL)) {
        sqlite3_close(db->db);
        *error = "could not create schema";
        goto error;
    }

    if (SQLITE_OK != sqlite3_prepare_v2(db->db,
                                        insert_gesture_sql,
                                        -1,
                                        &db->insert_gesture_stmt,
                                        NULL)) {
        *error = sqlite3_errstr(sqlite3_extended_errcode(db->db));
        goto error;
    }

    if (SQLITE_OK != sqlite3_prepare_v2(db->db,
                                        load_gestures_sql,
                                        -1,
                                        &db->load_gestures_stmt,
                                        NULL)) {
        *error = sqlite3_errstr(sqlite3_extended_errcode(db->db));
        goto error;
    }

    if (SQLITE_OK != sqlite3_prepare_v2(db->db,
                                        load_gesture_with_id_sql,
                                        -1,
                                        &db->load_gesture_with_id_stmt,
                                        NULL)) {
        *error = sqlite3_errstr(sqlite3_extended_errcode(db->db));
        goto error;
    }

    if (SQLITE_OK != sqlite3_prepare_v2(db->db,
                                        delete_gesture_with_id_sql,
                                        -1,
                                        &db->delete_gesture_with_id_stmt,
                                        NULL)) {
        *error = sqlite3_errstr(sqlite3_extended_errcode(db->db));
        goto error;
    }

    return db;

error:
    if (db->insert_gesture_stmt)
        sqlite3_finalize(db->insert_gesture_stmt);
    if (db->load_gestures_stmt)
        sqlite3_finalize(db->load_gestures_stmt);
    if (db->load_gesture_with_id_stmt)
        sqlite3_finalize(db->load_gesture_with_id_stmt);
    if (db->delete_gesture_with_id_stmt)
        sqlite3_finalize(db->delete_gesture_with_id_stmt);
    if (db->db)
        sqlite3_close(db->db);
    free(db);
    return NULL;
}

const char *
database_close(Database *db) {
    if (!db)
        return NULL;

    if (db->insert_gesture_stmt)
        sqlite3_finalize(db->insert_gesture_stmt);
    if (db->load_gestures_stmt)
        sqlite3_finalize(db->load_gestures_stmt);
    if (db->load_gesture_with_id_stmt)
        sqlite3_finalize(db->load_gesture_with_id_stmt);
    if (db->delete_gesture_with_id_stmt)
        sqlite3_finalize(db->delete_gesture_with_id_stmt);

    const int status = sqlite3_close_v2(db->db);
    free(db);
    db = NULL;
    if (SQLITE_OK == status)
        return NULL;

    return sqlite3_errstr(status);
}

const char *
database_add_gesture(Database *db,
                     stroke_t *stroke,
                     const char *description,
                     const char *command) {
    assert(db);
    assert(stroke);
    assert(description);
    assert(command);

    sqlite3_clear_bindings(db->insert_gesture_stmt);
    sqlite3_reset(db->insert_gesture_stmt);

    if (SQLITE_OK != sqlite3_bind_int(db->insert_gesture_stmt,
                                      1, time(NULL)))
        return sqlite3_errstr(sqlite3_extended_errcode(db->db));
    if (SQLITE_OK != sqlite3_bind_text(db->insert_gesture_stmt,
                                       2, description, -1,
                                       SQLITE_STATIC))
        return sqlite3_errstr(sqlite3_extended_errcode(db->db));
    if (SQLITE_OK != sqlite3_bind_text(db->insert_gesture_stmt,
                                       3, command, -1,
                                       SQLITE_STATIC))
        return sqlite3_errstr(sqlite3_extended_errcode(db->db));
    if (SQLITE_OK != sqlite3_bind_blob(db->insert_gesture_stmt,
                                       4, stroke->p, sizeof(point)*stroke->n,
                                       SQLITE_STATIC))
        return sqlite3_errstr(sqlite3_extended_errcode(db->db));

    sqlite3_reset(db->insert_gesture_stmt);

    if (SQLITE_DONE != sqlite3_step(db->insert_gesture_stmt))
        return "step was not done?";

    return NULL;
}

static void
_database_load_gesture(sqlite3_stmt *stmt,
                       int *id,
                       stroke_t *stroke,
                       char **description,
                       char **command) {
    assert(stmt);

    assert(sqlite3_column_type(stmt, 0) == SQLITE_INTEGER);
    assert(sqlite3_column_type(stmt, 1) == SQLITE_BLOB);
    assert(sqlite3_column_type(stmt, 2) == SQLITE_TEXT);
    assert(sqlite3_column_type(stmt, 3) == SQLITE_TEXT);

    if (id)
        *id = sqlite3_column_int(stmt, 0);

    if (stroke) {
        point *points = (point *)sqlite3_column_blob(stmt, 1);
        int n = sqlite3_column_bytes(stmt, 1);
        memcpy(stroke->p, points, n);
        stroke->n = n / sizeof(point);
    }


    if (description)
        *description = strdup((char *)sqlite3_column_text(stmt, 2));
    if (command)
        *command = strdup((char *)sqlite3_column_text(stmt, 3));
}

const char *
database_load_gestures(Database *db,
                       LoadGesturesCallback cb,
                       const void *user_data) {
    assert(db);

    sqlite3_reset(db->load_gestures_stmt);

    while (cb) {
        int status = sqlite3_step(db->load_gestures_stmt);
        stroke_t stroke = {};
        int id = -1;
        char *description = NULL;
        char *command = NULL;

        switch (status) {
        case SQLITE_DONE:
            sqlite3_reset(db->load_gestures_stmt);
            return NULL;
        case SQLITE_ROW:
            _database_load_gesture(db->load_gestures_stmt,
                                   &id,
                                   &stroke,
                                   &description,
                                   &command);
            cb(&stroke, id, description, command, user_data);
            break;
        default:
            return sqlite3_errstr(status);
        }
    }

    return NULL;
}

const char *
database_load_gesture_with_id(Database *db,
                              int id,
                              stroke_t *stroke,
                              char **description,
                              char **command) {
    assert(db);

    sqlite3_clear_bindings(db->load_gesture_with_id_stmt);
    sqlite3_reset(db->load_gesture_with_id_stmt);

    if (SQLITE_OK != sqlite3_bind_int(db->load_gesture_with_id_stmt, 1, id))
        return sqlite3_errstr(sqlite3_extended_errcode(db->db));

    int status = sqlite3_step(db->load_gesture_with_id_stmt);
    if (status != SQLITE_ROW)
        return "not found";

    _database_load_gesture(db->load_gesture_with_id_stmt,
                           NULL,
                           stroke,
                           description,
                           command);

    status = sqlite3_step(db->load_gesture_with_id_stmt);
    if (status != SQLITE_DONE) {
        sqlite3_reset(db->load_gesture_with_id_stmt);
        return "more than one gesture found?";
    }

    sqlite3_reset(db->load_gesture_with_id_stmt);
    return NULL;
}

const char *
database_delete_gesture_with_id(Database *db, int id) {
    sqlite3_clear_bindings(db->delete_gesture_with_id_stmt);
    sqlite3_reset(db->delete_gesture_with_id_stmt);

    if (SQLITE_OK != sqlite3_bind_int(db->delete_gesture_with_id_stmt, 1, id))
        return sqlite3_errstr(sqlite3_extended_errcode(db->db));

    int status = sqlite3_step(db->delete_gesture_with_id_stmt);
    if (status != SQLITE_DONE)
        return "not found";

    sqlite3_reset(db->delete_gesture_with_id_stmt);
    return NULL;
}
