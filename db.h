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
#ifndef __DB_H__
#define __DB_H__

#include <sqlite3.h>

#include "stroke.h"

typedef struct {
    sqlite3* db;
    sqlite3_stmt* insert_gesture_stmt;
    sqlite3_stmt* load_gestures_stmt;
    sqlite3_stmt* command_for_gesture_stmt;
} Database;

typedef void (*LoadGesturesCallback)(stroke_t* stroke, const char* description, const char* command, const void* user_data);

void database_uri(char* path, size_t len);
const char* database_open(/* out */ Database* db);
const char* database_close(Database db);
const char* database_add_gesture(Database db, stroke_t* stroke, const char* description, const char* command);
const char* database_load_gestures(Database db, LoadGesturesCallback cb, const void* user_data);

#endif
