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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

#include <media_content.h>
#include <Elementary.h>
#include <system_settings.h>
#include "mf-ug-main.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-util.h"
#include "mf-ug-conf.h"
#include "mf-ug-dlog.h"
#include "mf-ug-winset.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-resource.h"
#include "mf-ug-cb.h"
#include "mf-ug-list-play.h"
#include "mf-ug-widget.h"
#include "mf-ug-media.h"
#include "mf-ug-db-handle.h"
#include "mf-ug-music.h"
#include "mf-ug-ringtone-view.h"
#include "mf-ug-file-util.h"


#ifdef UG_OPERATION_SELECT_MODE

#define MF_BUNDLE_SELECTION_MODE        "http://tizen.org/appcontrol/data/selection_mode"
#define MF_BUNDLE_SELECTION_MODE_SINGLE "single"
#define MF_BUNDLE_SELECTION_MODE_MULTI  "multiple"
#endif

#define MF_DEFAULT_RINGTONE_SHOW	"show"

static int exit_flag = EINA_FALSE;
static Ecore_Idler *ctrlbar_state_idler = NULL;
static Elm_Win_Indicator_Opacity_Mode indi_o_mode;
static Elm_Win_Indicator_Mode indi_mode;
static bool overlap_mode = false;
static bool b_hide_indicator = false;
static bool b_is_background = false;

ugData *mf_ug_data = NULL;
ugData *mf_ug_ugdata()
{
	return mf_ug_data;
}

bool mf_ug_main_is_background()
{
	return b_is_background;
}

int __mf_ug_get_indicator_state(ugData *ugd)
{
	ug_mf_retvm_if(ugd == NULL, -1, "ugd is NULL");
	/* Save old view's indicator values */
	indi_mode = elm_win_indicator_mode_get(ugd->ug_MainWindow.ug_pWindow);
	indi_o_mode = elm_win_indicator_opacity_get(ugd->ug_MainWindow.ug_pWindow);
	ug_debug("indi_o_mode: %d, indi_mode: %d", indi_o_mode, indi_mode);
	/* Save old view's overlap mode */
	overlap_mode = (int)evas_object_data_get(ugd->ug_MainWindow.ug_pConformant, "overlap");
	ug_debug("overlap_mode: %d", overlap_mode);
	return 0;
}

