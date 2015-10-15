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
#include "mf-ug-media.h"
#include "mf-ug-media-db.h"
#include "mf-ug-media-error.h"
#include "mf-ug-dlog.h"
#include "mf-ug-media-types.h"

int mf_ug_media_connect(MFDHandle **handle)
{
	int ret = MFD_ERROR_NONE;
	sqlite3 *db_handle = NULL;

	ret = mf_ug_connect_db_with_handle(&db_handle);
	if (ret != MFD_ERROR_NONE) {
		return ret;
	}

	*handle = db_handle;
	return MFD_ERROR_NONE;

}

int mf_ug_media_disconnect(MFDHandle *handle)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if (handle == NULL) {
		return MFD_ERROR_INVALID_PARAMETER;
	}

	return mf_ug_disconnect_db_with_handle(db_handle);
}

/*1 Shortcut*/

int mf_ug_media_add_shortcut(MFDHandle *mfd_handle, const char *shortcut_path,
		const char *shortcut_name, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_insert_shortcut(mfd_handle, shortcut_path, shortcut_name, storage_type);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("insert device info into devices table failed");
		return ret;
	}

	return ret;
}



int mf_ug_media_delete_shortcut(MFDHandle *mfd_handle, const char *path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_delete_shortcut(mfd_handle, path);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_delete_shortcut_by_type(MFDHandle *mfd_handle, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	ret = mf_ug_delete_shortcut_by_type(mfd_handle, storage_type);

	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_foreach_shortcut_list(MFDHandle *mfd_handle, mf_shortcut_item_cb callback, void *user_data)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_foreach_shortcut_list(mfd_handle, callback, user_data);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_ug_media_get_short_count(MFDHandle *mfd_handle, int *count)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_get_short_count(mfd_handle, count);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_ug_destroy_shortcut_item(MFSitem *sitem)
{
	if (sitem == NULL) {
		ug_debug("ditem is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (sitem->path) {
		free(sitem->path);
		sitem->path = NULL;
	}
	if (sitem->name) {
		free(sitem->name);
		sitem->name = NULL;
	}

	return MFD_ERROR_NONE;
}

/*1 Recent files*/
int mf_ug_media_add_recent_files(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, const char *thumbnail_path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_insert_recent_file(mfd_handle, path, name, storage_type, thumbnail_path);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("insert content info into folder table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_delete_recent_files(MFDHandle *mfd_handle, const char *path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_delete_recent_files(mfd_handle, path);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_delete_recent_files_by_type(mfd_handle, storage_type);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_update_recent_files_thumbnail(mfd_handle, thumbnail, new_thumbnail);
	if (ret != MFD_ERROR_NONE) {
		ug_debug
			("update device icon failed");
		return ret;
	}

	return ret;

}



int mf_ug_media_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_foreach_recent_files_list(mfd_handle, callback, user_data);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug("foreach content list fail");
		return ret;
	}

	return ret;
}


int mf_ug_media_get_recent_files_count(MFDHandle *mfd_handle, int *count)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_get_recent_files_count(mfd_handle, count);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}



int mf_ug_destroy_recent_files_item(MFRitem *ritem)
{
	if (ritem == NULL) {
		ug_debug("citem is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (ritem->path) {
		free(ritem->path);
		ritem->path = NULL;
	}
	if (ritem->name) {
		free(ritem->name);
		ritem->name = NULL;
	}
	if (ritem->thumbnail) {
		free(ritem->thumbnail);
		ritem->thumbnail = NULL;
	}

	return MFD_ERROR_NONE;
}

/*1 Ringtone*/
int mf_ug_media_add_ringtone(MFDHandle *mfd_handle, const char *ringtone_path,
		const char *ringtone_name, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_insert_ringtone(mfd_handle, ringtone_path, ringtone_name, storage_type);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("insert device info into devices table failed");
		return ret;
	}

	return ret;
}



int mf_ug_media_delete_ringtone(MFDHandle *mfd_handle, const char *path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_delete_ringtone(mfd_handle, path);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_delete_ringtone_by_type(MFDHandle *mfd_handle, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	ret = mf_ug_delete_ringtone_by_type(mfd_handle, storage_type);

	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_foreach_ringtone_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_foreach_ringtone_list(mfd_handle, callback, user_data);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_ug_media_get_ringtone_count(MFDHandle *mfd_handle, int *count)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_get_ringtone_count(mfd_handle, count);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_ug_destroy_ringtone_item(mfRingtone *ritem)
{
	if (ritem == NULL) {
		ug_debug("ditem is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (ritem->path) {
		free(ritem->path);
		ritem->path = NULL;
	}
	if (ritem->name) {
		free(ritem->name);
		ritem->name = NULL;
	}

	return MFD_ERROR_NONE;
}

/*1 Alert*/
int mf_ug_media_add_alert(MFDHandle *mfd_handle, const char *alert_path,
		const char *alert_name, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_insert_alert(mfd_handle, alert_path, alert_name, storage_type);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("insert device info into devices table failed");
		return ret;
	}

	return ret;
}



int mf_ug_media_delete_alert(MFDHandle *mfd_handle, const char *path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_delete_alert(mfd_handle, path);
	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_delete_alert_by_type(MFDHandle *mfd_handle, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	ret = mf_ug_delete_alert_by_type(mfd_handle, storage_type);

	if (ret != MFD_ERROR_NONE) {
		ug_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_ug_media_foreach_alert_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_foreach_alert_list(mfd_handle, callback, user_data);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_ug_media_get_alert_count(MFDHandle *mfd_handle, int *count)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		ug_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_ug_get_alert_count(mfd_handle, count);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		ug_debug
			("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_ug_destroy_alert_item(mfRingtone *ritem)
{
	if (ritem == NULL) {
		ug_debug("ditem is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (ritem->path) {
		free(ritem->path);
		ritem->path = NULL;
	}
	if (ritem->name) {
		free(ritem->name);
		ritem->name = NULL;
	}

	return MFD_ERROR_NONE;
}

