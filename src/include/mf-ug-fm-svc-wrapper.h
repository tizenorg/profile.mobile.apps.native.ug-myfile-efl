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




#ifndef __DEF_MF_UG_FM_SVC_WRAPPER_H_
#define __DEF_MF_UG_FM_SVC_WRAPPER_H_

#include <glib.h>
#include "mf-ug-main.h"

#define MF_UG_PATH_INFO_MAX_LENGTH_PORTRAIT	35


/* file information get/set */
unsigned long mf_ug_fm_svc_wapper_get_file_filter(int file_filter_mode);

int mf_ug_fm_svc_wapper_get_file_list_by_filter(ugData *data, GString *fullpath, Eina_List **dir_list, Eina_List **filter_list);
bool mf_ug_fm_svc_wapper_is_root_path(void *data);
GString *mf_ug_fm_svc_wapper_get_file_name(GString *path);
char *mf_ug_fm_svc_wapper_get_root_path_by_tab_label(const char *label);
int mf_ug_fm_svc_wapper_get_location(char *fullpath);
gint mf_ug_fm_svc_wapper_get_folder_foldersystem(GString *path, bool *result);
GString *mf_ug_fm_svc_wrapper_get_file_parent_path(GString *fullpath);
char *mf_ug_fm_svc_wapper_path_info_get(char *original_path);
char *mf_ug_fm_svc_path_info_translate(char *path_info, int path_info_max_len);
//unsigned long mf_ug_fm_svc_wrapper_get_free_space(int state);
int mf_ug_fm_svc_wrapper_get_free_space();
int mf_ug_fm_svc_wrapper_file_auto_rename(void *data, GString *fullpath, int file_name_type, GString **filename);
int mf_ug_fm_svc_wrapper_create_service(void *data, GString *fullpath);
bool mf_ug_fm_svc_wrapper_detect_duplication(GString *to);
int mf_ug_fm_svc_wrapper_create_p(const char *fullpath);
char *mf_ug_fm_svc_wrapper_translate_path(char *original_path);
bool mf_ug_fm_svc_wapper_is_default_ringtone(void *data, char* selected_file);
Eina_List *mf_ug_fm_svc_wrapper_level_path_get(const char *original_path);

#endif