int _mf_ug_indicator_state_set(ugData *ugd, bool flag_hide)
{
	ug_mf_retvm_if(ugd == NULL, -1, "ugd is NULL");
	/* transparent indicator setting */
	if (!flag_hide) {
		elm_win_indicator_mode_set(ugd->ug_MainWindow.ug_pWindow, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(ugd->ug_MainWindow.ug_pWindow, ELM_WIN_INDICATOR_OPAQUE);
		elm_object_signal_emit(ugd->ug_MainWindow.ug_pConformant,
		                       "elm,state,indicator,nooverlap", "");
		evas_object_data_set(ugd->ug_MainWindow.ug_pConformant, "overlap", NULL);
	} else {
		elm_win_indicator_mode_set(ugd->ug_MainWindow.ug_pWindow, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(ugd->ug_MainWindow.ug_pWindow, ELM_WIN_INDICATOR_TRANSPARENT);
		elm_object_signal_emit(ugd->ug_MainWindow.ug_pConformant, "elm,state,indicator,overlap", "");
		evas_object_data_set(ugd->ug_MainWindow.ug_pConformant, "overlap", (void *)EINA_TRUE);
	}
	return 0;
}

int _mf_ug_reset_indicator(ugData *ugd)
{
	ug_mf_retvm_if(ugd == NULL, -1, "ugd is NULL");
	ug_debug("indi_o_mode: %d, indi_mode: %d", indi_o_mode,
	         indi_mode);
	ug_debug("overlap_mode: %d", overlap_mode);
	/* Set old view's indicator */
	elm_win_indicator_mode_set(ugd->ug_MainWindow.ug_pWindow, indi_mode);
	elm_win_indicator_opacity_set(ugd->ug_MainWindow.ug_pWindow, indi_o_mode);
	/* set old view's conformant overlap mode
	    if layout is different with new view and needs starts from (0,60) */
	if (!overlap_mode) {
		elm_object_signal_emit(ugd->ug_MainWindow.ug_pConformant,
		                       "elm,state,indicator,nooverlap", "");
		evas_object_data_set(ugd->ug_MainWindow.ug_pConformant, "overlap", NULL);
	} else {
		elm_object_signal_emit(ugd->ug_MainWindow.ug_pConformant, "elm,state,indicator,overlap", "");
		evas_object_data_set(ugd->ug_MainWindow.ug_pConformant, "overlap", (void *)EINA_TRUE);
	}
	ug_debug("indicator restored done!");
	return 0;
}

/******************************
** Prototype    : __mf_ug_main_init_data
** Description  :
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_main_init_data(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	if (ugd->ug_Status.ug_pPath != NULL) {
		g_string_free(ugd->ug_Status.ug_pPath, TRUE);
		ugd->ug_Status.ug_pPath = NULL;
	}
	ugd->ug_Status.ug_pPath = g_string_new(PHONE_FOLDER);
	ugd->ug_Status.ug_iState = STATE_PHONE;
	ugd->ug_Status.ug_iRadioOn = 0;
	ugd->ug_Status.ug_iMmcFlag = false;
	ugd->ug_Status.ug_bInstallFlag = true;
	ugd->ug_Status.ug_iMore = UG_MORE_DEFAULT;
	ugd->ug_Status.ug_iViewType = mf_ug_view_root;

	ugd->ug_UiGadget.ug_iSelectMode = SINGLE_FILE_MODE;
	ugd->ug_UiGadget.ug_iFilterMode = SHOW_ALL_LIST;
	ugd->ug_UiGadget.ug_pExtension = NULL;

	ugd->ug_UiGadget.ug_iMarkedMode = MARKED_OFF;
	ugd->ug_UiGadget.ug_pMultiSelectFileList = NULL;
	ugd->ug_UiGadget.ug_iSoundMode = mf_ug_sound_mode_none;

	ugd->ug_ListPlay.ug_pPlayFilePath = NULL;
	ugd->ug_ListPlay.play_data = NULL;
	ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_INIT;

	mf_ug_util_create_dir_monitor(ugd);
	mf_ug_util_set_mmc_state_cb(ugd);
	/*mf_ug_list_play_earjack_monitor(ugd); UG was not launching due to this function*/
	UG_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_main_free_data
** Description  :
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_main_free_data(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	if (ugd->ug_Status.ug_pPath) {
		g_string_free(ugd->ug_Status.ug_pPath, TRUE);
		ugd->ug_Status.ug_pPath = NULL;
	}
	if (ugd->ug_UiGadget.ug_pMultiSelectFileList) {
		mf_ug_util_free_eina_list_data(&ugd->ug_UiGadget.ug_pMultiSelectFileList, NODE_TYPE_CHAR);
		ugd->ug_UiGadget.ug_pMultiSelectFileList = NULL;
	}
	if (ugd->ug_UiGadget.ug_pDirList) {
		mf_ug_util_free_eina_list_data(&ugd->ug_UiGadget.ug_pDirList, NODE_TYPE_PNODE);
		ugd->ug_UiGadget.ug_pDirList = NULL;
	}
	if (ugd->ug_UiGadget.ug_pFilterList) {
		mf_ug_util_free_eina_list_data(&ugd->ug_UiGadget.ug_pFilterList, NODE_TYPE_PNODE);
		ugd->ug_UiGadget.ug_pFilterList = NULL;
	}

	if (ugd->ug_Status.ug_pUpper_folder) {
		free(ugd->ug_Status.ug_pUpper_folder);
		ugd->ug_Status.ug_pUpper_folder = NULL;
	}
	if (ugd->ug_Status.ug_pEntryPath) {
		free(ugd->ug_Status.ug_pEntryPath);
		ugd->ug_Status.ug_pEntryPath = NULL;
	}
	if (ugd->ug_UiGadget.title) {
		free(ugd->ug_UiGadget.title);
		ugd->ug_UiGadget.title = NULL;
	}
	if (ugd->ug_Status.mark_mode) {
		free(ugd->ug_Status.mark_mode);
		ugd->ug_Status.mark_mode = NULL;
	}

	UG_SAFE_FREE_CHAR(ugd->ug_MainWindow.ug_pNaviTitle);
	UG_SAFE_FREE_CHAR(ugd->ug_Status.monitor_path);
	UG_SAFE_FREE_CHAR(ugd->ug_Status.ug_launch_path);
	UG_SAFE_FREE_CHAR(ugd->ug_UiGadget.ug_pExtension);
	UG_SAFE_FREE_CHAR(ugd->ug_UiGadget.default_ringtone);
	UG_SAFE_FREE_CHAR(ugd->ug_UiGadget.domain);
	UG_SAFE_FREE_CHAR(ugd->ug_UiGadget.position);

	UG_TRACE_END;

}

/******************************
** Prototype    : __mf_ug_main_free_evas_object
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_main_free_evas_object(void *data)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	if (ugd->ug_MainWindow.ug_pNormalPopup != NULL) {
		evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = NULL;
	}
	if (ugd->ug_MainWindow.ug_pRadioGroup != NULL) {
		evas_object_del(ugd->ug_MainWindow.ug_pRadioGroup);
		ugd->ug_MainWindow.ug_pRadioGroup = NULL;
	}

	if (ugd->ug_MainWindow.ug_pBackGround != NULL) {
		evas_object_del(ugd->ug_MainWindow.ug_pBackGround);
		ugd->ug_MainWindow.ug_pBackGround = NULL;
	}

	if (ugd->ug_MainWindow.ug_pMainLayout != NULL) {
		evas_object_del(ugd->ug_MainWindow.ug_pMainLayout);
		ugd->ug_MainWindow.ug_pMainLayout = NULL;
	}
	if (ugd->ug_MainWindow.ug_pContextPopup != NULL) {
		evas_object_del(ugd->ug_MainWindow.ug_pContextPopup);
		ugd->ug_MainWindow.ug_pContextPopup = NULL;
	}

	UG_TRACE_END;
}

Evas_Object *mf_ug_main_create_bg(Evas_Object *win)
{
	ug_mf_retv_if(win == NULL, NULL);
	Evas_Object *bg = elm_bg_add(win);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg);

	return bg;
}

/******************************
** Prototype    : _ug_mf_create_layout_main
** Description  :
** Input        : Evas_Object* parent
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static Evas_Object *__mf_ug_main_create_main_layout(Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	Evas_Object *layout = NULL;
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	layout = elm_layout_add(parent);

	ug_mf_retvm_if(layout == NULL, NULL, "Failed elm_layout_add.\n");

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);
	UG_TRACE_END;

	return layout;
}

Evas_Object *mf_ug_main_tab_layout_create(Evas_Object *parent)
{
	Evas_Object *layout;

	ug_mf_retv_if(parent == NULL, NULL);

	layout = elm_layout_add(parent);
	ug_mf_retvm_if(layout == NULL, NULL, "Failed elm_layout_add.\n");
	elm_object_focus_set(layout, EINA_FALSE);

	/*elm_layout_theme_set(layout, "layout", "application", "tabbar");*/
	elm_layout_theme_set(layout, "layout", "tabbar", "default");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_hide(layout);
	return layout;
}

/******************************
** Prototype    : __mf_ug_main_set_path_option
** Description  : Samsung
** Input        : void *data
**                char *path
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
int __mf_ug_main_get_atoi(const char *number)
{
	char *endptr = NULL;
	long val = 0;

	errno = 0;

	val = strtol(number, &endptr, 10);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
		ug_error("strtol, val = %d", val);
		return -1;
	}

	if (endptr == number) {
		ug_error("No digits were found, number = %s", number);
		return -1;
	}

	return (int)val;
}

static void __mf_ug_main_set_max_len_option(void *data, const char *number)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_error("number is [%s]", number);
	ugd->ug_UiGadget.ug_MaxSetFlag = EINA_FALSE;

	if (number == NULL) {
		ugd->ug_UiGadget.ug_iMaxLength = 500;

	} else {
		int max_len = __mf_ug_main_get_atoi(number);
		if (max_len <= 0 || max_len > 500) {
			ugd->ug_UiGadget.ug_iMaxLength = 500;
		} else {
			ugd->ug_UiGadget.ug_iMaxLength = max_len;
			ugd->ug_UiGadget.ug_MaxSetFlag = EINA_TRUE;
		}
	}
	ug_error("ugd->ug_UiGadget.ug_iMaxLength is [%d]", ugd->ug_UiGadget.ug_iMaxLength);
}


static int __mf_ug_main_set_path_option(void *data, const char *path)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, MYFILE_ERR_SRC_ARG_INVALID, "ugd is NULL");
	ug_mf_retvm_if(path == NULL, MYFILE_ERR_SRC_ARG_INVALID, "path is NULL");

	int error_code = MYFILE_ERR_NONE;
	char *entry_path = NULL;

	if (strncmp(path, PHONE_FOLDER, strlen(PHONE_FOLDER)) == 0 || strncmp(path, MEMORY_FOLDER, strlen(MEMORY_FOLDER)) == 0) {

		/**check whether is /opt/media or /mnt/mmc  */
		entry_path = strdup(path);
		if (entry_path == NULL) {
			return MYFILE_ERR_ALLOCATE_FAIL;
		}
	} else if (strncmp(path, UG_SETTING_RINGTONE_PATH, strlen(UG_SETTING_RINGTONE_PATH)) == 0
	           || strncmp(path, UG_SETTING_ALERTS_PATH, strlen(UG_SETTING_ALERTS_PATH)) == 0
	           || strncmp(path, UG_SETTING_MSG_ALERTS_PATH, strlen(UG_SETTING_MSG_ALERTS_PATH)) == 0
	           || strncmp(path, UG_SETTING_SMART_ALRAMS, strlen(UG_SETTING_SMART_ALRAMS)) == 0) {

		/**check whether is setting ringtone or alerts path  */
		entry_path = strdup(path);
		if (entry_path == NULL) {
			return MYFILE_ERR_ALLOCATE_FAIL;
		}
		if (g_strcmp0(entry_path, UG_SETTING_MSG_ALERTS_PATH) != 0) {
			ugd->ug_UiGadget.ug_iSoundMode = mf_ug_sound_mode_ringtone;
		} else {
			ugd->ug_UiGadget.ug_iSoundMode = mf_ug_sound_mode_alert;
		}
	}  else if (strlen(path) == 1 && strncmp(path, "/", 1)) {
		/**chech the path whether is "/" */
		entry_path = strdup(PHONE_FOLDER);
		if (entry_path == NULL) {
			return MYFILE_ERR_ALLOCATE_FAIL;
		}
	} else {
		/**not begin with /mnt/mmc and /opt/media , so append it to /opt/media*/
		char *temp = strdup(PHONE_FOLDER);
		if (temp == NULL) {
			return MYFILE_ERR_ALLOCATE_FAIL;
		}
		entry_path = g_strconcat(temp, path, NULL);
		free(temp);
	}

	if (entry_path != NULL) {
		/** if there is a '/' at the end of the path, can't be recognized */
		if (entry_path[strlen(entry_path) - 1] == '/') {
			entry_path[strlen(entry_path) - 1] = '\0';
		}

		if (mf_file_exists(entry_path) == false) {

			if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE ||
			        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE ||
			        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE ||
			        ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE ||
			        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {
				error_code = mf_ug_fm_svc_wrapper_create_p(entry_path);
				if (error_code != MYFILE_ERR_NONE) {
					free(entry_path);
					return MYFILE_ERR_INVALID_FILE_PATH;
				}
			} else {
				free(entry_path);
				return MYFILE_ERR_INVALID_FILE_PATH;
			}
		}
		if (mf_is_dir(entry_path) == false) {
			ug_mf_debug("path is not a directory");
			free(entry_path);
			return MYFILE_ERR_INVALID_DIR_PATH;
		}

		if (ugd->ug_Status.ug_pPath) {
			g_string_free(ugd->ug_Status.ug_pPath, TRUE);
			ugd->ug_Status.ug_pPath = NULL;
		}
		ugd->ug_Status.ug_pEntryPath = g_strdup(entry_path);
		ugd->ug_Status.ug_pPath = g_string_new(entry_path);
		if (g_strcmp0(entry_path, PHONE_FOLDER) == 0) {
			ugd->ug_Status.ug_iViewType = mf_ug_view_root;
		} else {
			ugd->ug_Status.ug_iViewType = mf_ug_view_normal;
		}
		free(entry_path);
	} else {
		return MYFILE_ERR_ALLOCATE_FAIL;
	}
	UG_TRACE_END;
	return error_code;
}

