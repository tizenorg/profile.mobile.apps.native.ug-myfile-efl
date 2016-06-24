/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <sqlite3.h>
#include <string.h>
#include <assert.h>
#include <Eina.h>
#include <unistd.h>

#include "mf-ug-media-error.h"
#include "mf-ug-media-types.h"

#include "mf-ug-media.h"
#include "mf-ug-media-db.h"
#include "mf-ug-dlog.h"


#define MF_DB_NAME "/opt/usr/apps/org.tizen.myfile/data/.myfile_media.db"

#define MF_PRAGMA_FOREIGN_KEYS_ON 			"PRAGMA foreign_keys = ON;"

/************* Shortcut ************/
#define MF_INSERT_INTO_SHORTCUT_TABLE 			"INSERT INTO %s (%s, %s, %s) VALUES ('%q', %Q, %d);"
#define MF_DELETE_FROM_SHORTCUT_TABLE 				"DELETE FROM %s WHERE %s = '%q';"
#define MF_DELETE_BY_TYPE_FROM_SHORTCUT_TABLE 				"DELETE FROM %s WHERE %s = %d;"
#define MF_SELECT_SHORTCUT_TABLE				"SELECT * FROM %s;"
#define MF_SELECT_SHORTCUT_COUNT_TABLE 				"SELECT count(*) FROM %s;"

/************ Recent files ***********/
#define MF_INSERT_INTO_RECENT_FILES_TABLE 		"INSERT INTO %s (%s, %s, %s, %s) VALUES (?, ?, ?, ?);"
#define MF_DELETE_FROM_RECENT_FILES_TABLE 				"DELETE FROM %s WHERE %s = '%q';"
#define MF_DELETE_BY_TYPE_FROM_RECENT_FILES_TABLE 				"DELETE FROM %s WHERE %s = %d;"
#define MF_UPDATE_SET_RECENT_FILES_TABLE	 		"UPDATE %s SET %s = '%q' WHERE (%s = '%q');"
#define MF_UPDATE_FAVORATE_FILES_TABLE	 		"UPDATE %s SET %s = '%q' WHERE (%s = '%q');"
#define MF_SELECT_RECENT_FILES_TABLE			"SELECT * FROM %s;"
#define MF_SELECT_RECENT_FILES_COUNT_TABLE 				"SELECT count(*) FROM %s;"

#define MF_DELETE_ALL_FROM_TABLE 			"DELETE FROM %s;"

/************ ringtone ****************/
#define MF_INSERT_INTO_RINGTONE_TABLE 			"INSERT INTO %s (%s, %s, %s) VALUES ('%q', %Q, %d);"
#define MF_DELETE_FROM_RINGTONE_TABLE 				"DELETE FROM %s WHERE %s = '%q';"
#define MF_DELETE_BY_TYPE_FROM_RINGTONE_TABLE 				"DELETE FROM %s WHERE %s = %d;"
#define MF_SELECT_RINGTONE_TABLE				"SELECT * FROM %s;"
#define MF_SELECT_RINGTONE_COUNT_TABLE 				"SELECT count(*) FROM %s;"
#define MF_FIND_RINGTONE_TABLE				"SELECT * FROM %s WHERE %s = '%q';"

/************** alert *****************/
#define MF_INSERT_INTO_ALERT_TABLE 			"INSERT INTO %s (%s, %s, %s) VALUES ('%q', %Q, %d);"
#define MF_DELETE_FROM_ALERT_TABLE 				"DELETE FROM %s WHERE %s = '%q';"
#define MF_DELETE_BY_TYPE_FROM_ALERT_TABLE 				"DELETE FROM %s WHERE %s = %d;"
#define MF_SELECT_ALERT_TABLE				"SELECT * FROM %s;"
#define MF_SELECT_ALERT_COUNT_TABLE 				"SELECT count(*) FROM %s;"
#define MF_FIND_ALERT_TABLE				"SELECT * FROM %s WHERE %s = '%q';"

