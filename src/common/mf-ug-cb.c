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
#include <pthread.h>
#include <Elementary.h>
#include <device/power.h>

#include "mf-ug-main.h"
#include "mf-ug-util.h"
#include "mf-ug-inotify-handle.h"
#include "mf-ug-winset.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-resource.h"
#include "mf-ug-list-play.h"
#include "mf-ug-widget.h"
#include "mf-ug-cb.h"
#include "mf-ug-db-handle.h"
#include "mf-ug-music.h"
#include "mf-ug-ringtone-view.h"
#include "mf-ug-file-util.h"

#define UG_MAX_LEN_VIB_DURATION 0.5

#ifdef UG_OPERATION_SELECT_MODE
#define RESULT_KEY  "http://tizen.org/appcontrol/data/selected"
#endif

bool g_is_press_cancel_button = false;

/******************************
** Prototype    : mf_ug_cb_back_button_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
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

Eina_Bool mf_ug_ringtone_present_del_result(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, EINA_FALSE, "ugData is NULL");

	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none && ugd->ug_UiGadget.default_ringtone) {
		if (mf_ug_popup_present_flag_get()) {
			char *result = NULL;
			app_control_h app_control = NULL;
			result = g_strdup(DEFAULT_RINGTONE_MARK);
			if (result) {
				SECURE_ERROR("result is [%s]", result);
				int ret = 0;
				ret = app_control_create(&app_control);
				if (ret == APP_CONTROL_ERROR_NONE) {
					app_control_add_extra_data(app_control, "result", result);
					app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, result);
					ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
					app_control_destroy(app_control);
				}
				SECURE_DEBUG("result is [%s]", result);
				UG_SAFE_FREE_CHAR(result);
				return EINA_TRUE;
			}

		}
	}
	return EINA_FALSE;
}

bool mf_ug_cb_back_operation(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;

	mf_ug_view_node_s *view_node = NULL;
	view_node = mf_ug_util_path_pop();
	if (view_node && view_node->path) {
		if (g_strcmp0(ugd->ug_Status.ug_launch_path, view_node->path) == 0 && ugd->ug_Status.ug_launch_view == ugd->ug_Status.ug_iViewType) {
			mf_ug_util_view_node_free(&view_node);
			return true;
		}
	}
	view_node = NULL;
	view_node = mf_ug_util_path_top_get();
	if (view_node) {
		UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
		ugd->ug_Status.ug_pPath = g_string_new(view_node->path);
		ugd->ug_Status.ug_iViewType = view_node->view_type;
		ug_error("================= top path is [%s] view_type is [%d] ", ugd->ug_Status.ug_pPath->str, ugd->ug_Status.ug_iViewType);
		mf_ug_navi_bar_create_default_view(ugd);
		/*}   modify by wangyan
		}   modify by wangyan*/
		mf_ug_navi_bar_set_ctrl_item_disable(ugd);
		elm_naviframe_item_title_enabled_set(ugd->ug_MainWindow.ug_pNaviItem, EINA_TRUE, EINA_FALSE);
		return false;
	} else {
		return true;
	}
	return true;

}

Eina_Bool mf_ug_cb_back_button_cb(void *data, Elm_Object_Item *it)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, EINA_FALSE, "ugData is NULL");

	if (ugd->ug_Status.ug_bCancelDisableFlag) {
		return EINA_FALSE;
	}
	if (0 != ugd->ug_ListPlay.ug_Player) {
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}
	if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
		ugd->ug_Status.ug_iViewType = mf_ug_view_normal;
		ugd->ug_Status.ug_iCheckedCount = 0;
		ugd->ug_Status.ug_bSelectAllChecked = EINA_FALSE;
		mf_ug_create_rintone_view(ugd);
		mf_ug_main_update_ctrl_in_idle(ugd);
		return EINA_FALSE;
	}
	if (ugd->ug_Status.ug_iMore == UG_MORE_SEARCH) {
		Evas_Object *playout = ugd->ug_MainWindow.ug_pNaviLayout;
		ug_mf_retvm_if(playout == NULL, EINA_FALSE, "get conformant failed");
		Evas_Object *newContent = NULL;

		newContent = mf_ug_genlist_create_content_list_view(ugd);

		Evas_Object *unUsed = elm_object_part_content_unset(playout, "part1");
		evas_object_del(unUsed);

		elm_object_part_content_set(playout, "part1", newContent);
		ugd->ug_Status.ug_iMore = UG_MORE_DEFAULT;
	} else {
		bool is_exit_ug = true;/*Fix the P130924-02121 bug*/
		if (g_is_press_cancel_button == true) {
			is_exit_ug = true;
		} else {
			is_exit_ug = mf_ug_cb_back_operation(ugd);
		}
		g_is_press_cancel_button = false;/*initiate it.*/
		ug_error("is_exit_ug is [%d]", is_exit_ug);
		if (is_exit_ug) {
			/*Fix the P130910-01714 problem, when back from the UG, the other app will be crashed. need to communicate with other app, then apply the new code.
			Fix the P131009-01740, and P130902-01617*/
			if (!mf_ug_ringtone_present_del_result(ugd)) {
				app_control_h service = NULL;
				int ret = app_control_create(&service);
				if (ret == APP_CONTROL_ERROR_NONE) {
					ug_send_result_full(ugd->ug, service, APP_CONTROL_RESULT_FAILED);
					app_control_destroy(service);
				}
			}
			ug_destroy_me(ugd->ug);
			ugd->ug = NULL;
		}
	}
	UG_TRACE_END;
	return EINA_FALSE;

}

void mf_ug_cb_cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");
	g_is_press_cancel_button = true;
	mf_ug_cb_back_button_cb(ugd, NULL);
}