/******************************
** Prototype    : __mf_ug_main_set_select_mode
** Description  : Samsung
** Input        : void *data
**                char *select_mode
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_main_set_select_mode(void *data, const char *select_mode)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (select_mode != NULL) {
		if (!strncmp(select_mode, UG_SELECT_MODE_SINGLE_FILE, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = SINGLE_FILE_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_MULTI_FILE, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = MULTI_FILE_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_SINGLE_ALL, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = SINGLE_ALL_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_MULTI_ALL, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = MULTI_ALL_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_IMPORT, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = IMPORT_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_IMPORT_PATH_SELECT, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = IMPORT_PATH_SELECT_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_EXPORT, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = EXPORT_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_IMPORT_SINGLE, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = IMPORT_SINGLE;
			ugd->ug_Status.ug_iViewType = mf_ug_view_normal;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_SAVE, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = SAVE_MODE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_DOCUMENT_SHARE, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = DOCUMENT_SHARE;
		} else if (!strncmp(select_mode, UG_SELECT_MODE_SSM_DOCUMENT_SHARE, strlen(select_mode))) {
			ugd->ug_UiGadget.ug_iSelectMode = SSM_DOCUMENT_SHARE;
		} else {
			ugd->ug_UiGadget.ug_iSelectMode = SINGLE_FILE_MODE;
		}
	} else {
		ugd->ug_UiGadget.ug_iSelectMode = SINGLE_FILE_MODE;
	}
	UG_TRACE_END;

}

static void __mf_ug_main_set_filter_by_mime(void *data, const char *mime_type)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (mime_type != NULL) {
		if (!strncmp(mime_type, UG_FILE_MIME_TYPE_IMAGE, strlen(mime_type))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_IMAGE_LIST;
		} else if (!strncmp(mime_type, UG_FILE_MIME_TYPE_VIDEO, strlen(mime_type))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_VIDEO_LIST;
		} else if (!strncmp(mime_type, UG_FILE_MIME_TYPE_AUDIO, strlen(mime_type))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_SOUND_LIST;
		} else if (!strncmp(mime_type, UG_FILE_MIME_TYPE_DOCUMENT, strlen(mime_type))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_DOCUMENT_LIST;
		} else if (!strncmp(mime_type, UG_FILE_MIME_TYPE_DIR, strlen(mime_type))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_FOLDER_LIST;
		} else if (!strncmp(mime_type, UG_FILE_MIME_TYPE_ALL, strlen(mime_type))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_ALL_LIST;
		}
	}
	if (ugd->ug_UiGadget.ug_iFilterMode != SHOW_BY_EXTENSION) {
		ugd->ug_UiGadget.ug_iFileFilter = mf_ug_fm_svc_wapper_get_file_filter(ugd->ug_UiGadget.ug_iFilterMode);
	}


	if (ugd->ug_UiGadget.ug_iFilterMode == SHOW_SOUND_LIST && ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
		ugd->ug_Status.ug_iCtrlBarType = CTRL_BAR_MUSIC;
	} else {
		ugd->ug_Status.ug_iCtrlBarType = CTRL_BAR_MULTI;
	}
	UG_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_main_set_filter_mode
** Description  : Samsung
** Input        : void *data
**                char *file_filter
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_main_set_filter_mode(void *data, const char *file_filter)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (file_filter != NULL) {
		if (!strncmp(file_filter, UG_FILE_FILTER_ALL, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_ALL_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_IMAGE, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_IMAGE_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_SOUND, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_SOUND_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_VIDEO, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_VIDEO_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_FLASH, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_FLASH_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_FOLDER, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_FOLDER_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_IV, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_IMAGE_VIDEO_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_IS, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_IMAGE_SOUND_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_VS, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_VIDEO_SOUND_LIST;
		} else if (!strncmp(file_filter, UG_FILE_FILTER_DOCUMENT, strlen(file_filter))) {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_DOCUMENT_LIST;
		} else {
			ugd->ug_UiGadget.ug_iFilterMode = SHOW_BY_EXTENSION;
			ugd->ug_UiGadget.ug_pExtension = strdup(file_filter);
			if (g_strcmp0(file_filter, "opml") == 0) {
				ugd->ug_UiGadget.ug_iImportMode = 1;
			}
		}
	} else {
		ugd->ug_UiGadget.ug_iFilterMode = SHOW_ALL_LIST;
		ugd->ug_UiGadget.ug_pExtension = NULL;
	}

	if (ugd->ug_UiGadget.ug_iFilterMode != SHOW_BY_EXTENSION) {
		ugd->ug_UiGadget.ug_iFileFilter = mf_ug_fm_svc_wapper_get_file_filter(ugd->ug_UiGadget.ug_iFilterMode);
	}


	if (ugd->ug_UiGadget.ug_iFilterMode == SHOW_SOUND_LIST && ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
		ugd->ug_Status.ug_iCtrlBarType = CTRL_BAR_MUSIC;
	} else {
		ugd->ug_Status.ug_iCtrlBarType = CTRL_BAR_MULTI;
	}
	UG_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_main_set_marked_mode
** Description  : Samsung
** Input        : void *data
**                char *marked_mode
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_main_set_marked_mode(void *data, const char *marked_mode)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (marked_mode != NULL) {
		ugd->ug_UiGadget.ug_iMarkedMode = MARKED_ON;
		mf_ug_util_get_params_path(&ugd->ug_UiGadget.ug_pMultiSelectFileList, marked_mode);
	} else {
		ugd->ug_UiGadget.ug_iMarkedMode = MARKED_OFF;
	}
	UG_TRACE_END;
}


static int __mf_ug_main_set_view_mode(void *data, const char *view_mode, const char *path)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, MYFILE_ERR_SRC_ARG_INVALID, "ugd is NULL");
	ug_mf_retvm_if(view_mode == NULL, MYFILE_ERR_SRC_ARG_INVALID, "view_mode is NULL");
	ug_mf_retvm_if(path == NULL, MYFILE_ERR_SRC_ARG_INVALID, "path is NULL");

	int error_code = MYFILE_ERR_ALLOCATE_FAIL;
	char *entry_path = NULL;

	if (view_mode != NULL) {
		if (!strncmp(view_mode, UG_VIEW_MODE_DEFAULT_SOUND_ITEM, strlen(view_mode))) {
			entry_path = strdup(path);
			if (entry_path == NULL) {
				error_code = MYFILE_ERR_ALLOCATE_FAIL;
				return error_code;
			}

			if (entry_path[strlen(entry_path) - 1] == '/') {
				entry_path[strlen(entry_path) - 1] = '\0';
			}
			if (ugd->ug_Status.ug_pPath) {
				g_string_free(ugd->ug_Status.ug_pPath, TRUE);
				ugd->ug_Status.ug_pPath = NULL;
			}
			if (mf_file_exists(entry_path) == false || mf_is_dir(entry_path) == false) {
				error_code = MYFILE_ERR_ALLOCATE_FAIL;
			} else {
				ugd->ug_Status.ug_pEntryPath = g_strdup(entry_path);
				ugd->ug_Status.ug_pPath = g_string_new(entry_path);
				if (g_strcmp0(entry_path, PHONE_FOLDER) == 0) {
					ugd->ug_Status.ug_iViewType = mf_ug_view_root;
				} else {
					ugd->ug_Status.ug_iViewType = mf_ug_view_normal;
				}
				error_code = MYFILE_ERR_NONE;
			}
			free(entry_path);
		}
	}
	return error_code;
}

#ifdef UG_OPERATION_SELECT_MODE
static void __mf_ug_main_set_operation_select_mode(void *data, const char *select_mode)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (select_mode != NULL) {
		if (!strcasecmp(select_mode, MF_BUNDLE_SELECTION_MODE_SINGLE)) {
			ugd->ug_UiGadget.ug_iSelectMode = SINGLE_FILE_MODE;
		} else if (!strcasecmp(select_mode, MF_BUNDLE_SELECTION_MODE_MULTI)) {
			ugd->ug_UiGadget.ug_iSelectMode = MULTI_FILE_MODE;
		} else {
			ugd->ug_UiGadget.ug_iSelectMode = SINGLE_FILE_MODE;
		}
		ugd->ug_UiGadget.ug_bOperationSelectFlag = true;
	}
	UG_TRACE_END;

}
#endif

/******************************
** Prototype    : __mf_ug_main_set_option_status
** Description  : Samsung
** Input        : void *data
**                app_control_h  app_control
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
  void __mf_ug_main_set_option_status( app_control_h app_control,void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	char *path = NULL;
	char *select_mode = NULL;
	char *filter_mode = NULL;
	char *marked_mode = NULL;
	char *default_ringtone = NULL;
	char *view_mode = NULL;
	char *mime_type = NULL;
	char *title = NULL;
	char *domain = NULL;
	char *key[UG_OPTION_COUNT] = { "path", "select_type", "file_type",
	                               "marked_mode", "default ringtone",
	                               "view_mode", "title", "domain"
	                             };

	char *operation = NULL;
	app_control_get_operation(app_control, &operation);
	ug_error("operation is [%s]", operation);
	app_control_get_extra_data(app_control, key[0], &path);
	app_control_get_extra_data(app_control, key[1], &select_mode);
	app_control_get_extra_data(app_control, key[2], &filter_mode);
	app_control_get_extra_data(app_control, key[3], &marked_mode);
	app_control_get_extra_data(app_control, key[4], &default_ringtone);
	app_control_get_extra_data(app_control, key[5], &view_mode);
	app_control_get_extra_data(app_control, key[6], &title);
	app_control_get_extra_data(app_control, key[7], &domain);
	app_control_get_mime(app_control, &mime_type);
	SECURE_ERROR("path is [%s] select_mode is [%s] filter_mode is [%s] marked_mode is [%s] default_ringtone is [%s] view_mode is [%s] title is [%s] domain is [%s]",
	             path, select_mode, filter_mode, marked_mode, default_ringtone, view_mode, title, domain);

	if (__mf_ug_main_set_view_mode(ugd, view_mode, path) != MYFILE_ERR_NONE) {
		__mf_ug_main_set_path_option(ugd, path);
	}

	char *indicator = NULL;
	app_control_get_extra_data(app_control, "indicator-state", &indicator);

	/* add indicator state for PLM P131108-02061, leo */
	if (indicator) {
		ug_debug("indicator: %s", indicator);
		if (!strcasecmp(indicator, "hide")) {
			b_hide_indicator = true;
		}
		UG_SAFE_FREE_CHAR(indicator);
	}

	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {

		ugd->ug_Status.mark_mode = g_strdup(marked_mode);
		if (default_ringtone) {
			if (g_strcmp0(default_ringtone, MF_DEFAULT_RINGTONE_SHOW) == 0) {
				if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
					ugd->ug_UiGadget.default_ringtone = mf_ug_util_get_default_ringtone();
					mf_ug_util_set_default_ringtone_cb(ugd);
				} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
					ugd->ug_UiGadget.default_ringtone = mf_ug_util_get_default_alert();
					mf_ug_util_set_default_alert_cb(ugd);
				}
			}
			if (marked_mode && g_strcmp0(DEFAULT_RINGTONE_MARK, marked_mode) == 0) {
				UG_SAFE_FREE_CHAR(marked_mode);
				marked_mode = g_strdup(MF_UG_LABEL_DEFAULT_RINGTONE);
			}
		}
		if (title) {
			ugd->ug_UiGadget.title = g_strdup(title);
			UG_SAFE_FREE_CHAR(title);
		}
		if (domain) {
			ugd->ug_UiGadget.domain = g_strdup(domain);
			UG_SAFE_FREE_CHAR(domain);
		}

		char *silent = NULL;
		app_control_get_extra_data(app_control, "silent", &silent);
		if (silent) {
			ug_debug("silent: %s", silent);
			if (g_strcmp0(SILENT_SHOW, silent) == 0) {
				ugd->ug_UiGadget.silent = EINA_TRUE;
			}
			if (marked_mode && g_strcmp0(SILENT, marked_mode) == 0) {
				UG_SAFE_FREE_CHAR(marked_mode);
				marked_mode = g_strdup(MF_UG_LABEL_SILENT);
			}
		}
	} else {
		__mf_ug_main_set_select_mode(ugd, select_mode);

		ug_error("mime_type is [%s]", mime_type);
		if (mime_type) {
			__mf_ug_main_set_filter_by_mime(ugd, mime_type);
		} else {
			__mf_ug_main_set_filter_mode(ugd, filter_mode);
		}

#ifdef UG_OPERATION_SELECT_MODE
		char *operation_select_mode = NULL;
		app_control_get_extra_data(app_control, APP_CONTROL_DATA_SELECTION_MODE, &operation_select_mode);
		ug_error("operation_select_mode is [%s]", operation_select_mode);
		__mf_ug_main_set_operation_select_mode(ugd, operation_select_mode);
		UG_SAFE_FREE_CHAR(operation_select_mode);
#endif

	}
	__mf_ug_main_set_marked_mode(ugd, marked_mode);
	ugd->limitsize = -1;

	char *max_size = NULL;
	app_control_get_extra_data(app_control, APP_CONTROL_DATA_TOTAL_SIZE, &(max_size));
	if (max_size) {
		ugd->limitsize = atoi(max_size);
		UG_SAFE_FREE_CHAR(max_size);
	}
	char *number = NULL;
	app_control_get_extra_data(app_control, APP_CONTROL_DATA_TOTAL_COUNT, &number);
	__mf_ug_main_set_max_len_option(ugd, number);

	UG_SAFE_FREE_CHAR(number);
	UG_SAFE_FREE_CHAR(path);
	UG_SAFE_FREE_CHAR(select_mode);
	UG_SAFE_FREE_CHAR(filter_mode);
	UG_SAFE_FREE_CHAR(marked_mode);
	UG_SAFE_FREE_CHAR(default_ringtone);
	UG_SAFE_FREE_CHAR(mime_type);
	UG_SAFE_FREE_CHAR(view_mode);

	ugd->ug_Status.ug_launch_path = g_strdup(ugd->ug_Status.ug_pPath->str);
	UG_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_main_create_default_layout
