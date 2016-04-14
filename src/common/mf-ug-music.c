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




//#include <ui-gadget.h>
#include <app.h>

#include "mf-ug-main.h"
#include "mf-ug-util.h"
#include "mf-ug-db-handle.h"
#include "mf-ug-winset.h"
#include "mf-ug-list-play.h"
#include "mf-ug-fm-svc-wrapper.h"
#include <unistd.h>
#include "mf-ug-resource.h"
#include "mf-ug-cb.h"
#include "mf-ug-file-util.h"
#if 0 //Chandan
static ui_gadget_h music_ug = NULL;
#endif
Eina_Bool mf_ug_is_music_ug_run()
{
	UG_TRACE_BEGIN;
	#if 0 //Chandan
	if (music_ug) {
		UG_TRACE_END;
		return EINA_TRUE;
	}
	#endif
	UG_TRACE_END;
	return EINA_FALSE;
}
void mf_ug_destory_music_ug()
{
	UG_TRACE_BEGIN;
	#if 0 //Chandan
	if (music_ug) {
		ug_destroy(music_ug);
		music_ug = NULL;
	}
#endif
	UG_TRACE_END;
}

void __mf_ug_music_request_send(void *data, const char *path)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");
	ug_mf_retm_if(path == NULL, "path is NULL");
	//ug_mf_retm_if(ugd->ug == NULL, "ugd->ugis NULL");/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	SECURE_ERROR("result is [%s]", path);
	int ret = 0;
	app_control_h app_control = NULL;
	ret = app_control_create(&app_control);
	if (ret == APP_CONTROL_ERROR_NONE) {

		int count = 1;
		char **array = NULL;

		array = calloc(count, sizeof(char *));
		if (array) {
			array[0] = g_strdup(path);
			app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_SELECTED, (const char **)array, count);
			app_control_add_extra_data_array(app_control, "path", (const char **)array, count);
			UG_SAFE_FREE_CHAR(array[0]);
			UG_SAFE_FREE_CHAR(array);
		}
		app_control_add_extra_data(app_control, "result", path);
		app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, path);

		bool reply_requested = false;
		app_control_is_reply_requested(app_control, &reply_requested);
		if (reply_requested) {
			SECURE_DEBUG("send reply to caller");
			app_control_h reply = NULL;
			app_control_create(&reply);
			app_control_reply_to_launch_request(reply, app_control, APP_CONTROL_RESULT_SUCCEEDED);
			app_control_destroy(reply);
		}
//		ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(app_control);
//		ug_destroy_me(ugd->ug);
//		ugd->ug = NULL;
	}

}


void  __mf_ug_service_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)user_data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");
	mf_ug_player_vol_set(ugd, ugd->ug_Status.ug_pEntryPath);
	switch (result) {
	case APP_CONTROL_RESULT_SUCCEEDED:
		break;
	case APP_CONTROL_RESULT_FAILED:
		break;
	case APP_CONTROL_RESULT_CANCELED:
		break;
	default:
		break;
	}
	char *music_path = NULL;
	app_control_get_extra_data(reply, "uri", &music_path);
	/*__mf_ug_music_request_send(user_data, music_path);*/


	if (music_path) {
		int location = mf_ug_fm_svc_wapper_get_location(music_path);
		int ret = MFD_ERROR_NONE;
		if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
			ret = mf_ug_db_handle_add_ringtone(music_path, NULL, location);
		} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
			ret = mf_ug_db_handle_add_alert(music_path, NULL, location);
		}
		Evas_Object *rbtn = elm_object_item_part_content_get(ugd->ug_MainWindow.ug_pNaviItem, TITLE_RIGHT_BTN);
		if (rbtn) {
			elm_object_disabled_set(rbtn, EINA_FALSE);
		}
		if (ret == MFD_ERROR_NONE) {/*if there isn't the record, we will add the music.*/
			Elm_Object_Item *default_item = mf_ug_genlist_default_item_get();
			if (default_item) {
				mf_ug_genlist_first_item_insert(ugd, music_path, default_item);
			} else {
				mf_ug_genlist_first_item_append(ugd, music_path);
			}
			mf_ug_navi_bar_set_ctrl_item_disable(ugd);
		} else if (ret == MFD_ERROR_FILE_EXSITED) {/*To fix P131209-06058 wangyan,if there is this record,check this record and top it*/
			/*check this record*/
			mf_ug_genlist_item_bringin_top(ugd, music_path);
			mf_ug_navi_bar_set_ctrl_item_disable(ugd);
		}
	}
}
#if 0//Chandan
static void __mf_ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(priv == NULL, "priv is NULL");

	Evas_Object *base = NULL;

	base = ug_get_layout(ug);
	if (!base) {
		ug_destroy(ug);
		return;
	}

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(base);
		break;
	default:
		break;
	}
}