/******************************
** Prototype    : mf_ug_cb_add_button_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
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
static void __mf_ug_cb_ringtone_set(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(ugd->ug == NULL, "ugd is NULL");	/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	char *file_path = mf_ug_util_get_send_result(ugd);
	app_control_h app_control = NULL;
	if (mf_ug_is_silent(ugd, file_path)) {
		UG_SAFE_FREE_CHAR(file_path);
		file_path = g_strdup(SILENT);
		if (file_path) {
			SECURE_DEBUG("result is [%s]", file_path);
			int ret = 0;
			ret = app_control_create(&app_control);
			if (ret == APP_CONTROL_ERROR_NONE) {
				app_control_add_extra_data(app_control, "result", file_path);
				app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, file_path);
				ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(app_control);
			}
			SECURE_DEBUG("result is [%s]", file_path);
			UG_SAFE_FREE_CHAR(file_path);
		}
		ug_destroy_me(ugd->ug);
		ugd->ug = NULL;

	}	else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert
	             || mf_ug_is_default_ringtone(ugd, file_path)
	             || mf_ug_fm_svc_wapper_is_default_ringtone(ugd, file_path)/*Fixed P140612-01028, only support the added music file */) {
		if (file_path) {
			SECURE_DEBUG("result is [%s]", file_path);
			if (mf_ug_is_default_ringtone(ugd, file_path)) {
				UG_SAFE_FREE_CHAR(file_path);
				file_path = g_strdup(DEFAULT_RINGTONE_MARK);
			}
			int ret = 0;
			ret = app_control_create(&app_control);
			if (ret == APP_CONTROL_ERROR_NONE) {
				app_control_add_extra_data(app_control, "result", file_path);
				app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, file_path);
				ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(app_control);
			}
			SECURE_DEBUG("result is [%s]", file_path);
			UG_SAFE_FREE_CHAR(file_path);
		}
		ug_destroy_me(ugd->ug);
		ugd->ug = NULL;
	} else {
		if (mf_ug_ringtone_is_default(ugd->ug_UiGadget.ug_iSoundMode, file_path)) {
			int ret = 0;
			ret = app_control_create(&app_control);
			if (ret == APP_CONTROL_ERROR_NONE) {
				app_control_add_extra_data(app_control, "result", file_path);
				app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, file_path);
				ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(app_control);
			}
			SECURE_DEBUG("result is [%s]", file_path);
			UG_SAFE_FREE_CHAR(file_path);
			ug_destroy_me(ugd->ug);
			ugd->ug = NULL;
		}
	}
	return;
}

static bool __mf_ug_cb_normal_result_send(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retv_if(ugd == NULL, false);
	ug_mf_retv_if(ugd->ug == NULL, false);	/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	bool flag_exit = true;
	char *result = NULL;
	app_control_h app_control = NULL;

	if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE) {
		result = g_strdup(ugd->ug_Status.ug_pPath->str);
	} else {
		result = mf_ug_util_get_send_result(ugd);
	}

	if (result) {
		SECURE_ERROR("result is [%s]", result);
		int ret = 0;
		ret = app_control_create(&app_control);
		if (ret == APP_CONTROL_ERROR_NONE) {
			int count = 0;
			char **array = mf_ug_util_get_send_result_array(ugd, &count);
			int i = 0;
			if (array) {
				app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_SELECTED, (const char **)array, count);
				app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_PATH, (const char **)array, count);
				app_control_add_extra_data_array(app_control, "path", (const char **)array, count);
				for (i = 0; i < count; i++) {
					UG_SAFE_FREE_CHAR(array[i]);
				}
				UG_SAFE_FREE_CHAR(array);
			} else {
				ug_error("Invalid selection!!");
			}
			app_control_add_extra_data(app_control, "result", result);
			app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, result);
			ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
			app_control_destroy(app_control);
		} else {
			ug_error("failed to create app control.");
		}
		SECURE_DEBUG("result is [%s]", result);
		UG_SAFE_FREE_CHAR(result);
	} else {
		ug_error("Invalid selection!!");
	}
	return flag_exit;
}

#ifdef UG_OPERATION_SELECT_MODE
static bool __mf_ug_selected_mode_result_send(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retv_if(ugd == NULL, false);
	ug_mf_retv_if(ugd->ug == NULL, false);/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	bool flag_exist = true;
	app_control_h app_control = NULL;

	if (ugd->ug_UiGadget.ug_bOperationSelectFlag) {
		ug_error();
		int ret = 0;
		ret = app_control_create(&app_control);
		if (ret == APP_CONTROL_ERROR_NONE) {
			int count = 0;
			char **array = mf_ug_util_get_send_result_array(ugd, &count);
			int i = 0;
			if (array) {
				app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_SELECTED, (const char **)array, count);
				app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_PATH, (const char **)array, count);

				for (i = 0; i < count; i++) {
					UG_SAFE_FREE_CHAR(array[i]);
				}
				UG_SAFE_FREE_CHAR(array);
				ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(app_control);
			} else {
				ug_error("Invalid selection!!");
				app_control_destroy(app_control);
			}
		} else {
			ug_error("failed to create app control.");
		}
	} else {
		flag_exist = __mf_ug_cb_normal_result_send(ugd);

	}
	return flag_exist;
}
#endif

void mf_ug_cb_add_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	if (0 != ugd->ug_ListPlay.ug_Player) {
		ugd->ug_ListPlay.hiden_flag = true;
		mf_ug_list_play_destory_playing_file(ugd);
		mf_ug_list_disable_play_itc(ugd, false);;
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}
	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		__mf_ug_cb_ringtone_set(ugd);
		return;
	}