** Description  : Samsung
** Input        : Evas_Object* parent
**                void* data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static Evas_Object *__mf_ug_main_create_default_layout(Evas_Object *parent, void *data)
{
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	int tab_item_count = 1;

	int mmc_card = 0;
	int error_code = 0;
	/*/check if mmc mounted */
	error_code = mf_ug_util_get_mmc_state(&mmc_card);
	if (error_code == 0 && mmc_card == 1) {
		tab_item_count++;
	}
	ugd->ug_MainWindow.ug_pNaviBar = mf_ug_navi_bar_create_navi_bar(parent);

	UG_TRACE_END;
	return ugd->ug_MainWindow.ug_pNaviBar;
}

/******************************
** Prototype    : __mf_ug_main_start
** Description  :
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static Eina_Bool __mf_ug_ctrlbar_state_idler(void *data)
{
	UG_TRACE_BEGIN;
	ctrlbar_state_idler = NULL;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, EINA_FALSE, "ugd is NULL");
	mf_ug_navi_bar_set_ctrl_item_disable(ugd);

	return EINA_FALSE;

}

void mf_ug_main_update_ctrl_in_idle(void *data)
{
	ug_ecore_idler_del(ctrlbar_state_idler);
	ctrlbar_state_idler = ecore_idler_add(__mf_ug_ctrlbar_state_idler, data);
}

static void __mf_ug_main_start(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	Evas_Object *pContent = NULL;

	pContent = __mf_ug_main_create_default_layout(ugd->ug_MainWindow.ug_pMainLayout, ugd);

	if (pContent != NULL) {
		elm_object_part_content_set(ugd->ug_MainWindow.ug_pMainLayout, "elm.swallow.content", pContent);
		{
			if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
				mf_ug_create_rintone_view(ugd);
				/*Fix the P130902-01617, refer to the android galaxy S4.*/
				mf_ug_player_vol_set(ugd, ugd->ug_Status.ug_pEntryPath);

			} else {
				mf_ug_navi_bar_create_default_view(ugd);

			}
			mf_ug_util_path_push(ugd->ug_Status.ug_pPath->str, ugd->ug_Status.ug_iViewType);
		}
		mf_ug_main_update_ctrl_in_idle(ugd);
	} else {
		return;
	}
	UG_TRACE_END;

	return;
}