typedef enum {
    MF_TABLE_NONE = -1,
    MF_TABLE_SHORTCUT,
    MF_TABLE_RECENT_FILES,
    MF_TABLE_RINGTONE,
    MF_TABLE_ALERT,
    MF_TABLE_NUM,
} mf_tbl_name_e;

typedef enum {
    MF_FIELD_SHORTCUT_NONE		= -1,
    MF_FIELD_SHORTCUT_PATH,
    MF_FIELD_SHORTCUT_NAME,
    MF_FIELD_SHORTCUT_STORAGE_TYPE,
    MF_FIELD_SHORTCUT_NUM,
} mf_field_shortcut_e;

typedef enum {
    MF_FIELD_RECENT_FILES_NONE		= -1,
    MF_FIELD_RECENT_FILES_PATH,
    MF_FIELD_RECENT_FILES_NAME,
    MF_FIELD_RECENT_FILES_STORAGE_TYPE,
    MF_FIELD_RECENT_FILES_THUMBNAIL,
    MF_FIELD_RECENT_FILES_NUM,
} mf_field_recent_files_e;

typedef enum {
    MF_FIELD_RINGTONE_NONE		= -1,
    MF_FIELD_RINGTONE_PATH,
    MF_FIELD_RINGTONE_NAME,
    MF_FIELD_RINGTONE_STORAGE_TYPE,
    MF_FIELD_RINGTONE_NUM,
} mf_field_ringtone_e;

typedef enum {
    MF_FIELD_ALERT_NONE		= -1,
    MF_FIELD_ALERT_PATH,
    MF_FIELD_ALERT_NAME,
    MF_FIELD_ALERT_STORAGE_TYPE,
    MF_FIELD_ALERT_NUM,
} mf_field_alert_e;

typedef struct {
	char *field_name;
	char *field_type;
} mf_tbl_field_s;

typedef struct {
	char *table_name;
	mf_tbl_field_s mf_tbl_field[MF_FIELD_RECENT_FILES_NUM + 1];
} mf_tbl_s;

mf_tbl_s mf_tbl[MF_TABLE_NUM] = {
	{
		"shortcut", {
			{"path", ""}	/* PK */
			,
			{"name", ""}	/* PK */
			,
			{"storage_type", ""}	/* PK */
		}
	}
	,
	{
		"recent_files", {
			{"path", ""}	/* PK */
			,
			{"name", ""}
			,
			{"storage_type", ""}
			,
			{"thumbnail_path", ""}
		}

	}
	,
	{
		"ringtone", {
			{"path", ""}	/* PK */
			,
			{"name", ""}	/* PK */
			,
			{"storage_type", ""}	/* PK */
		}
	}
	,
	{
		"alert", {
			{"path", ""}	/* PK */
			,
			{"name", ""}	/* PK */
			,
			{"storage_type", ""}	/* PK */
		}
	}
};

#if 0
static int __mf_busy_handler(void *pData, int count)
{
	usleep(50000);

	ug_debug("web_media_busy_handler called : %d", count);

	return 100 - count;
}
#endif

int mf_sqlite3_exec(
    sqlite3 *p_db,                                  /* An open database */
    const char *sql,                           /* SQL to be evaluated */
    int (*callback)(void*, int, char**, char**),  /* Callback function */
    void *params,                                    /* 1st argument to callback */
    char **errmsg                              /* Error msg written here */
)
{
	ug_debug("mf_sqlite3_exec enter\n");
	sqlite3_stmt *p_statement = NULL;
	int result = sqlite3_prepare_v2(p_db, sql, -1, &p_statement, NULL);
	if (result != SQLITE_OK) {
		ug_debug("sqlite3_prepare_v2 error result=%d", result);
		return result;
	}
	result = sqlite3_step(p_statement);

	result = sqlite3_finalize(p_statement);
	if (result != SQLITE_OK) {
		ug_debug("sqlite3_finalize error result=%d", result);
	}
	ug_debug("mf_sqlite3_exec leave result=%d", result);
	return result;
}