static void __mf_ug_destory_cb(ui_gadget_h ug, void *priv)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(priv == NULL, "priv is NULL");

	ug_destroy(ug);

	UG_TRACE_END;
}
#endif
static void __mf_ug_music_recommendation_ringtone_set(void *data, char *path, char *time)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
//	ug_mf_retm_if(ugd->ug == NULL, "ugd->ug is NULL");	/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	ug_error(" file is [%s] time is [%s]", path, time);
	char *result = NULL;

	app_control_h service = NULL;
	result = g_strdup(path); /*mf_ug_util_get_send_result(ugd);*/

	SECURE_ERROR("result is [%s]", result);

	if (mf_ug_is_default_ringtone(ugd, result)) {
		UG_SAFE_FREE_CHAR(result);
		result = g_strdup(DEFAULT_RINGTONE_MARK);
		if (result) {
			SECURE_ERROR("result is [%s]", result);
			int ret = 0;
			ret = app_control_create(&service);
			if (ret == APP_CONTROL_ERROR_NONE) {
				app_control_add_extra_data(service, "result", result);
				app_control_add_extra_data(service, "position", time);
				app_control_add_extra_data(service, APP_CONTROL_DATA_SELECTED, result);

				bool reply_requested = false;
				app_control_is_reply_requested(service, &reply_requested);
				if (reply_requested) {
					SECURE_DEBUG("send reply to caller");
					app_control_h reply = NULL;
					app_control_create(&reply);
					app_control_reply_to_launch_request(reply, service, APP_CONTROL_RESULT_SUCCEEDED);
					app_control_destroy(reply);
				}
//				ug_send_result_full(ugd->ug, service, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(service);
			}
		}
	} else {
		int ret = 0;
		ret = app_control_create(&service);
		if (ret == APP_CONTROL_ERROR_NONE) {

			int count = 0;
			char **array = mf_ug_util_get_send_result_array(ugd, &count);
			int i = 0;
			if (array) {
				app_control_add_extra_data_array(service, APP_CONTROL_DATA_SELECTED, (const char **)array, count);
				app_control_add_extra_data_array(service, "path", (const char **)array, count);
				for (i = 0; i < count; i++) {
					UG_SAFE_FREE_CHAR(array[i]);
				}
				UG_SAFE_FREE_CHAR(array);
			}
			app_control_add_extra_data(service, "result", result);
			app_control_add_extra_data(service, "position", time);
			app_control_add_extra_data(service, APP_CONTROL_DATA_SELECTED, result);
			bool reply_requested = false;
			app_control_is_reply_requested(service, &reply_requested);
			if (reply_requested) {
				SECURE_DEBUG("send reply to caller");
				app_control_h reply = NULL;
				app_control_create(&reply);
				app_control_reply_to_launch_request(reply, service, APP_CONTROL_RESULT_SUCCEEDED);
				app_control_destroy(reply);
			}
//			ug_send_result_full(ugd->ug, service, APP_CONTROL_RESULT_SUCCEEDED);
			app_control_destroy(service);
		}

	}
//	ug_destroy_me(ugd->ug);
//	ugd->ug = NULL;

}
#if 0//Chandan
void  __mf_ug_reply_cb(ui_gadget_h ug, app_control_h result, void *priv)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)priv;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");

	char *music_path = NULL;
	char *position = NULL;
	app_control_get_extra_data(result, "uri", &music_path);
	app_control_get_extra_data(result, "position", &position);
	/*__mf_ug_music_request_send(user_data, music_path);*/
	mf_ug_player_vol_set(ugd, ugd->ug_Status.ug_pEntryPath);

	if (music_path && !mf_file_exists(music_path)) {
		mf_ug_destory_music_ug();
		char *message = MF_UG_LABEL_ADD_FAILED;
		ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TITLE_TEXT_BTN, MF_UG_POP_TITLE_OPERATION_ERROR, message, NULL, NULL, NULL, __mf_ug_popup_show_vk, ugd);
	} else if (music_path) {
		int location = mf_ug_fm_svc_wapper_get_location(music_path);
		/*int ret = MFD_ERROR_NONE;*/
		if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
			mf_ug_db_handle_add_ringtone(music_path, NULL, location);
		} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
			mf_ug_db_handle_add_alert(music_path, NULL, location);
		}
		mf_ug_destory_music_ug();
		__mf_ug_music_recommendation_ringtone_set(ugd, music_path, position);
		return;
	}
	/*mf_ug_destory_music_ug();*/
}
void mf_ug_music_select(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	ui_gadget_h ug = NULL;
	struct ug_cbs cbs = { 0, };

	app_control_h app_control;
	int ret = 0;
	ret = app_control_create(&app_control);
	ug_mf_retm_if(ret != APP_CONTROL_ERROR_NONE, "app_control create failed");
	cbs.layout_cb = __mf_ug_layout_cb;
	cbs.result_cb = __mf_ug_reply_cb;
	cbs.destroy_cb = __mf_ug_destory_cb;
	cbs.priv = data;

	ret = app_control_add_extra_data(app_control, "request_type", "SelectRingtone");
	ret = app_control_add_extra_data(app_control, "select_uri", ugd->ug_Status.mark_mode);
	if (ret != APP_CONTROL_ERROR_NONE) {
		goto LAUNCH_END;
	}

	UG_INIT_EFL(ug_get_window(), UG_OPT_INDICATOR_ENABLE);

	ug = ug_create(NULL, "music-player-efl", UG_MODE_FULLVIEW, app_control, &cbs);
	if (ug != NULL) {
		music_ug = ug;
	}
LAUNCH_END:
	if (app_control) {
		app_control_destroy(app_control);
	}

	UG_TRACE_END;
}
#endif
void mf_ug_music_launch_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");
	/*To fix P131118-01579 by wangyan,the pause/play icon do not locate at the item whose radio icon is on. */
	/*destory playfile when go to add music file option*/
	if (0 != ugd->ug_ListPlay.ug_Player) {
		mf_ug_list_disable_play_itc(ugd, true);
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}

	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pContextPopup);
	//mf_ug_music_select(ugd); Chandan
}