/******************************
** Prototype    : on_create
** Description  : Samsung
** Input        : ui_gadget_h  ug
**                enum ug_mode mode
**                app_control_h app_control
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
bool on_create( void *priv)
{
	UG_TRACE_BEGIN;

	Evas_Object *win = NULL;
	ugData *ugd = (ugData*)priv;

	ug_mf_retv_if(NULL == priv, false);

	ugd = priv;
	bindtextdomain(UGPACKAGE, UGLOCALEDIR);
	elm_theme_extension_add(NULL, UG_EDJ_NAVIGATIONBAR);

	win = elm_win_util_standard_add(UGPACKAGE, UGPACKAGE);

	ug_mf_retv_if(NULL == win, false);

	elm_win_conformant_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);

	Evas_Object *parent = elm_conformant_add(win);
	if (!parent)
		return false;
	evas_object_size_hint_weight_set(parent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//	elm_win_resize_object_add(win, parent);
	evas_object_show(parent);
	evas_object_show(win);
	ugd->ug_MainWindow.ug_pConformant = parent;
	ug_mf_retv_if(NULL == ugd->ug_MainWindow.ug_pConformant, false);

	ugd->ug_MainWindow.ug_pWindow = win;
	ugd->ug_Status.ug_bInstallFlag = false;
	ugd->ug_Status.ug_bCancelDisableFlag = false;
	ugd->ug_UiGadget.ug_iImportMode = 0;

	/*evas_object_event_callback_add(ugd->ug_MainWindow.ug_pWindow, EVAS_CALLBACK_RESIZE, mf_ug_resize_more_ctxpopup_cb, ugd);*/

	ugd->ug_MainWindow.ug_pMainLayout = __mf_ug_main_create_main_layout(ugd->ug_MainWindow.ug_pWindow);
	ugd->ug_MainWindow.ug_pBackGround = mf_ug_main_create_bg(ugd->ug_MainWindow.ug_pWindow);
	elm_object_part_content_set(ugd->ug_MainWindow.ug_pMainLayout, "elm.swallow.bg", ugd->ug_MainWindow.ug_pBackGround);

	__mf_ug_get_indicator_state(ugd);

	__mf_ug_main_init_data(ugd);
	ug_error("b_hide_indicator is [%d]", b_hide_indicator);
	_mf_ug_indicator_state_set(ugd, b_hide_indicator);
	int err = media_content_connect();
	if (err != MEDIA_CONTENT_ERROR_NONE) {
		ug_debug("media_content_connect failed!");
	}

	/*device_add_callback(DEVICE_CALLBACK_POWER_STATE, mf_ug_cb_lcd_state_changed_cb, ugd);*/

	int ret = mf_ug_db_handle_create();
	if (ret == MFD_ERROR_NONE) {
		ug_error("db open success");
	} else {
		ug_error("db open failed");
	}

	__mf_ug_main_start(ugd);
	ugd->ug_Status.ug_launch_view = ugd->ug_Status.ug_iViewType;

	elm_win_resize_object_add(win, ugd->ug_MainWindow.ug_pMainLayout);

	UG_TRACE_END;
	return true;
}