#ifdef UG_OPERATION_SELECT_MODE
	if (__mf_ug_selected_mode_result_send(ugd)) {
		ug_destroy_me(ugd->ug);
		ugd->ug = NULL;
	}
#else
	if (__mf_ug_cb_normal_result_send(ugd)) {
		ug_destroy_me(ugd->ug);
		ugd->ug = NULL;
	}
#endif
	UG_TRACE_END;
}

void mf_ug_cb_delete_button_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	ugData *ugd = (ugData *)data;
	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);
	ug_error("label = %s", label);
	if (g_strcmp0(label, mf_ug_widget_get_text(MF_UG_LABEL_DELETE)) == 0) {
		mf_ug_cb_delete_button_cb(data, obj, event_info);
		UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNormalPopup);
	} else {
		UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNormalPopup);
	}
}

void mf_ug_cb_delete_button_popup_create(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	if (ugd->ug_MainWindow.ug_pNormalPopup) {
		UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNormalPopup);
	}

	ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT_TWO_BTN, NULL, MF_UG_BUTTON_LABEL_DEL, MF_UG_LABEL_CANCEL, MF_UG_LABEL_DELETE, NULL, mf_ug_cb_delete_button_confirm_cb, ugd);
	UG_TRACE_END;
}


void mf_ug_cb_delete_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	if (0 != ugd->ug_ListPlay.ug_Player) {
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}

	Evas_Object *content = ugd->ug_MainWindow.ug_pNaviGenlist;
	Elm_Object_Item *gli = elm_genlist_first_item_get(content);
	Elm_Object_Item *nli = NULL;
	while (gli) {
		ugListItemData *params = (ugListItemData *)elm_object_item_data_get(gli);
		ug_mf_retm_if(params == NULL, "params is NULL");
		if (params->ug_pCheckBox) {
			if (params->ug_bChecked == true) {
				if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
					mf_ug_db_handle_del_ringtone(params->ug_pItemName->str);
				} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
					mf_ug_db_handle_del_alert(params->ug_pItemName->str);
				}
			}
		}
		nli = elm_genlist_item_next_get(gli);
		gli = nli;
	}

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
		ugd->ug_Status.ug_iViewType = mf_ug_view_normal;
		mf_ug_create_rintone_view(ugd);
		return;
	}
	UG_TRACE_END;
}