static int __mf_sqlite3_commit_trans(MFDHandle *mfd_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		ug_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	ug_debug("gm_sqlite3_commit_trans enter\n");
	if (SQLITE_OK != mf_sqlite3_exec(handle, "COMMIT;", NULL, NULL, &err_msg)) {
		if (err_msg) {
			ug_debug("Error:failed to end transaction: error=%s\n",
			         err_msg);
			sqlite3_free(err_msg);
		}
		return MFD_ERROR_DB_INTERNAL;
	}
	if (err_msg) {
		sqlite3_free(err_msg);
	}
	ug_debug("gm_sqlite3_commit_trans leave\n");
	return 0;
}

static int __mf_query_bind_text(sqlite3_stmt *stmt, int pos, const char *str)
{
	assert(NULL != stmt);

	if (str) {
		return sqlite3_bind_text(stmt, pos, (const char *)str, strlen(str), SQLITE_STATIC);
	} else {
		return sqlite3_bind_null(stmt, pos);
	}
}

static int __mf_query_bind_int(sqlite3_stmt *stmt, int pos, int num)
{
	assert(NULL != stmt);
	assert(pos > -1);
	return sqlite3_bind_int(stmt, pos, num);
}

static char *__mf_query_table_column_text(sqlite3_stmt *stmt, int pos)
{
	assert(NULL != stmt);
	assert(pos > -1);
	return (char *)sqlite3_column_text(stmt, pos);
}

static int __mf_query_table_column_int(sqlite3_stmt *stmt, int pos)
{
	assert(NULL != stmt);
	assert(pos > -1);
	return sqlite3_column_int(stmt, pos);
}

static void __mf_data_to_text(char *textbuf, char **output)
{
	if (textbuf && strlen(textbuf) > 0) {
		if (*output) {
			free(*output);
			*output = NULL;
		}
		*output = strdup(textbuf);
	}
}


static int __mf_query_sql(MFDHandle *mfd_handle, char *query_str)
{
	int err = -1;
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		ug_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	ug_debug("SQL = %s\n", query_str);

	err = mf_sqlite3_exec(handle, query_str, NULL, NULL, &err_msg);
	if (SQLITE_OK != err) {
		if (err_msg) {
			ug_debug("failed to query[%s]", err_msg);
			sqlite3_free(err_msg);
		}
		ug_debug("Query fails : query_string[%s]", query_str);
		return MFD_ERROR_DB_INTERNAL;
	}

	if (err_msg) {
		sqlite3_free(err_msg);
	}
	ug_debug("query success\n");

	return err;
}

static int __mf_sqlite3_begin_trans(MFDHandle *mfd_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		ug_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	ug_debug("gm_sqlite3_begin_trans enter\n");
	if (SQLITE_OK !=
	        mf_sqlite3_exec(handle, "BEGIN IMMEDIATE;", NULL, NULL, &err_msg)) {
		if (err_msg) {
			ug_debug("Error:failed to begin transaction: error=%s\n",
			         err_msg);
			sqlite3_free(err_msg);
		}
		return MFD_ERROR_DB_INTERNAL;
	}
	if (err_msg) {
		sqlite3_free(err_msg);
	}
	ug_debug("gm_sqlite3_begin_trans leave\n");
	return 0;
}

static int __mf_sqlite3_rollback_trans(MFDHandle *mfd_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		ug_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	ug_debug("gm_sqlite3_rollback_trans enter\n");
	if (SQLITE_OK !=
	        mf_sqlite3_exec(handle, "ROLLBACK;", NULL, NULL, &err_msg)) {
		if (err_msg) {
			ug_debug("Error:failed to rollback transaction: error=%s\n",
			         err_msg);
			sqlite3_free(err_msg);
		}
		return MFD_ERROR_DB_INTERNAL;
	}
	if (err_msg) {
		sqlite3_free(err_msg);
	}
	ug_debug("gm_sqlite3_rollback_trans leave\n");
	return 0;
}


