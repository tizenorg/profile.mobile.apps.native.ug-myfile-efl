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




#ifndef __MF_UG_DB_HANDLE_DEF__
#define __MF_UG_DB_HANDLE_DEF__

#include "mf-ug-media-error.h"

void mf_ug_db_handle_destory();
int mf_ug_db_handle_create();
MFDHandle *mf_ug_db_handle_get();
void mf_ug_db_handle_get_shortcut_files(void *data);
void mf_ug_db_handle_get_ringtone_files(void *data);
int mf_ug_db_handle_del_ringtone(const char *ringtone_path);
int mf_ug_db_handle_ringtone_in_db(const char *ringtone_path);
int mf_ug_db_handle_ringtone_get_count();
int mf_ug_db_handle_add_ringtone(const char *ringtone_path, const char *ringtone_name, int storage_type);
bool mf_ug_db_handle_find_ringtone(const char* path);

//1 Alert
void mf_ug_db_handle_get_alert_files(void *data);
int mf_ug_db_handle_del_alert(const char *alert_path);
int mf_ug_db_handle_alert_in_db(const char *alert_path);
int mf_ug_db_handle_alert_get_count();
int mf_ug_db_handle_add_alert(const char *alert_path, const char *alert_name, int storage_type);
bool mf_ug_db_handle_find_alert(const char* path);

#endif