/******************************
** Prototype    : _ug_popup_exit
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

void mf_ug_cb_mass_storage_popup_cb(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	mf_ug_cb_back_button_cb(ugd, NULL);

	if (ugd->ug_MainWindow.ug_pNormalPopup) {
		evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = NULL;
	}
	UG_TRACE_END;
}

void mf_ug_cb_upper_button_pressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	Evas_Object *upper_ic = (Evas_Object *)data;

	elm_image_file_set(upper_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_UPPER_PRESS);
}

void mf_ug_cb_upper_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	Evas_Object *upper_ic = (Evas_Object *)data;
	elm_image_file_set(upper_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_UPPER);
}

/******************************
** Prototype    : mf_ug_cb_upper_click_cb
** Description  : Samsung
** Input        : void *data
**                Evas_Object * obj
**                void *event_info
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
void mf_ug_cb_upper_click_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	if (ugd->ug_Status.ug_pPath != NULL) {
		if (mf_ug_fm_svc_wapper_is_root_path(ugd->ug_Status.ug_pPath->str)) {
			ugd->ug_Status.ug_iViewType = mf_ug_view_root;
		}

		if (g_strcmp0(ugd->ug_Status.ug_pPath->str, mf_ug_widget_get_text(MF_UG_LABEL_PHONE)) == 0
		        || g_strcmp0(ugd->ug_Status.ug_pPath->str, mf_ug_widget_get_text(MF_UG_LABEL_MMC)) == 0) {
			mf_ug_navi_bar_create_default_view(ugd);
			return;
		}
		GString *new_path = NULL;
		char *file_dir = mf_dir_get(ugd->ug_Status.ug_pPath->str);
		if (file_dir && ugd->ug_Status.ug_iViewType != mf_ug_view_root) {
			new_path = g_string_new(file_dir);
			UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
			ugd->ug_Status.ug_pPath = new_path;
			free(file_dir);
			file_dir = NULL;
		} else {
			ug_debug("file_dir is NULL");
			if (file_dir) {
				free(file_dir);
				file_dir = NULL;
			}
		}
	} else {
		ugd->ug_Status.ug_pPath = g_string_new(mf_ug_widget_get_text(MF_UG_LABEL_PHONE));
	}
	mf_ug_navi_bar_create_default_view(ugd);
	mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	UG_TRACE_END;
}

void mf_ug_cb_home_button_pressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	Evas_Object *home_ic = (Evas_Object *)data;
	elm_image_file_set(home_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_HOME_PRESS);
}

void mf_ug_cb_home_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	Evas_Object *home_ic = (Evas_Object *)data;
	elm_image_file_set(home_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_HOME);
}

void mf_ug_cb_home_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	int storage = MF_UG_PHONE;
	ugd->ug_Status.ug_iViewType = mf_ug_view_root;

	storage = mf_ug_fm_svc_wapper_get_location(ugd->ug_Status.ug_pPath->str);
	switch (storage) {
	case MF_UG_PHONE:
		UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
		ugd->ug_Status.ug_pPath = g_string_new(PHONE_FOLDER);
		mf_ug_util_set_current_state(ugd, STATE_PHONE);
		break;
	case MF_UG_MMC:
		UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
		ugd->ug_Status.ug_pPath = g_string_new(MEMORY_FOLDER);
		mf_ug_util_set_current_state(ugd, STATE_MEMORY);
		break;
	default:
		return;
	}
	ugd->ug_Status.ug_iCheckedCount = 0;

	mf_ug_navi_bar_create_default_view(ugd);
	mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	mf_ug_util_path_push(ugd->ug_Status.ug_pPath->str, ugd->ug_Status.ug_iViewType);
	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_cb_list_play_cb
** Description  : Samsung
** Input        : ugListItemData *data
**                Evas_Object *obj
**                void *event_info
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
void mf_ug_cb_list_play_cb(ugListItemData *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");
	ug_mf_retm_if(itemData->ug_pData == NULL, "ug_pData is NULL");

	if (mf_ug_main_is_background()) {
		return;
	}

	mf_ug_list_play_play_music_item(itemData);

	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_cb_select_info_show_cb
** Description  : Samsung
** Input        : void *data
**                Evas *e
**                Evas_Object *obj
**                void *event_info
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
void mf_ug_cb_select_info_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	edje_object_signal_emit(_EDJ(ugd->ug_MainWindow.ug_pMainLayout), "elm,state,show,default", "elm");
	UG_TRACE_END;
	return;
}

/******************************
** Prototype    : mf_ug_cb_select_info_hide_cb
** Description  : Samsung
** Input        : void *data
**                Evas *e
**                Evas_Object *obj
**                void *event_info
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
void mf_ug_cb_select_info_hide_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	edje_object_signal_emit(_EDJ(ugd->ug_MainWindow.ug_pMainLayout), "elm,state,hide,default", "elm");
	UG_TRACE_END;
	return;
}

/******************************
** Prototype    : mf_ug_cb_select_info_timeout_cb
** Description  : Samsung
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
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
void mf_ug_cb_select_info_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	edje_object_signal_emit(_EDJ(ugd->ug_MainWindow.ug_pMainLayout), "elm,state,hide,default", "elm");
	UG_TRACE_END;
	return;
}

/******************************
** Prototype    : mf_ug_cb_mmc_changed_cb
** Description  : Samsung
** Input        : int storage_id
**		  storage_state_e state
**                void* data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2015/03/24
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_ug_cb_mmc_changed_cb(int storage_id, storage_state_e state, void *user_data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)user_data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(ugd->ug_Status.ug_pPath == NULL || ugd->ug_Status.ug_pPath->str == NULL, "ugd->ug_Status.ug_pPath is NULL");

	int optStorage = MF_UG_NONE;

	if (state == STORAGE_STATE_MOUNTED) {
		ugd->ug_Status.ug_iMmcFlag = MMC_ON;
		mf_ug_util_storage_insert_action(ugd, mf_ug_widget_get_text(MF_UG_LABEL_MMC));
	} else {
		if (state == STORAGE_STATE_REMOVED || state == STORAGE_STATE_UNMOUNTABLE) {
			optStorage = MF_UG_MMC;
			ugd->ug_Status.ug_iMmcFlag = MMC_OFF;
		}

		if (optStorage == MF_UG_NONE) {
			ug_debug("get removed storage failed");
			return;
		}
		mf_ug_util_mmc_remove_action(ugd);
		mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	}

	mf_ug_navi_bar_title_set(ugd);

	UG_TRACE_END;
	return;
}

void mf_ug_cb_default_ringtone_changed_cb(system_settings_key_e key, void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(ugd->ug_UiGadget.default_ringtone == NULL, "ugd->ugUiGadget.default_ringtone is NULL");

	char *default_ringtone = NULL;
	int retcode = -1;

	retcode = system_settings_get_value_string(key, &default_ringtone);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE)) {
		ug_error("failed to get default_ringtone");
	}

	if (default_ringtone) {
		UG_SAFE_FREE_CHAR(ugd->ug_UiGadget.default_ringtone);
		ugd->ug_UiGadget.default_ringtone = g_strdup(default_ringtone);
	}

	UG_TRACE_END;
	return;
}

/******************************
** Prototype    : mf_ug_cb_dir_update_cb
** Description  : Samsung
** Input        : mf_ug_inotify_event event
**                char *name
**                void *data
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
void mf_ug_cb_dir_update_cb(mf_ug_inotify_event event, char *name, void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	SECURE_DEBUG("event : %d, name : %s", event, name);

	ug_dir_event_t buffer;

	buffer.event = event;
	buffer.name = name;

	ecore_pipe_write(ugd->ug_UiGadget.ug_pInotifyPipe, &buffer, sizeof(buffer));
	UG_TRACE_END;

	return;
}

/******************************
** Prototype    : mf_ug_cb_dir_pipe_cb
** Description  : Samsung
** Input        : void *data
**                void *buffer
**                unsigned int nbyte
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
static GString *__mf_ug_cb_dir_pipe_get_parent(GString *path)
{

	ug_mf_retvm_if(path == NULL, NULL, "path is NULL");
	ug_mf_retvm_if(path->str == NULL, NULL, "path->str is NULL");

	if (mf_file_exists(path->str)) {
		return path;
	} else if (mf_ug_fm_svc_wapper_is_root_path(path->str)) {
		return path;
	} else {
		GString *parent = mf_ug_fm_svc_wrapper_get_file_parent_path(path);
		UG_SAFE_FREE_GSTRING(path);
		__mf_ug_cb_dir_pipe_get_parent(parent);
	}
	return path;
}

void mf_ug_cb_dir_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");


	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		return;
	}

	if (g_strcmp0(ugd->ug_Status.monitor_path, ugd->ug_Status.ug_pPath->str)) {
		return;
	}
	if (buffer) {
		ug_dir_event_t *msg = (ug_dir_event_t *) buffer;
		SECURE_DEBUG("event : %d, name : %s", msg->event, msg->name);

		Evas_Object *newContent = NULL;
		ugListItemData *itemData = NULL;
		Elm_Object_Item *it = NULL;
		GString *parent = NULL;
		char *path = NULL;
		int count = 0;

		switch (msg->event) {
		case UG_MF_INOTI_CREATE:
		case UG_MF_INOTI_MOVE_IN:
			/*/1 TODO:  add new item to list */
			if (msg->name) {
				path = g_strconcat(ugd->ug_Status.ug_pPath->str, "/", msg->name, NULL);
				int file_type = 0;
				if (mf_ug_file_attr_is_dir(path)) {
					file_type = UG_FILE_TYPE_DIR;
				} else {
					file_type = UG_FILE_TYPE_FILE;
				}
				if (ugd->ug_MainWindow.ug_pNaviGenlist == NULL) {

					Evas_Object *genlist = NULL;
					genlist = elm_genlist_add(ugd->ug_MainWindow.ug_pNaviBar);
					elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
					evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
					evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

					ugd->ug_MainWindow.ug_pNaviGenlist = genlist;
					evas_object_smart_callback_add(genlist, "selected", mf_ug_genlist_selected_gl, ugd);

					evas_object_del(elm_object_part_content_unset(ugd->ug_MainWindow.ug_pNaviLayout, "part1"));

					elm_object_part_content_set(ugd->ug_MainWindow.ug_pNaviLayout, "part1", newContent);
					elm_object_part_content_set(ugd->ug_MainWindow.ug_pNaviLayout, "part1", genlist);
				}
				if (file_type == UG_FILE_TYPE_DIR) {
					if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE ||
					        ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE ||
					        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE ||
					        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE ||
					        ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE ||
					        ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE ||
					        ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {

						mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, path, ugd, 0, &ugd->ug_Status.ug_1text1icon_itc);
					} else {
						int groupValue = elm_genlist_items_count(ugd->ug_MainWindow.ug_pNaviGenlist);
						mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, path, ugd, groupValue, &ugd->ug_Status.ug_1text3icon_itc);
					}
				} else {
					if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
						int groupValue = elm_genlist_items_count(ugd->ug_MainWindow.ug_pNaviGenlist);
						mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, path, ugd, groupValue, &ugd->ug_Status.ug_1text3icon_itc);
					} else if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE) {
						mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, path, ugd, 0, &ugd->ug_Status.ug_1text1icon_itc);
					} else {
						mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, path, ugd, 0, &ugd->ug_Status.ug_1text3icon_itc);
					}

				}

				if (path != NULL) {
					free(path);
					path = NULL;
				}
				ugd->ug_Status.ug_bNoContentFlag = EINA_FALSE;
				mf_ug_genlist_show_select_info(ugd);
			}
			break;
		case UG_MF_INOTI_DELETE:
		case UG_MF_INOTI_MOVE_OUT:
			/*/1 TODO:  remove item from list */
			path = g_strconcat(ugd->ug_Status.ug_pPath->str, "/", msg->name, NULL);
			it = elm_genlist_first_item_get(ugd->ug_MainWindow.ug_pNaviGenlist);
			while (it) {
				itemData = elm_object_item_data_get(it);
				if (itemData->ug_pItemName == NULL || itemData->ug_pItemName->str == NULL) {
					continue;
				}
				if (g_strcmp0(path, itemData->ug_pItemName->str) == 0) {
					if (ugd->ug_ListPlay.ug_pPlayFilePath && g_strcmp0(path, ugd->ug_ListPlay.ug_pPlayFilePath) == 0) {
						if (0 != ugd->ug_ListPlay.ug_Player) {
							mf_ug_list_play_destory_playing_file(ugd);
							ugd->ug_ListPlay.play_data = NULL;
							UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
						}
					}
					elm_object_item_del(it);
					break;
				}

				it = elm_genlist_item_next_get(it);
			}
			count = elm_genlist_items_count(ugd->ug_MainWindow.ug_pNaviGenlist);
			if (count == 0) {
				Evas_Object *nocontent = mf_ug_widget_nocontent_create(ugd->ug_MainWindow.ug_pMainLayout, MF_UG_LABEL_NO_RESULT, UG_ICON_MULTI_NO_CONTENTS);

				ugd->ug_Status.ug_bNoContentFlag = true;
				Evas_Object *unset = elm_object_part_content_unset(ugd->ug_MainWindow.ug_pNaviLayout, "part1");
				evas_object_del(unset);
				ugd->ug_MainWindow.ug_pNaviGenlist = NULL;
				elm_object_part_content_set(ugd->ug_MainWindow.ug_pNaviLayout, "part1", nocontent);
				/*return nocontent;*/
			}
			mf_ug_genlist_show_select_info(ugd);
			break;
		case UG_MF_INOTI_MODIFY:

			path = g_strconcat(ugd->ug_Status.ug_pPath->str, "/", msg->name, NULL);
			if (path) {
				it = elm_genlist_first_item_get(ugd->ug_MainWindow.ug_pNaviGenlist);
				while (it) {
					itemData = elm_object_item_data_get(it);
					if (itemData->ug_pItemName == NULL || itemData->ug_pItemName->str == NULL) {
						continue;
					}
					if (g_strcmp0(path, itemData->ug_pItemName->str) == 0) {
						UG_SAFE_FREE_CHAR(path);
						return;
					}

					it = elm_genlist_item_next_get(it);
				}
				UG_SAFE_FREE_CHAR(path);
			}
			ugd->ug_MainWindow.ug_pNaviGenlist = newContent = mf_ug_genlist_create_content_list_view(ugd);
			mf_ug_navi_bar_set_new_content(ugd->ug_MainWindow.ug_pNaviLayout, newContent);
			break;
		case UG_MF_INOTI_DELETE_SELF:
		case UG_MF_INOTI_MOVE_SELF:
			/*/1 TODO:  watching directory is removed, change current directory. */
		{
			GString *current = NULL;
			current = g_string_new(ugd->ug_Status.ug_pPath->str);
			parent = __mf_ug_cb_dir_pipe_get_parent(current);
			g_string_free(ugd->ug_Status.ug_pPath, TRUE);
			ugd->ug_Status.ug_pPath = NULL;
			ugd->ug_Status.ug_pPath = parent;
			mf_ug_navi_bar_create_default_view(ugd);
		}
		break;
		default:
			ug_mf_error("Unknown event");
			break;
		}
		mf_ug_navi_bar_set_ctrl_item_disable(ugd);
		if (msg->name) {
			free(msg->name);
			msg->name = NULL;
		}
	}
	UG_TRACE_END;
}