/******************************
** Prototype    : on_start
** Description  :
** Input        : ui_gadget_h ug
**                app_control_h app_control
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/

#if 0
static void on_start(ui_gadget_h ug, app_control_h app_control, void *priv)
{

}
#endif

/******************************
** Prototype    : on_pause
** Description  :
** Input        : ui_gadget_h ug
**                app_control_h app_control
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void on_pause( void *priv)
{
	UG_TRACE_BEGIN;
	if (!priv) {
		return;
	}
	ugData *ugd = (ugData *)priv;


	if (0 != ugd->ug_ListPlay.ug_Player) {
		if (!mf_ug_list_play_pause(ugd)) {
			ug_error("===========================");
			mf_ug_list_play_destory_playing_file(ugd);
		}

	}
	/*Fix the P130902-01617, refer to the android galaxy S4.*/
	mf_ug_player_vol_reset_default_value(ugd);
}

/******************************
** Prototype    : on_resume
** Description  :
** Input        : ui_gadget_h ug
**                bundle *data
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
bool mf_ug_main_check_exist(const char *path)
{
	if (path && (access(path, F_OK) == 0)) {
		return true;
	}
	return false;
}

 void on_resume( void *priv)
{
	UG_TRACE_BEGIN;

	b_is_background = false;
	ug_mf_retm_if(NULL == priv, "priv is NULL");
	ugData *ugd = priv;

	if (mf_ug_util_is_unique_view(ugd->ug_UiGadget.ug_iSelectMode)) {
		return;
	}
	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
			int count = mf_ug_navibar_get_ringtone_count(ugd->ug_UiGadget.ug_iSoundMode);
			int item_count = elm_genlist_items_count(ugd->ug_MainWindow.ug_pNaviGenlist);
			if (count != item_count) {
				/*To fix P140507-02173, there is no need to update delete view when reenter settings
				UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNormalPopup);
				mf_ug_navi_bar_create_delete_view(ugd);*/
			}
		}
		/*Fix the P130902-01617, refer to the android galaxy S4.*/
		if (!mf_ug_is_music_ug_run()) {
			ug_error("========================== music ug is not running");
			mf_ug_player_vol_set(ugd, ugd->ug_Status.ug_pEntryPath);
		}
		mf_ug_ringtone_list_resume(ugd);
		return;
	}
	if (!mf_ug_main_check_exist(ugd->ug_Status.ug_pPath->str)) {
		GString *parent_path = mf_ug_fm_svc_wrapper_get_file_parent_path(ugd->ug_Status.ug_pPath);
		int storage = mf_ug_fm_svc_wapper_get_location(ugd->ug_Status.ug_pPath->str);

		g_string_free(ugd->ug_Status.ug_pPath, TRUE);
		ugd->ug_Status.ug_pPath = NULL;

		if (parent_path && parent_path->str) {
			if (mf_ug_main_check_exist(parent_path->str)) {
				ugd->ug_Status.ug_pPath = g_string_new(parent_path->str);
			} else {
				if (storage == MF_UG_PHONE) {
					ugd->ug_Status.ug_pPath = g_string_new(PHONE_FOLDER);
				} else {
					ugd->ug_Status.ug_pPath = g_string_new(MEMORY_FOLDER);
				}
			}
		} else {
			if (storage == MF_UG_PHONE) {
				ugd->ug_Status.ug_pPath = g_string_new(PHONE_FOLDER);
			} else {
				ugd->ug_Status.ug_pPath = g_string_new(MEMORY_FOLDER);
			}
		}

		mf_ug_navi_bar_create_default_view(ugd);
		mf_ug_navi_bar_set_ctrl_item_disable(ugd);
		g_string_free(parent_path, TRUE);
		parent_path = NULL;
	}


	UG_TRACE_END;
}

