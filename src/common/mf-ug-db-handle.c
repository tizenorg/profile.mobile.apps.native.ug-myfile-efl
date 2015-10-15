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


#include <Eina.h>
#include <Elementary.h>
#include <Ecore.h>

#include "mf-ug-media.h"
#include "mf-ug-media-db.h"
#include "mf-ug-file-util.h"

static MFDHandle *mfd_handle = NULL;

void mf_ug_db_handle_destory()
{
	mf_ug_media_disconnect(mfd_handle);
}

int mf_ug_db_handle_create()
{
	if (mfd_handle) {
		mf_ug_db_handle_destory();
	}

	int ret = mf_ug_media_connect(&mfd_handle);
	return ret;
}

MFDHandle *mf_ug_db_handle_get()
{
	return mfd_handle;
}

/*1 Shortcut*/
bool mf_ug_db_handle_get_shortcut_cb(MFSitem *Sitem, void *user_data)
{
	Eina_List **list = (Eina_List **)user_data;
	if (Sitem && Sitem->path) {
		if (mf_file_exists(Sitem->path)) {
			*list = eina_list_append(*list, g_strdup(Sitem->path));
		} else {
			mf_ug_media_delete_shortcut(mfd_handle, Sitem->path);
		}
	}
	return true;
}

void mf_ug_db_handle_get_shortcut_files(void *data)
{
	mf_ug_media_foreach_shortcut_list(mfd_handle, mf_ug_db_handle_get_shortcut_cb, data);
}


/*1 Ringtone*/

bool mf_ug_db_handle_get_ringtone_cb(mfRingtone *ritem, void *user_data)
{
	Eina_List **list = (Eina_List **)user_data;
	if (ritem && ritem->path) {
		if (mf_file_exists(ritem->path)) {
			*list = eina_list_append(*list, g_strdup(ritem->path));
		} else {
			mf_ug_media_delete_ringtone(mfd_handle, ritem->path);
		}
	}
	return true;
}

void mf_ug_db_handle_get_ringtone_files(void *data)
{
	mf_ug_media_foreach_ringtone_list(mfd_handle, mf_ug_db_handle_get_ringtone_cb, data);
}


bool mf_ug_db_handle_find_ringtone(const char *path)
{
	int find = mf_ug_find_ringtone(mfd_handle, path);
	return (find == 1 ? true : false);
}

int mf_ug_db_handle_add_ringtone(const char *ringtone_path, const char *ringtone_name, int storage_type)
{
	bool find = mf_ug_db_handle_find_ringtone(ringtone_path);
	int ret = MFD_ERROR_DB_NO_RECORD;
	if (find == false) {
		ret = mf_ug_media_add_ringtone(mfd_handle, ringtone_path, NULL, storage_type);
	} else if (find == true) {
		ret = MFD_ERROR_FILE_EXSITED;
	}
	return ret;
}

int mf_ug_db_handle_del_ringtone(const char *ringtone_path)
{
	int ret = mf_ug_media_delete_ringtone(mfd_handle, ringtone_path);
	return ret;
}

int mf_ug_db_handle_ringtone_in_db(const char *ringtone_path)
{
	int ret = mf_ug_find_ringtone(mfd_handle, ringtone_path);
	return ret;
}

int mf_ug_db_handle_ringtone_get_count()
{
	int count = 0;
	mf_ug_media_get_ringtone_count(mfd_handle, &count);
	return count;
}
/*1 Alert*/

bool mf_ug_db_handle_get_alert_cb(mfRingtone *ritem, void *user_data)
{
	Eina_List **list = (Eina_List **)user_data;
	if (ritem && ritem->path) {
		if (mf_file_exists(ritem->path)) {
			*list = eina_list_append(*list, g_strdup(ritem->path));
		} else {
			mf_ug_media_delete_alert(mfd_handle, ritem->path);
		}
	}
	return true;
}

void mf_ug_db_handle_get_alert_files(void *data)
{
	mf_ug_media_foreach_alert_list(mfd_handle, mf_ug_db_handle_get_alert_cb, data);
}


bool mf_ug_db_handle_find_alert(const char *path)
{
	int find = mf_ug_find_alert(mfd_handle, path);
	return (find == 1 ? true : false);
}

int mf_ug_db_handle_add_alert(const char *alert_path, const char *alert_name, int storage_type)
{
	bool find = mf_ug_db_handle_find_alert(alert_path);
	int ret = MFD_ERROR_DB_NO_RECORD;
	if (find == false) {
		ret = mf_ug_media_add_alert(mfd_handle, alert_path, NULL, storage_type);
	} else if (find == true) {
		ret = MFD_ERROR_FILE_EXSITED;
	}
	return ret;
}

int mf_ug_db_handle_del_alert(const char *alert_path)
{
	int ret = mf_ug_media_delete_alert(mfd_handle, alert_path);
	return ret;
}

int mf_ug_db_handle_alert_in_db(const char *alert_path)
{
	int ret = mf_ug_find_alert(mfd_handle, alert_path);
	return ret;
}

int mf_ug_db_handle_alert_get_count()
{
	int count = 0;
	mf_ug_media_get_alert_count(mfd_handle, &count);
	return count;
}

