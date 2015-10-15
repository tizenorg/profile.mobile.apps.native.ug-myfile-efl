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

#ifndef __MF_UG_MEDIA_DB_H__
#define __MF_UG_MEDIA_DB_H__

#include "mf-ug-media-types.h"
#include "mf-ug-media-error.h"
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int mf_ug_connect_db_with_handle(sqlite3 **db_handle);
int mf_ug_disconnect_db_with_handle(sqlite3 *db_handle);

//1 Shortcut
int mf_ug_update_shortcut(MFDHandle *mfd_handle,const char *new_name, char *old_name);
int mf_ug_insert_shortcut(MFDHandle *mfd_handle, const char *shortcut_path, const char *shortcut_name, int storage_type);
int mf_ug_delete_shortcut(MFDHandle *mfd_handle, const char *shortcut_path);
int mf_ug_delete_shortcut_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_foreach_shortcut_list(MFDHandle *mfd_handle, mf_shortcut_item_cb callback, void *user_data);
int mf_ug_get_short_count(MFDHandle *mfd_handle, int *count);

//1 Recent files
int mf_ug_insert_recent_file(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, const char *thumbnail_path);
int mf_ug_delete_recent_files(MFDHandle *mfd_handle, const char *path);
int mf_ug_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail);
int mf_ug_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data);
int mf_ug_get_recent_files_count(MFDHandle *mfd_handle, int *count);

//1 Ringtone
int mf_ug_update_ringtone(MFDHandle *mfd_handle,const char *new_name, char *old_name);
int mf_ug_insert_ringtone(MFDHandle *mfd_handle, const char *ringtone_path, const char *ringtone_name, int storage_type);
int mf_ug_delete_ringtone(MFDHandle *mfd_handle, const char *ringtone_path);
int mf_ug_delete_ringtone_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_foreach_ringtone_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data);
int mf_ug_get_ringtone_count(MFDHandle *mfd_handle, int *count);
int mf_ug_find_ringtone(MFDHandle *mfd_handle, const char *ringtone_path);

//1 Alerts
int mf_ug_update_alert(MFDHandle *mfd_handle,const char *new_name, char *old_name);
int mf_ug_insert_alert(MFDHandle *mfd_handle, const char *alert_path, const char *alert_name, int storage_type);
int mf_ug_delete_alert(MFDHandle *mfd_handle, const char *alert_path);
int mf_ug_delete_alert_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_foreach_alert_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data);
int mf_ug_get_alert_count(MFDHandle *mfd_handle, int *count);
int mf_ug_find_alert(MFDHandle *mfd_handle, const char *alert_path);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*_GALLERY_MEDIA_DB_H_*/