static void __mf_convert_recent_files_column_to_citem(sqlite3_stmt *stmt, MFRitem *ritem)
{
	char *textbuf = NULL;

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_RECENT_FILES_PATH);
	__mf_data_to_text(textbuf, &(ritem->path));

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_RECENT_FILES_NAME);
	__mf_data_to_text(textbuf, &(ritem->name));

	ritem->storyage_type = __mf_query_table_column_int(stmt, MF_FIELD_RECENT_FILES_STORAGE_TYPE);

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_RECENT_FILES_THUMBNAIL);
	__mf_data_to_text(textbuf, &(ritem->thumbnail));

}

static void __mf_foreach_recent_files_ritem_cb(mf_recent_files_item_cb callback, void *data, void *user_data)
{
	Eina_List *list = (Eina_List *)data;
	Eina_List *iter = NULL;

	for (iter = list; iter != NULL; iter = eina_list_next(iter)) {
		MFRitem *ritem = NULL;
		ritem = (MFRitem *)iter->data;

		if (callback(ritem, user_data) == FALSE) {
			break;
		}
	}
}

static void __mf_free_recent_files_list(void *data)
{
	mf_ug_destroy_recent_files_item(data);
}

static void __mf_media_db_eina_list_free_full(Eina_List **list, void (*func)(void *data))
{
	ug_mf_retm_if(*list == NULL, "list is NULL");

	void *pNode = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(*list, l, pNode) {
		func(pNode);
	}
	eina_list_free(*list);
	*list = NULL;
}

