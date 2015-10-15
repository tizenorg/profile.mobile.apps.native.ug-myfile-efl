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





#ifndef __DEF_MF_UG_UTIL_H_
#define __DEF_MF_UG_UTIL_H_

#include <Elementary.h>
#include <stdbool.h>
#include <glib.h>

#include "mf-ug-main.h"
#include "mf-ug-fs-util.h"

#define UG_SAFE_FREE_CHAR(x) do {\
					if ((x) != NULL) {\
						free(x); \
						x = NULL;\
					} \
			     } while (0)

#define UG_SAFE_FREE_GSTRING(x) do {\
					if ((x) != NULL) {\
						g_string_free(x, TRUE); \
						x = NULL;\
					} \
				} while (0)

#define UG_CHAR_CHECK_NULL_GOTO(arg, dest) do {\
						if ((arg) == NULL) {\
							goto dest;\
						} \
					   } while (0)

#define UG_GSTRING_CHECK_NULL_GOTO(arg, dest) do {\
						if ((arg) == NULL || (arg->str) == NULL) {\
							goto dest;\
						} \
					   } while (0)


#define UG_SAFE_DEL_NAVI_ITEM(x) do {\
					if ((*x) != NULL) {\
						elm_object_item_del(*x); \
						*x = NULL;\
					} \
				  } while (0)


#define UG_SAFE_FREE_OBJ(x) do {\
				if ((x) != NULL) {\
					evas_object_del(x); \
					x = NULL;\
				} \
			  } while (0)

#define UG_SAFE_STRCPY(dest, src) \
				do { if (!dest || !src) break; \
					strncpy (dest, src, sizeof(dest)-1); \
					dest[sizeof(dest)-1] = 0;	} while (0)

#define UG_SAFE_DEL_ECORE_TIMER(timer) do { \
						if (timer) { \
							ecore_timer_del(timer);\
							timer = NULL; \
						} \
					} while (0)

#define goto_if(x, dest) do { \
				if (x) {\
					goto dest;\
				}\
			} while (0)

#define ug_ecore_idler_del(idler) do { \
					if (idler) { \
						ecore_idler_del(idler);\
						idler = NULL; \
					} \
				} while (0)

typedef enum _mf_ug_list_play_state mf_ug_list_play_state;

enum _mf_ug_list_play_state {
	PLAY_STATE_INIT = 0,
	PLAY_STATE_READY,
	PLAY_STATE_PLAYING,
	PLAY_STATE_PAUSED,
	PLAY_STATE_STOP,
	PLAY_STATE_MAX
};


typedef enum _mf_ug_select_mode mf_ug_select_mode;
enum _mf_ug_select_mode {
	SELECT_MODE = 0,
	SINGLE_FILE_MODE,
	SINGLE_ALL_MODE,
	MULTI_FILE_MODE,
	MULTI_ALL_MODE,
	EXPORT_MODE,
	IMPORT_MODE,
	IMPORT_SINGLE,
	IMPORT_PATH_SELECT_MODE,
	DOCUMENT_SHARE,
	SSM_DOCUMENT_SHARE,
	SAVE_MODE,
	SELECT_MODE_MAX
};

typedef enum _mf_ug_marked_switch mf_ug_marked_switch;
enum _mf_ug_marked_switch {
	MARKED_OFF = 0,
	MARKED_ON,
};

typedef enum _mf_ug_file_filter_type mf_ug_file_filter_type;
enum _mf_ug_file_filter_type {
	SHOW_ALL_LIST = 0,
	SHOW_IMAGE_LIST,
	SHOW_SOUND_LIST,
	SHOW_VIDEO_LIST,
	SHOW_FLASH_LIST,
	SHOW_FOLDER_LIST,
	SHOW_IMAGE_VIDEO_LIST,
	SHOW_IMAGE_SOUND_LIST,
	SHOW_VIDEO_SOUND_LIST,
	SHOW_DOCUMENT_LIST,
	SHOW_BY_EXTENSION
};



typedef enum _mf_ug_state_mode mf_ug_state_mode;
enum _mf_ug_state_mode {
	STATE_PHONE = 0,
	STATE_MEMORY,
	STATE_MODE_MAX
};

typedef enum _mf_ug_mmc_insert_state mf_ug_mmc_insert_state;
enum _mf_ug_mmc_insert_state {
	MMC_OFF = 0,
	MMC_ON
};

typedef enum _mf_ug_launch_type mf_ug_launch_type;
enum _mf_ug_launch_type {
	LAUNCH_TYPE_FORK = 0,
	LAUNCH_TYPE_FAIL,
	LAUNCH_TYPE_DIR,
	LAUNCH_TYPE_IMAGE,
	LAUNCH_TYPE_MUSIC,
	LAUNCH_TYPE_VIDEO,
	LAUNCH_TYPE_UNSUPPORT,
	LAUNCH_TYPE_MAX
};

typedef enum _mf_ug_eina_list_node_type mf_ug_eina_list_node_type;
enum _mf_ug_eina_list_node_type {
	NODE_TYPE_NONE = 0,
	NODE_TYPE_CHAR,
	NODE_TYPE_PNODE,
	NODE_TYPE_GSTRING,
	NODE_TYPE_MAX
};

typedef enum _mf_ug_ctrl_bar_type mf_ug_ctrl_bar_type;
enum _mf_ug_ctrl_bar_type {
	CTRL_BAR_NORMAL = 0,
	CTRL_BAR_MUSIC,
	CTRL_BAR_RINGTONE,
	CTRL_BAR_MULTI,
	CTR_BAR_MAX
};