/******************************
** Prototype    : on_message
** Description  :
** Input        : ui_gadget_h ug
**                bundle *msg
**                bundle *data
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
#if 0
static void on_message(ui_gadget_h ug, app_control_h msg, app_control_h app_control, void *priv)
{
}
#endif
void __mf_ug_subtitle_show(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ugListItemData *itemData = NULL ;

	int iDirCount = 0, iFileCount = 0;
	Elm_Object_Item *it = NULL;
	Evas_Object *pGenlist = ugd->ug_MainWindow.ug_pNaviGenlist;

	it = elm_genlist_first_item_get(pGenlist);
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->ug_bChecked) {
			if (mf_is_dir(itemData->ug_pItemName->str)) {
				iDirCount++;
			} else {
				iFileCount++;
			}
		}

		it = elm_genlist_item_next_get(it);
	}
	int count = 0;
	count = iDirCount + iFileCount;
	if (count > 0) {
		char *label = NULL;
		label = g_strdup_printf(mf_ug_widget_get_text(MF_UG_LABEL_SELECTED), count);
		if (ugd->ug_MainWindow.ug_pNaviItem != NULL) {
			ug_error("label = %s", label);
		}
		elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "subtitle", label);
		ug_error("label = %s", label);
		UG_SAFE_FREE_CHAR(label);
	} else {
		elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "subtitle", "");
		elm_object_item_signal_emit(ugd->ug_MainWindow.ug_pNaviItem, "elm,state,subtitle,hide", "elm");
	}
	UG_TRACE_END;

}

static void __ug_language_changed_cb(void *user_data)
{
	ug_error("__ug_language_changed_cb");
	ugData *ugd = (ugData *)user_data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	char *locale = NULL;
	int retcode = -1;
	retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ug_mf_error("[ERR] failed to update the language");
	}
	if (locale) {
		ug_error("locale is [%s]", locale);
		elm_language_set(locale);
		if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE ||
		        ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE ||
		        ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE ||
		        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE) {
			__mf_ug_subtitle_show(ugd);
		}
	}
}

/******************************
** Prototype    : on_event
** Description  :
** Input        : ui_gadget_h ug
**                enum ug_event event
**                app_control_h app_control
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
#if 0
static void on_event(ui_gadget_h ug, enum ug_event event, app_control_h app_control, void *priv)
{

	ugData *ugd = NULL;

	ug_mf_retm_if(NULL == priv, "priv is NULL");

	ugd = priv;

	UG_TRACE_BEGIN;
	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		break;

	case UG_EVENT_LOW_BATTERY:
		break;

	case UG_EVENT_LANG_CHANGE:
		__ug_language_changed_cb(ugd);
		break;

	case UG_EVENT_ROTATE_PORTRAIT:
		break;

	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		break;

	case UG_EVENT_ROTATE_LANDSCAPE:
		break;

	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		break;

	default:
		break;
	}
	UG_TRACE_END;
}
#endif


/******************************
** Prototype    : on_key_event
** Description  : Samsung
** Input        : ui_gadget_h ug
**                enum ug_key_event event
**                app_contrul_h app_contrul
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
#if 0
static void on_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h app_control, void *priv)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)priv;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	switch (event) {
	case UG_KEY_EVENT_END:
		if (ugd->ug_MainWindow.ug_pNormalPopup) {
			evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
			ugd->ug_MainWindow.ug_pNormalPopup = NULL;
		}
		if (0 != ugd->ug_ListPlay.ug_Player) {
			mf_ug_list_play_destory_playing_file(ugd);
			ugd->ug_ListPlay.play_data = NULL;
			UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
		}
		break;
	default:
		break;
	}
	UG_TRACE_END;
}
#endif
/******************************
** Prototype    : on_destroy
** Description  :
** Input        : ui_gadget_h ug
**                app_control_h app_control
**                void *priv
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
 void on_destroy( void *priv)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)priv;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (exit_flag) {
		return;
	}
	/*close_rec_ext_handle();*/
	exit_flag = EINA_TRUE;
	mf_ug_util_path_stack_free();
	mf_ug_destory_music_ug();
	__mf_ug_main_free_data(ugd);
	if (ugd->show) {/*no destroy the idle after UG destroy, Fixed the P131014-03517  by jian12.li*/
		ecore_idler_del(ugd->show);
		ugd->show = NULL;
	}
	if (ugd->ug_ListPlay.playing_err_idler) {/*no destroy the idle after UG destroy, Fixed the P131014-03517  by jian12.li*/
		ecore_idler_del(ugd->ug_ListPlay.playing_err_idler);
		ugd->ug_ListPlay.playing_err_idler = NULL;
	}
	if (ugd->ug_Status.popup_del_idler) {/*no destroy the idle after UG destroy, Fixed the P131014-03517  by jian12.li*/
		ecore_idler_del(ugd->ug_Status.popup_del_idler);
		ugd->ug_Status.popup_del_idler = NULL;
	}

	if (ugd->ug_Status.search_idler) {/*no destroy the idle after UG destroy, Fixed the P131014-03517  by jian12.li*/
		ecore_idler_del(ugd->ug_Status.search_idler);
		ugd->ug_Status.search_idler = NULL;
	}
	if (ugd->ug_Status.ug_bInstallFlag == true) {
		mf_ug_util_destory_dir_monitor(ugd);
		mf_ug_util_destory_mmc_state_cb();
		mf_ug_destory_earjack_monitor();
		if (media_content_disconnect() != MEDIA_CONTENT_ERROR_NONE) {
			ug_error("media content disconnect failed.");
		}
		/*power_unset_changed_cb();*/
	}

	ug_ecore_idler_del(ctrlbar_state_idler);

	if (ugd->ug_Status.search_handler > 0) {
		mf_ug_search_stop(ugd->ug_Status.search_handler);
	}


	if (ugd->ug_Status.search_handler > 0) {
		mf_ug_search_finalize(&ugd->ug_Status.search_handler);
	}
	if (ugd->ug_UiGadget.ug_pSyncPipe != NULL) {
		ecore_pipe_del(ugd->ug_UiGadget.ug_pSyncPipe);
		ugd->ug_UiGadget.ug_pSyncPipe = NULL;
	}

	/*evas_object_event_callback_del(ugd->ug_MainWindow.ug_pWindow, EVAS_CALLBACK_RESIZE, mf_ug_resize_more_ctxpopup_cb);*/

	mf_ug_db_handle_destory();
	if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
		mf_ug_util_destory_default_ringtone_cb();
	} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
		mf_ug_util_destory_default_alert_cb();
	}
	ug_ecore_idler_del(ugd->ug_Status.search_idler);
	ug_ecore_idler_del(ugd->ug_Status.popup_del_idler);
	ug_ecore_idler_del(ugd->ug_Status.popup_create_idler);
	ug_ecore_idler_del(ugd->ug_Status.msg_finish_idler);
	UG_SAFE_DEL_ECORE_TIMER(ugd->ug_Status.pSearchTimer);

	if (0 != ugd->ug_ListPlay.ug_Player) {
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}

	UG_SAFE_DEL_ECORE_TIMER(ugd->ug_Status.play_timer);
	_mf_ug_reset_indicator(ugd);
	__mf_ug_main_free_evas_object(ugd);

	/*Fix the P130902-01617, refer to the android galaxy S4.*/
	mf_ug_player_vol_reset_default_value(ugd);

	UG_TRACE_END;
}

