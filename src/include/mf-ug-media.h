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

#ifndef __MF_UG_MEDIA_H_
#define __MF_UG_MEDIA_H_

#include "mf-ug-media-types.h"
#include "mf-ug-media-error.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int mf_ug_media_connect(MFDHandle **handle);
int mf_ug_media_disconnect(MFDHandle *handle);
int mf_ug_media_add_shortcut(MFDHandle *mfd_handle, const char *shortcut_path, const char *shortcut_name, int storage_type);
int mf_ug_media_delete_shortcut(MFDHandle *mfd_handle, const char *path);
int mf_ug_media_delete_shortcut_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_media_add_recent_files(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, const char *thumbnail_path);
int mf_ug_media_delete_recent_files(MFDHandle *mfd_handle, const char *path);
int mf_ug_media_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_media_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail);
int mf_ug_media_foreach_shortcut_list(MFDHandle *mfd_handle, mf_shortcut_item_cb callback, void *user_data);
int mf_ug_media_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data);
int mf_ug_media_get_short_count(MFDHandle *mfd_handle, int *count);
int mf_ug_media_get_recent_files_count(MFDHandle *mfd_handle, int *count);
int mf_ug_destroy_shortcut_item(MFSitem *sitem);
int mf_ug_destroy_recent_files_item(MFRitem *ritem);

//1 Ringtone
int mf_ug_media_add_ringtone(MFDHandle *mfd_handle, const char *ringtone_path,
		const char *ringtone_name, int storage_type);
int mf_ug_media_delete_ringtone(MFDHandle *mfd_handle, const char *path);
int mf_ug_media_delete_ringtone_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_media_foreach_ringtone_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data);
int mf_ug_media_get_ringtone_count(MFDHandle *mfd_handle, int *count);
int mf_ug_destroy_ringtone_item(mfRingtone *ritem);

//1 Alert
int mf_ug_media_add_alert(MFDHandle *mfd_handle, const char *alert_path,
		const char *alert_name, int storage_type);
int mf_ug_media_delete_alert(MFDHandle *mfd_handle, const char *path);
int mf_ug_media_delete_alert_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_ug_media_foreach_alert_list(MFDHandle *mfd_handle, mf_ringtone_item_cb callback, void *user_data);
int mf_ug_media_get_alert_count(MFDHandle *mfd_handle, int *count);
int mf_ug_destroy_alert_item(mfRingtone *ritem);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*__MF_UG_MEDIA_H_*/