typedef enum _mf_ug_view_type mf_ug_view_type;
enum _mf_ug_view_type {
	mf_ug_view_root = 0,
	mf_ug_view_normal,
	mf_ug_view_ringtone_del,
	mf_ug_view_max
};

#define INHERIT_MF_LIST \
	int list_type;\
	int ug_iGroupValue;\
	int storage_type;\
	ugData *ug_pData;

typedef struct __mf_list_data_t{
	INHERIT_MF_LIST
}mf_list_data_t;


typedef struct _ugListItemData ugListItemData;
struct _ugListItemData {
	INHERIT_MF_LIST
	Evas_Object *ug_pCheckBox;
	Evas_Object *ug_pRadioBox;
	Evas_Object *ug_pPlaybtn;
	Elm_Object_Item *ug_pItem;
	char *ug_pThumbPath;
	GString *ug_pItemName;
	bool ug_bChecked;
	bool ug_bRealThumbFlag;
	Eina_Bool thumbnail_create;
	media_info_h media;
	char *sound_title;
	bool ug_bDefaultItem;
	double selsize;
};

typedef struct _ug_dir_event_t ug_dir_event_t;
struct _ug_dir_event_t {
	int event;
	char *name;
};

typedef enum _mf_ug_theme_type	mf_ug_theme_type;
enum _mf_ug_theme_type {
	UG_THEME_INVALID = -1,
	UG_THEME_NBEAT = 0,
	UG_THEME_NBEAT_BLACK = 1,
	UG_THEME_ERROR
};

typedef enum _mf_ug_more_type_e	mf_ug_more_type_e;

enum _mf_ug_more_type_e{			/* softkey / contextual popup */
	UG_MORE_DEFAULT = 0,
	UG_MORE_CREATE_FOLDER,
	UG_MORE_SEARCH,
	UG_MORE_TYPE_MAX
};

typedef enum _mf_ug_file_name_type_e mf_ug_file_name_type_e;
enum _mf_ug_file_name_type_e {
	FILE_NAME_WITH_BRACKETS,
	FILE_NAME_WITH_UNDERLINE,
	FILE_NAME_NONE,
};

typedef enum __mf_ug_thumbnail_type_e mf_ug_thumbnail_type_e;
enum __mf_ug_thumbnail_type_e {
	MF_UG_THUMBNAIL_TYPE_DEFAULT,
	MF_UG_THUMBNAIL_TYPE_THUMBNAIL,
	MF_UG_THUMBNAIL_TYPE_MAX
};


typedef struct __mf_ug_view_node_s mf_ug_view_node_s;
struct __mf_ug_view_node_s {
	int view_type;
	char *path;
};


bool mf_ug_util_is_mass_storage_on();
int mf_ug_util_create_dir_monitor(void *data);
int mf_ug_util_set_mmc_state_cb(void *data);
int mf_ug_util_get_mmc_state(int *mmc_card);
char *mf_ug_util_get_send_result(void *data);
bool mf_ug_util_is_genlist_selected(void *data);

void mf_ug_list_play_play_music_item(ugListItemData *data);
void mf_ug_list_play_destory_playing_file(void *data);
void mf_ug_list_play_update_item_icon(void *data);


void mf_ug_util_storage_insert_action(void *data, char *pItemLabel);
void mf_ug_util_destory_mmc_state_cb();
void mf_ug_util_destory_dir_monitor(void *data);
void mf_ug_util_get_params_path(Eina_List **dest_list, const char *path_list);
void mf_ug_util_free_eina_list_data(Eina_List **list, mf_ug_eina_list_node_type node_type);
void mf_ug_util_set_current_state(void *data, int state);
int mf_ug_util_add_dir_watch(const char *path, void *data);
mf_ug_launch_type mf_ug_util_get_file_launch_type(char *path);

void mf_ug_util_destory_mass_storage_callback();
int mf_ug_file_attr_get_parent_path(const char *path, char **parent_path);
char *mf_ug_util_upper_folder_name_get(void *data, GString *fullpath);
void mf_ug_util_operation_alloc_failed(void *data);
int mf_ug_util_remove_dir_watch(void);
void mf_ug_util_sort_the_file_list(void *data);
int mf_ug_util_check_disk_space(void *data);
long mf_ug_util_character_count_get(const char *original);
int mf_ug_util_generate_root_view_file_list(Eina_List **list, int storage_state);
int mf_ug_util_set_default_ringtone_cb(void *data);
void mf_ug_util_destory_default_ringtone_cb();
bool mf_ug_util_is_unique_view(int mode);
bool mf_ug_util_is_multi_select_mode(int mode);
bool mf_ug_util_is_single_select_mode(int mode);
bool mf_ug_util_is_import_mode(int mode);
bool mf_ug_util_find_item_from_pnode_list(Eina_List *list, const char *fullpath);
void mf_ug_util_destory_default_alert_cb();
int mf_ug_util_set_default_alert_cb(void *data);
void mf_ug_util_destory_default_ringtone_cb();
int mf_ug_util_set_default_ringtone_cb(void *data);
char *mf_ug_util_get_default_alert();
char *mf_ug_util_get_default_ringtone();
void mf_ug_util_mmc_remove_action(void *data);
char **mf_ug_util_get_send_result_array(void *data, int *item_count);
void mf_ug_util_path_push(char *path, int view_type);
mf_ug_view_node_s *mf_ug_util_path_pop();
mf_ug_view_node_s *mf_ug_util_path_top_get();
void mf_ug_util_view_node_free(mf_ug_view_node_s **view_node);
void mf_ug_util_path_stack_free();

#endif /* __DEF_MYFILE_UTIL_H_ */