void __mf_ug_popup_show_vk(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	if (ugd->ug_MainWindow.ug_pNormalPopup) {
		evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = NULL;
	}

	if (ugd->ug_MainWindow.ug_pEntry) {
		elm_entry_cursor_end_set(ugd->ug_MainWindow.ug_pEntry);
		elm_object_focus_set(ugd->ug_MainWindow.ug_pEntry, EINA_TRUE);
	}
	UG_TRACE_END;

}

void mf_ug_cb_create_new_folder(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	ugData *ugd = (ugData *)data;
	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pContextPopup);

	if (ugd->ug_Status.ug_iMore == UG_MORE_CREATE_FOLDER) {
		return;
	}

	int ret = 0;
	ugd->ug_Status.ug_iMore = UG_MORE_CREATE_FOLDER;

	ret = mf_ug_util_check_disk_space(ugd);
	if (ret == MYFILE_ERR_NO_FREE_SPACE) {
		ugd->ug_Status.ug_iMore = UG_MORE_DEFAULT;
		return;
	}
	ugd->ug_MainWindow.ug_pNewFolderPopup = mf_ug_popup_create_new_folder_popup(ugd, MF_UG_LABEL_CREATE);

}

void mf_ug_cb_warning_popup_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	if (g_strcmp0(label, mf_ug_widget_get_text(MF_UG_LABEL_OK)) == 0) {
		evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = NULL;
	}
}