int mf_ug_connect_db_with_handle(sqlite3 **db_handle)
{
	int ret = MFD_ERROR_NONE;

	if (db_handle == NULL) {
		ug_debug("error invalid arguments");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	/*Connect DB*/
	ret = sqlite3_open(MF_DB_NAME, db_handle);
	if (SQLITE_OK != ret) {

		ug_debug("error when db open");
		*db_handle = NULL;
		return MFD_ERROR_DB_CONNECT;
	}

	/*Register busy handler*/
	if (*db_handle) {
		ret = sqlite3_busy_handler(*db_handle, NULL, NULL);
		if (SQLITE_OK != ret) {

			if (*db_handle) {
				ug_debug("[error when register busy handler] %s\n", sqlite3_errmsg(*db_handle));
			}

			ret = sqlite3_close(*db_handle);
			*db_handle = NULL;

			return MFD_ERROR_DB_CONNECT;
		}

		/* set foreign_keys */
		char *query_string = NULL;
		query_string =
				sqlite3_mprintf(MF_PRAGMA_FOREIGN_KEYS_ON);

		ug_debug("Query : %s", query_string);

		ret = __mf_query_sql(*db_handle, query_string);
		sqlite3_free(query_string);
	} else {
		ug_debug("invalid parameter");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_disconnect_db_with_handle(sqlite3 *db_handle)
{
	int ret = MFD_ERROR_NONE;

	ret = sqlite3_close(db_handle);
	if (SQLITE_OK != ret) {
		ug_debug("error when db close");
		ug_debug("Error : %s", sqlite3_errmsg(db_handle));
		db_handle = NULL;
		return MFD_ERROR_DB_DISCONNECT;
	}

	return MFD_ERROR_NONE;
}

/*1 Shortcut*/

static void __mf_convert_shortcut_column_to_sitem(sqlite3_stmt *stmt, MFSitem *sitem)
{
	char *textbuf = NULL;

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_SHORTCUT_PATH);
	__mf_data_to_text(textbuf, &(sitem->path));
}

static void __mf_foreach_shortcut_sitem_cb(mf_shortcut_item_cb callback, void *data, void *user_data)
{
	Eina_List *list = (Eina_List *)data;
	Eina_List *iter = NULL;

	for (iter = list; iter != NULL; iter = eina_list_next(iter)) {
		MFSitem *sitem = NULL;
		sitem = (MFSitem *)iter->data;

		if (callback(sitem, user_data) == FALSE) {
			break;
		}
	}
}

static void __mf_free_shortcut_list(void *data)
{
	mf_ug_destroy_shortcut_item(data);
}

int mf_ug_update_shortcut(MFDHandle *mfd_handle, const char *new_name, char *old_name)
{
	if (new_name == NULL) {
		ug_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ug_error("mf_ug_update_shortcut");
	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_UPDATE_FAVORATE_FILES_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    /*mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,*/
	                    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
	                    new_name,
	                    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
	                    old_name
	                   );

	ug_error("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Inserting device table failed\n");
		ug_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}



int mf_ug_insert_shortcut(MFDHandle *mfd_handle, const char *shortcut_path, const char *shortcut_name, int storage_type)
{
	ug_debug("");

	if (shortcut_path == NULL) {
		ug_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_INSERT_INTO_SHORTCUT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
	                    mf_tbl_field[MF_FIELD_SHORTCUT_NAME].field_name,
	                    mf_tbl_field[MF_FIELD_SHORTCUT_STORAGE_TYPE].field_name,
	                    shortcut_path,
	                    shortcut_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Inserting device table failed\n");
		ug_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_delete_shortcut(MFDHandle *mfd_handle, const char *shortcut_path)
{
	ug_debug("");

	if (shortcut_path == NULL) {
		ug_debug("shortcut_path is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_FROM_SHORTCUT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
	                    shortcut_path);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_delete_shortcut_by_type(MFDHandle *mfd_handle, int storage_type)
{
	ug_debug("");

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_BY_TYPE_FROM_SHORTCUT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_SHORTCUT_STORAGE_TYPE].field_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_foreach_shortcut_list(MFDHandle *mfd_handle, mf_shortcut_item_cb callback, void *user_data)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_SHORTCUT_TABLE,
	                    mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return MFD_ERROR_DB_NO_RECORD;
	}

	Eina_List *shortcut_list = NULL;
	MFSitem *sitem = NULL;

	while (SQLITE_ROW == rc) {
		sitem = (MFSitem *)calloc(1, sizeof(MFSitem));
		if (!sitem) {
			ug_debug("allocation failed");
			return MFD_ERROR_DB_INTERNAL;
		}
		__mf_convert_shortcut_column_to_sitem(stmt, sitem);
		shortcut_list = eina_list_append(shortcut_list, sitem);
		rc = sqlite3_step(stmt);
		ug_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	__mf_foreach_shortcut_sitem_cb(callback, shortcut_list, user_data);

	if (shortcut_list) {
		__mf_media_db_eina_list_free_full(&shortcut_list, __mf_free_shortcut_list);
	}

	return MFD_ERROR_NONE;
}

int mf_ug_get_short_count(MFDHandle *mfd_handle, int *count)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_SHORTCUT_COUNT_TABLE,
	                    mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		*count = 0;
		return MFD_ERROR_DB_NO_RECORD;
	}

	*count = sqlite3_column_int(stmt, 0);
	ug_debug("count : %d", *count);

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	return MFD_ERROR_NONE;
}

/*1 Recent files*/



int mf_ug_insert_recent_file(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, const char *thumbnail_path)
{
	ug_debug("");
	ug_mf_retvm_if(path == NULL, MFD_ERROR_INVALID_PARAMETER, "path is NULL");
	/*mf_retvm_if (name == NULL, MFD_ERROR_INVALID_PARAMETER, "path is NULL");
	  mf_retvm_if (thumbnail_path == NULL, MFD_ERROR_INVALID_PARAMETER, "path is NULL");*/


	sqlite3_stmt *stmt = NULL;
	int err = -1;

	char query_string[255];
	memset(query_string, 0, sizeof(query_string));
	mf_tbl_field_s *mf_tbl_field;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	snprintf(query_string, sizeof(query_string), MF_INSERT_INTO_RECENT_FILES_TABLE,
	         mf_tbl[field_seq].table_name,
	         mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
	         mf_tbl_field[MF_FIELD_RECENT_FILES_NAME].field_name,
	         mf_tbl_field[MF_FIELD_RECENT_FILES_STORAGE_TYPE].field_name,
	         mf_tbl_field[MF_FIELD_RECENT_FILES_THUMBNAIL].field_name);

	err = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	if (err != SQLITE_OK) {
		ug_debug("sqlite3_prepare_v2");
		goto INSERT_FAIL;
	}

	__mf_query_bind_text(stmt, 1, path);
	__mf_query_bind_text(stmt, 2, name);
	__mf_query_bind_int(stmt, 3, storage_type);
	__mf_query_bind_text(stmt, 4, thumbnail_path);

INSERT_FAIL:
	err = sqlite3_step(stmt);
	if (err != SQLITE_DONE) {
		SECURE_DEBUG("Inserting content table failed. %s", sqlite3_errmsg(mfd_handle));
		if (SQLITE_OK != sqlite3_finalize(stmt)) {
			ug_debug("sqlite3_finalize failed");
		}
		return MFD_ERROR_DB_INTERNAL;
	}

	if (SQLITE_OK != sqlite3_finalize(stmt)) {
		ug_debug("sqlite3_finalize failed");
	}
	ug_debug("Query : %s", query_string);

	return MFD_ERROR_NONE;
}

int mf_ug_delete_recent_files(MFDHandle *mfd_handle, const char *path)
{
	ug_debug("");

	if (path == NULL) {
		ug_debug("shortcut_path is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_FROM_RECENT_FILES_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
	                    path);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type)
{
	ug_debug("");

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_BY_TYPE_FROM_RECENT_FILES_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RECENT_FILES_STORAGE_TYPE].field_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail)
{
	ug_debug("");

	if (thumbnail == NULL) {
		ug_debug("thumbnail is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (new_thumbnail == NULL) {
		ug_debug("new_thumbnail is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;

	mf_tbl_field_s *mf_tbl_field;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	char *query_string = NULL;

	query_string =
	    sqlite3_mprintf(MF_UPDATE_SET_RECENT_FILES_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RECENT_FILES_THUMBNAIL].field_name,
	                    new_thumbnail,
	                    mf_tbl_field[MF_FIELD_RECENT_FILES_THUMBNAIL].field_name,
	                    thumbnail);

	ug_debug("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Updating content table failed");
		ug_debug("query string is %s", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}



int mf_ug_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string = sqlite3_mprintf(MF_SELECT_RECENT_FILES_TABLE, mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return MFD_ERROR_DB_NO_RECORD;
	}

	Eina_List *recent_files_list = NULL;
	MFRitem *ritem = NULL;

	while (SQLITE_ROW == rc) {
		ritem = (MFRitem *)calloc(1, sizeof(MFRitem));
		if (!ritem) {
			ug_debug("allocation failed");
			return MFD_ERROR_DB_INTERNAL;
		}

		__mf_convert_recent_files_column_to_citem(stmt, ritem);
		recent_files_list = eina_list_append(recent_files_list, ritem);
		rc = sqlite3_step(stmt);
		ug_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	__mf_foreach_recent_files_ritem_cb(callback, recent_files_list, user_data);

	if (recent_files_list) {
		__mf_media_db_eina_list_free_full(&recent_files_list, __mf_free_recent_files_list);
	}

	return MFD_ERROR_NONE;
}


int mf_ug_get_recent_files_count(MFDHandle *mfd_handle, int *count)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string = sqlite3_mprintf(MF_SELECT_RECENT_FILES_COUNT_TABLE, mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		*count = 0;
		return MFD_ERROR_DB_NO_RECORD;
	}

	*count = sqlite3_column_int(stmt, 0);
	ug_debug("count : %d", *count);

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	return MFD_ERROR_NONE;
}

/*1 Ringtones*/

static void __mf_convert_ringtone_column_to_ringtone_item(sqlite3_stmt *stmt, mfRingtone *ritem)
{
	char *textbuf = NULL;

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_SHORTCUT_PATH);
	__mf_data_to_text(textbuf, &(ritem->path));
}

static void __mf_foreach_ringtone_item_cb(mf_ringtone_item_cb callback, void *data, void *user_data)
{
	Eina_List *list = (Eina_List *)data;
	Eina_List *iter = NULL;

	for (iter = list; iter != NULL; iter = eina_list_next(iter)) {
		mfRingtone *ritem = NULL;
		ritem = (mfRingtone *)iter->data;

		if (callback(ritem, user_data) == FALSE) {
			break;
		}
	}
}

static void __mf_free_ringtone_list(void *data)
{
	mf_ug_destroy_ringtone_item(data);
}

int mf_ug_update_ringtone(MFDHandle *mfd_handle, const char *new_name, char *old_name)
{
	if (new_name == NULL) {
		ug_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ug_error("mf_ug_update_shortcut");
	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_UPDATE_FAVORATE_FILES_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    /*mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,*/
	                    mf_tbl_field[MF_FIELD_RINGTONE_PATH].field_name,
	                    new_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_PATH].field_name,
	                    old_name
	                   );

	ug_error("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Inserting device table failed\n");
		ug_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}



int mf_ug_insert_ringtone(MFDHandle *mfd_handle, const char *ringtone_path, const char *ringtone_name, int storage_type)
{
	ug_debug("");

	if (ringtone_path == NULL) {
		ug_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_INSERT_INTO_RINGTONE_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_PATH].field_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_NAME].field_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_STORAGE_TYPE].field_name,
	                    ringtone_path,
	                    ringtone_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Inserting device table failed\n");
		ug_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_delete_ringtone(MFDHandle *mfd_handle, const char *ringtone_path)
{
	ug_debug("");

	if (ringtone_path == NULL) {
		ug_debug("shortcut_path is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_FROM_RINGTONE_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_PATH].field_name,
	                    ringtone_path);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_find_ringtone(MFDHandle *mfd_handle, const char *ringtone_path)
{
	ug_debug("");

	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;
	int find = 0;
	query_string =
	    sqlite3_mprintf(MF_FIND_RINGTONE_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_PATH].field_name,
	                    ringtone_path);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return find;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return find;
	}


	while (SQLITE_ROW == rc) {

		find = 1;
		rc = sqlite3_step(stmt);
		ug_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}
	return find;
}



int mf_ug_delete_ringtone_by_type(MFDHandle *mfd_handle, int storage_type)
{
	ug_debug("");

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_BY_TYPE_FROM_RINGTONE_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_RINGTONE_STORAGE_TYPE].field_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_foreach_ringtone_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_RINGTONE_TABLE,
	                    mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return MFD_ERROR_DB_NO_RECORD;
	}

	Eina_List *ringtone_list = NULL;
	mfRingtone *ritem = NULL;

	while (SQLITE_ROW == rc) {
		ritem = (mfRingtone *)calloc(1, sizeof(mfRingtone));
		if (ritem) {
			__mf_convert_ringtone_column_to_ringtone_item(stmt, ritem);
			ringtone_list = eina_list_append(ringtone_list, ritem);
		}
		rc = sqlite3_step(stmt);
		ug_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	__mf_foreach_ringtone_item_cb(callback, ringtone_list, user_data);

	if (ringtone_list) {
		__mf_media_db_eina_list_free_full(&ringtone_list, __mf_free_ringtone_list);
	}

	return MFD_ERROR_NONE;
}

int mf_ug_get_ringtone_count(MFDHandle *mfd_handle, int *count)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RINGTONE;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_RINGTONE_COUNT_TABLE,
	                    mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		*count = 0;
		return MFD_ERROR_DB_NO_RECORD;
	}

	*count = sqlite3_column_int(stmt, 0);
	ug_debug("count : %d", *count);

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	return MFD_ERROR_NONE;
}