/******************************
** Prototype    : UG_MODULE_INIT
** Description  :
** Input        : struct ug_module_ops *ops
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
int main(int argc, char *argv[])
{
	UG_TRACE_BEGIN;
	ui_app_lifecycle_callback_s ops;
	int ret = APP_ERROR_NONE;
	struct _ugData ugd;
	app_event_handler_h hLanguageChangedHandle;
	app_event_handler_h hRegionFormatChangedHandle;
	memset(&ops, 0x0, sizeof(ui_app_lifecycle_callback_s));
	memset(&ugd, 0x0, sizeof(struct _ugData));
	mf_ug_data = &ugd;
	ops.create = on_create;
	ops.terminate = on_destroy;
	ops.pause = on_pause;
	ops.resume = on_resume;
	ops.app_control =__mf_ug_main_set_option_status;
	ret = ui_app_add_event_handler(&hRegionFormatChangedHandle, APP_EVENT_REGION_FORMAT_CHANGED, __ug_language_changed_cb, (void*)&ugd);
	if (ret != APP_ERROR_NONE) {
		ug_error("APP_EVENT_REGION_FORMAT_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}

	ret = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, __ug_language_changed_cb, (void*)&ugd);
	if (ret != APP_ERROR_NONE) {
		ug_error("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}
	UG_TRACE_END
	return ui_app_main(argc, argv, &ops, &ugd);
}