void mf_ug_cb_reach_max_len_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	mf_ug_popup_indicator_popup(mf_ug_widget_get_text(MF_UG_LABEL_MAX_CHARACTER_REACHED));

	UG_TRACE_END;
}

void mf_ug_cb_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_entry_entry_set(data, "");
}

void mf_ug_cb_cancel_new_folder_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	ugd->ug_Status.ug_iMore = UG_MORE_DEFAULT;
	ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(ugd->ug_MainWindow.ug_pEntry));
	evas_object_del(ugd->ug_MainWindow.ug_pEntry);

	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNewFolderPopup);
}

static int __mf_ug_cb_ime_mkdir_cb(void *data, char *fileName)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;

	ug_mf_retvm_if(ugd == NULL, MYFILE_ERR_SRC_ARG_INVALID, "ugd is NULL");
	ug_mf_retvm_if(fileName == NULL, MYFILE_ERR_SRC_ARG_INVALID, "data is NULL");
	ug_mf_retvm_if(ugd->ug_Status.ug_pPath == NULL, MYFILE_ERR_SRC_ARG_INVALID, "ugd is NULL");
	ug_mf_retvm_if(ugd->ug_Status.ug_pPath->str == NULL, MYFILE_ERR_SRC_ARG_INVALID, "ugd is NULL");


	int ret = 0;
	const char *message = NULL;

	if (strlen(fileName)) {

		GString *fullpathdir = g_string_new(ugd->ug_Status.ug_pPath->str);

		if (fullpathdir == NULL) {
			mf_ug_util_operation_alloc_failed(ugd);
			return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
		}
		GString *fullpathname = g_string_new(fileName);

		if (fullpathname == NULL) {
			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			mf_ug_util_operation_alloc_failed(ugd);
			return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
		}
		/*check the space */
		gchar *test_space = g_strdup(fileName);
		if (test_space == NULL) {
			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;
			mf_ug_util_operation_alloc_failed(ugd);
			return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
		}
		if (strlen(g_strchug(test_space)) == 0) {
			message = MF_UG_LABEL_NAME_INVALID;	/*TODO */
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message,
			                                     NULL, NULL, NULL,
			                                     __mf_ug_popup_show_vk, ugd);

			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;
			g_free(test_space);
			test_space = NULL;
			return MYFILE_ERR_DIR_CREATE_FAIL;
		}

		g_free(test_space);
		test_space = NULL;

		/*check if input name is valid */
		if (mf_ug_file_attr_is_valid_name(fullpathname->str) == MYFILE_ERR_INVALID_FILE_NAME) {

			message = MF_UG_LABEL_ILLEGAL_CHAR;
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message,
			                                     NULL, NULL, NULL,
			                                     __mf_ug_popup_show_vk, ugd);
			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;

			return MYFILE_ERR_DIR_CREATE_FAIL;
		}

		GString *fullpath = NULL;
		char *temp_fullpath = g_strconcat(fullpathdir->str, "/", fullpathname->str, NULL);
		if (temp_fullpath) {
			fullpath = g_string_new(temp_fullpath);

			free(temp_fullpath);
			temp_fullpath = NULL;
			if (fullpath == NULL) {
				g_string_free(fullpathdir, TRUE);
				fullpathdir = NULL;
				g_string_free(fullpathname, TRUE);
				fullpathname = NULL;
				mf_ug_util_operation_alloc_failed(ugd);
				return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
			}
		} else {
			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;
			return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
		}

		/*check whether DIR name is override(DIR name has no extention) */
		/*check whether path length is override */
		if ((strlen(fullpathdir->str) + strlen(fullpathname->str)) > MYFILE_FILE_PATH_LEN_MAX) {

			message = MF_UG_LABEL_MAX_CHARACTER_REACHED;
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message,
			                                     NULL, NULL, NULL,
			                                     __mf_ug_popup_show_vk, ugd);
			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;
			ret = MYFILE_ERR_DIR_CREATE_FAIL;
		}
		/*check if duplicated name */
		else if (mf_ug_file_attr_is_duplicated_name(ugd->ug_Status.ug_pPath->str, fileName) == MYFILE_ERR_DUPLICATED_NAME) {
			message = MF_UG_LABEL_DUP_NAME;
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message,
			                                     NULL, NULL, NULL,
			                                     __mf_ug_popup_show_vk, ugd);/*fixe P131022-06134*/

			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;
			ret = MYFILE_ERR_DIR_CREATE_FAIL;
		}
		/*check if DIR name is all spaces */
		else {
			ret = mf_ug_fm_svc_wrapper_create_service(ugd, fullpath);
			/*check whether operate on read only area */
			if (errno == EROFS) {
				message = MF_UG_LABEL_OPER_READ_ONLY;
				ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message, NULL, NULL, NULL, __mf_ug_popup_show_vk, ugd);
				ret = MYFILE_ERR_DIR_CREATE_FAIL;
			} else if (ret) {
				message = MF_UG_LABEL_CREATE_DIR_FAILED;
				ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message, NULL, NULL, NULL, __mf_ug_popup_show_vk, ugd);
			}

			g_string_free(fullpathdir, TRUE);
			fullpathdir = NULL;
			g_string_free(fullpathname, TRUE);
			fullpathname = NULL;
			g_string_free(fullpath, TRUE);
			fullpath = NULL;

		}
	} else {
		message = mf_ug_widget_get_text(MF_UG_LABEL_EMPTY_FOLDER_NAME);

		ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message,
		                                     NULL, NULL, NULL,
		                                     __mf_ug_popup_show_vk, ugd);
		ret = MYFILE_ERR_DIR_CREATE_FAIL;
	}
	return ret;
}