int mf_ug_update_alert(MFDHandle *mfd_handle, const char *new_name, char *old_name)
{
	if (new_name == NULL) {
		ug_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ug_error("mf_ug_update_shortcut");
	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_UPDATE_FAVORATE_FILES_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    /*mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,*/
	                    mf_tbl_field[MF_FIELD_ALERT_PATH].field_name,
	                    new_name,
	                    mf_tbl_field[MF_FIELD_ALERT_PATH].field_name,
	                    old_name
	                   );

	ug_error("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Inserting device table failed\n");
		ug_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}



int mf_ug_insert_alert(MFDHandle *mfd_handle, const char *alert_path, const char *alert_name, int storage_type)
{
	ug_debug("");

	if (alert_path == NULL) {
		ug_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_INSERT_INTO_ALERT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_ALERT_PATH].field_name,
	                    mf_tbl_field[MF_FIELD_ALERT_NAME].field_name,
	                    mf_tbl_field[MF_FIELD_ALERT_STORAGE_TYPE].field_name,
	                    alert_path,
	                    alert_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("Inserting device table failed\n");
		ug_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_delete_alert(MFDHandle *mfd_handle, const char *alert_path)
{
	ug_debug("");

	if (alert_path == NULL) {
		ug_debug("shortcut_path is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_FROM_ALERT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_ALERT_PATH].field_name,
	                    alert_path);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_find_alert(MFDHandle *mfd_handle, const char *alert_path)
{
	ug_debug("");

	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;
	int find = 0;
	query_string =
	    sqlite3_mprintf(MF_FIND_ALERT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_ALERT_PATH].field_name,
	                    alert_path);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return find;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return find;
	}


	while (SQLITE_ROW == rc) {

		find = 1;
		rc = sqlite3_step(stmt);
		ug_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}
	return find;
}



int mf_ug_delete_alert_by_type(MFDHandle *mfd_handle, int storage_type)
{
	ug_debug("");

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_BY_TYPE_FROM_ALERT_TABLE,
	                    mf_tbl[field_seq].table_name,
	                    mf_tbl_field[MF_FIELD_ALERT_STORAGE_TYPE].field_name,
	                    storage_type);

	ug_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		ug_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		ug_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_ug_foreach_alert_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_ALERT_TABLE,
	                    mf_tbl[field_seq].table_name);

	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return MFD_ERROR_DB_NO_RECORD;
	}

	Eina_List *alert_list = NULL;
	mfRingtone *ritem = NULL;

	while (SQLITE_ROW == rc) {
		ritem = (mfRingtone *)calloc(1, sizeof(mfRingtone));
		if (!ritem) {
			ug_debug("allocation failed");
			return MFD_ERROR_DB_INTERNAL;
		}
		__mf_convert_ringtone_column_to_ringtone_item(stmt, ritem);
		alert_list = eina_list_append(alert_list, ritem);
		rc = sqlite3_step(stmt);
		ug_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	__mf_foreach_ringtone_item_cb(callback, alert_list, user_data);

	if (alert_list) {
		__mf_media_db_eina_list_free_full(&alert_list, __mf_free_ringtone_list);
	}

	return MFD_ERROR_NONE;
}

int mf_ug_get_alert_count(MFDHandle *mfd_handle, int *count)
{
	ug_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_ALERT;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_ALERT_COUNT_TABLE,
	                    mf_tbl[field_seq].table_name);

	if (query_string  == NULL) {
		ug_debug("error to get the query string");
		return MFD_ERROR_DB_INTERNAL;
	}
	ug_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		ug_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		ug_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ug_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		*count = 0;
		return MFD_ERROR_DB_NO_RECORD;
	}

	*count = sqlite3_column_int(stmt, 0);
	ug_debug("count : %d", *count);

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		ug_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	return MFD_ERROR_NONE;
}