void mf_ug_cb_save_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	int ret = 0;
	const char *message = NULL;

	if (ugd->ug_Status.ug_iMore == UG_MORE_CREATE_FOLDER) {
		const char *entry_data = NULL;
		char *name = NULL;

		/*hide IMF*/
		if (ugd->ug_MainWindow.ug_pEntry != NULL) {
			Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(ugd->ug_MainWindow.ug_pEntry);
			if (imf_context != NULL) {
				ecore_imf_context_hide(imf_context);
			}
		}

		entry_data = elm_entry_entry_get(ugd->ug_MainWindow.ug_pEntry);

		if (entry_data) {
			name = elm_entry_markup_to_utf8(entry_data);
		} else {
			message = MF_UG_LABEL_GET_NAME_FAILED;
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, message,
			                                     NULL, NULL, NULL,
			                                     NULL, NULL);
			return;
		}

		if (name == NULL) {
			return;
		}

		ret = __mf_ug_cb_ime_mkdir_cb(ugd, name);
		if (ret == 0) {
			ugd->ug_Status.ug_iMore = UG_MORE_DEFAULT;
			ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(ugd->ug_MainWindow.ug_pEntry));
			evas_object_del(ugd->ug_MainWindow.ug_pEntry);
			UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNewFolderPopup);
			mf_ug_navi_bar_create_default_view(ugd);
		}

		if (name != NULL) {
			free(name);
			name = NULL;
		}
	}
}

#if 0
void mf_ug_cb_lcd_state_changed_cb(power_state_e state, void *user_data)
{
	UG_TRACE_BEGIN;

	ug_mf_retm_if(user_data == NULL, "user_data is NULL");
	ugData *ugd = (ugData *)user_data;

	if (state == POWER_STATE_SCREEN_OFF) {
		if (0 != ugd->ug_ListPlay.ug_Player) {
			mf_ug_list_play_destory_playing_file(ugd);
			mf_ug_list_disable_play_itc(ugd, true);
			ugd->ug_ListPlay.play_data = NULL;
			UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
		}
	}
	UG_TRACE_END;
}
#endif

Eina_Bool mf_ug_cb_popup_del_idler_cb(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");
	ugData *ugd = (ugData *)data;

	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pSearchPopup);
	ugd->ug_MainWindow.ug_pSearchLabel = NULL;
	ugd->ug_Status.popup_del_idler = NULL;

	return ECORE_CALLBACK_CANCEL;
}

void mf_ug_cb_entry_button_pressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	Evas_Object *home_button_ic = (Evas_Object *)data;
	elm_image_file_set(home_button_ic, UG_EDJ_IMAGE, UG_ICON_ENTRY_FOLDER_PRESS);
}

void mf_ug_cb_entry_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");

	Evas_Object *home_button_ic = (Evas_Object *)data;
	elm_image_file_set(home_button_ic, UG_EDJ_IMAGE, UG_ICON_ENTRY_FOLDER);
}

void mf_ug_cb_play_button_pressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	const char *play_icon = NULL;

	Evas_Object *music_icon = elm_object_part_content_get(obj, "icon");
	if (ugd->ug_ListPlay.ug_iPlayState != PLAY_STATE_PLAYING) {
		play_icon = UG_ICON_MUSIC_PLAY_WHITE_PRESS;
	} else {
		play_icon = UG_ICON_MUSIC_PAUSE_WHITE_PRESS;
	}
	elm_image_file_set(music_icon, UG_EDJ_IMAGE, play_icon);
	evas_object_size_hint_min_set(music_icon, ELM_SCALE_SIZE(45), ELM_SCALE_SIZE(45));
}

void mf_ug_cb_play_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	const char *play_icon = NULL;

	Evas_Object *music_icon = elm_object_part_content_get(obj, "icon");
	if (ugd->ug_ListPlay.ug_iPlayState != PLAY_STATE_PLAYING) {
		play_icon = UG_ICON_MUSIC_PLAY_WHITE;
	} else {
		play_icon = UG_ICON_MUSIC_PAUSE_WHITE;
	}
	elm_image_file_set(music_icon, UG_EDJ_IMAGE, play_icon);
	evas_object_size_hint_min_set(music_icon, 45, 45);
}

void mf_ug_cb_more_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	mf_ug_context_popup_create_more(ugd, obj);
}

void mf_ug_cb_thumb_created_cb(media_content_error_e error, const char *path, void *user_data)
{
	ug_mf_retm_if(user_data == NULL, "user_data is NULL");
	ug_mf_retm_if(path == NULL, "path is NULL");
	ugListItemData *pListData = (ugListItemData *)user_data;
	ug_mf_retm_if(pListData->ug_pItem == NULL, "pListData->item is NULL");

	if (error == MEDIA_CONTENT_ERROR_NONE && mf_file_exists(path)) {
		ug_debug("Update item with new thumbnail[%s]", path);
		UG_SAFE_FREE_CHAR(pListData->ug_pThumbPath);
		pListData->ug_pThumbPath = g_strdup(path);
		pListData->ug_bRealThumbFlag = true;
		if (pListData->ug_pItem) {
			elm_genlist_item_update(pListData->ug_pItem);
		}
	} else {
		ug_debug("Invalid thumb path!");
	}
	pListData->thumbnail_create = EINA_FALSE;
}

bool mf_ug_cb_create_thumbnail(void *data, media_thumbnail_completed_cb callback)
{
	ug_mf_retvm_if(data == NULL, -1, "filter is NULL");
	ugListItemData *pListData = (ugListItemData *)data;

	int ret = -1;

	ret = media_info_create_thumbnail(pListData->media, callback,
	                                  pListData);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ug_debug("Failed to create thumbnail! ret is [%d]", ret);
		return -1;
	}
	return 0;
}

void mf_ug_ringtone_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pContextPopup);
	ugd->ug_Status.ug_iViewType = mf_ug_view_ringtone_del;
	mf_ug_navi_bar_create_delete_view(ugd);
	return;
}

void mf_ug_select_all_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	Elm_Object_Item *it = NULL;
	ugListItemData *it_data = NULL;
	bool state = true;
	int prev_check_count = ugd->ug_Status.ug_iCheckedCount;
	if (elm_check_state_get(obj) == 1) {
		ugd->ug_Status.ug_iCheckedCount = eina_list_count(ugd->ug_UiGadget.ug_pFilterList);
	} else {
		ugd->ug_Status.ug_iCheckedCount = 0;
	}
	long long int total_item_size = 0;
	struct stat stFileInfo;

	if (ugd->ug_Status.ug_iCheckedCount > ugd->ug_UiGadget.ug_iMaxLength) {
		ug_error();
		elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, false);
		char *reach_string = mf_ug_widget_get_text(MF_UG_LABEL_REACH_MAX_SHARE_COUNT);
		char *max_string = g_strdup_printf(reach_string, ugd->ug_UiGadget.ug_iMaxLength);
		/*UG_SAFE_FREE_CHAR(reach_string);*/
		ugd->ug_Status.ug_iCheckedCount = prev_check_count;
		ug_error("max_string is [%s]", max_string);
		mf_ug_popup_indicator_popup(max_string);
		UG_SAFE_FREE_CHAR(max_string);
		return;
	}
	it = elm_genlist_first_item_get(ugd->ug_MainWindow.ug_pNaviGenlist);
	while (it) {
		it_data = elm_object_item_data_get(it);
		stat(it_data->ug_pItemName->str, &stFileInfo);
		total_item_size += stFileInfo.st_size;
		if (ugd->limitsize > 0 && total_item_size > ugd->limitsize) {
			char *noti = NULL;
			noti = g_strdup_printf("%s", mf_ug_widget_get_text(MF_UG_LABEL_MAXIMUM_SIZE));
			mf_ug_popup_indicator_popup(noti);
			elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, false);
			ugd->ug_Status.ug_iCheckedCount = prev_check_count;
			state = false;

			UG_SAFE_FREE_CHAR(noti);
			return;
		}
		it = elm_genlist_item_next_get(it);
	}
	if (state) {
		ugd->selsize = total_item_size;
		it = elm_genlist_first_item_get(ugd->ug_MainWindow.ug_pNaviGenlist);
		while (it) {
			it_data = elm_object_item_data_get(it);
			if (elm_check_state_get(obj) == 0) {
				it_data->ug_bChecked = 0;
			} else {
				it_data->ug_bChecked = 1;
			}
			elm_check_state_set(it_data->ug_pCheckBox, it_data->ug_bChecked);
			elm_genlist_item_update(it);

			it = elm_genlist_item_next_get(it);
		}

	}
	mf_ug_genlist_show_select_info(ugd);
	mf_ug_navi_bar_set_ctrl_item_disable(ugd);

	return;
}

void mf_ug_item_sel_all_press_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	if (ugd->ug_MainWindow.ug_pSelectAllCheckBox) {
		Eina_Bool state = elm_check_state_get(ugd->ug_MainWindow.ug_pSelectAllCheckBox);
		elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, !state);
		mf_ug_select_all_cb(data, ugd->ug_MainWindow.ug_pSelectAllCheckBox, NULL);
	}
	return;
}

