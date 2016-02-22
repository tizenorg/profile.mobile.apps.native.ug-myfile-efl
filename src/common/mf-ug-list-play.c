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
#include <stdbool.h>

#include <Elementary.h>
#include <runtime_info.h>

#include "mf-ug-main.h"
#include "mf-ug-conf.h"
#include "mf-ug-cb.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-util.h"
#include "mf-ug-winset.h"
#include "mf-ug-dlog.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-resource.h"
#include "mf-ug-list-play.h"
#include "mf-ug-widget.h"
#include "mf-ug-file-util.h"

static mf_player_cbs *g_player_cbs = NULL;
static Ecore_Pipe *g_player_pipe = NULL;
sound_type_e g_init_current_type;
int g_init_volume = -1;

static void __mf_ug_list_play_control_cb(void *data);
static bool __mf_ug_list_play_play_current_file(void *data);
static bool __mf_ug_list_play_stop(ugData *data);

static sound_type_e __mf_ug_list_play_sound_type(const char *path)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(path == NULL, SOUND_TYPE_RINGTONE, "path is NULL");
	if (mf_ug_main_is_background()) {
		return SOUND_TYPE_RINGTONE;
	}

	sound_type_e type = SOUND_TYPE_RINGTONE;
	if (g_strcmp0(path, UG_SETTING_MSG_ALERTS_PATH) == 0) {
		type = SOUND_TYPE_NOTIFICATION;
	}
	if (g_strcmp0(path, UG_SETTING_ALERTS_PATH) == 0 || g_strcmp0(path, UG_SETTING_SMART_ALRAMS) == 0) {
		type = SOUND_TYPE_ALARM;
	}

	return type;
}


/******************************
** Prototype    : __mf_ug_list_play_init_data
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
static void __mf_ug_list_play_init_data(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;

	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (ugd->ug_ListPlay.ug_Player != 0) {
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.ug_Player = 0;
	}
	ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_INIT;
	ugd->ug_ListPlay.play_data = NULL;
	UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_list_play_update_item_icon
** Description  :
** Input        : ugListItemData *data
**                int state
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
void mf_ug_list_item_play_btn_update(void *data)
{
	ugListItemData *itemData = (ugListItemData *)data;
	ugData *ugd = (ugData *)itemData->ug_pData;
	Evas_Object *music_icon = NULL;
	Evas_Object *music_button = NULL;

	const char *play_icon = NULL;
	char *pause_icon = NULL;
	music_button = elm_object_item_part_content_get(itemData->ug_pItem, "elm.icon.3");

	if (music_button) {
		music_icon = elm_object_part_content_get(music_button, "icon");
		if (music_icon) {
			if (ugd->ug_ListPlay.ug_pPlayFilePath) {
				if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, itemData->ug_pItemName->str) == 0) {
					if (ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PLAYING) {
						pause_icon = UG_ICON_MUSIC_PAUSE_WHITE;
						elm_image_file_set(music_icon, UG_EDJ_IMAGE, pause_icon);
					} else if (ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PAUSED) {
						play_icon = UG_ICON_MUSIC_PLAY_WHITE;
						elm_image_file_set(music_icon, UG_EDJ_IMAGE, play_icon);
					} else {
						play_icon = UG_ICON_MUSIC_PLAY_WHITE;
						elm_image_file_set(music_icon, UG_EDJ_IMAGE, play_icon);
					}
				} else {
					play_icon = UG_ICON_MUSIC_PLAY_WHITE;
					elm_image_file_set(music_icon, UG_EDJ_IMAGE, play_icon);
				}
			} else {
				play_icon = UG_ICON_MUSIC_PLAY_WHITE;
				elm_image_file_set(music_icon, UG_EDJ_IMAGE, play_icon);
			}
			evas_object_size_hint_min_set(music_icon, ELM_SCALE_SIZE(45), ELM_SCALE_SIZE(45));
		}
	}

}
void mf_ug_list_disable_play_itc(void *data, bool disable)
{
	UG_TRACE_BEGIN;

	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Elm_Object_Item *gl_item = NULL;
	ugListItemData *itemData = NULL;
	Evas_Object *pGenlist = NULL;

	if (ugd->ug_ListPlay.ug_pPlayFilePath == NULL || strlen(ugd->ug_ListPlay.ug_pPlayFilePath) == 0) {

		UG_TRACE_END;
		return;
	}
	pGenlist = ugd->ug_MainWindow.ug_pNaviGenlist;

	gl_item = elm_genlist_first_item_get(pGenlist);
	while (gl_item) {
		itemData = elm_object_item_data_get(gl_item);
		if (itemData && itemData->ug_pItemName) {
			if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, itemData->ug_pItemName->str) == 0) {
				if (disable) {
					UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
					ugd->ug_ListPlay.play_data = NULL;
				}
				if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
					elm_genlist_item_fields_update(itemData->ug_pItem, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
				} else {
					mf_ug_list_item_play_btn_update(itemData);
				}
				UG_TRACE_END;
				return;
			}
		}
		gl_item = elm_genlist_item_next_get(gl_item);
	}

	UG_TRACE_END;
}

void mf_ug_list_play_update_item_icon(void *data)
{
	UG_TRACE_BEGIN;

	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Elm_Object_Item *gl_item = NULL;
	ugListItemData *itemData = NULL;
	Evas_Object *pGenlist = NULL;

	if (ugd->ug_ListPlay.ug_pPlayFilePath == NULL || strlen(ugd->ug_ListPlay.ug_pPlayFilePath) == 0) {

		UG_TRACE_END;
		return;
	}
	pGenlist = ugd->ug_MainWindow.ug_pNaviGenlist;

	gl_item = elm_genlist_first_item_get(pGenlist);
	while (gl_item) {
		itemData = elm_object_item_data_get(gl_item);
		if (itemData && itemData->ug_pItemName) {
			if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, itemData->ug_pItemName->str) == 0) {
				if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
					elm_genlist_item_fields_update(itemData->ug_pItem, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
				} else {
					elm_genlist_item_fields_update(itemData->ug_pItem, "elm.icon.3", ELM_GENLIST_ITEM_FIELD_CONTENT);
				}
				UG_TRACE_END;
				return;
			}
		}
		gl_item = elm_genlist_item_next_get(gl_item);
	}

	UG_TRACE_END;
}


/******************************
** Prototype    : __mf_ug_list_play_set_play_start_status
** Description  :
** Input        : ugListItemData *data
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
static void __mf_ug_list_play_set_play_start_status(void *data, char *path)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_PLAYING;

	UG_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_list_play_set_play_resume_status
** Description  :
** Input        : ugListItemData *data
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
static void __mf_ug_list_play_set_play_resume_status(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_PLAYING;

	UG_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_list_play_set_play_pause_status
** Description  :
** Input        : ugListItemData *data
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
static void __mf_ug_list_play_set_play_pause_status(ugData *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_PAUSED;

	UG_TRACE_END;
}

/******************************
** Prototype    : _mp_player_mgr_create
** Description  :
** Input        : void *data
**                const gchar *path
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
static void __mf_player_mgr_callback_pipe_handler(void *data, void *buffer, unsigned int nbyte)
{
	UG_TRACE_BEGIN;
	mf_player_cb_extra_data *extra_data = buffer;
	ug_mf_retm_if(extra_data == NULL, "NULL");
	ug_mf_retm_if(g_player_cbs == NULL, "NULL");

	switch (extra_data->cb_type) {
		/*note: start callback and paused callback for player have been removed*/
		/*case MF_PLAYER_CB_TYPE_STARTED:
			if (g_player_cbs->started_cb)
				g_player_cbs->started_cb(g_player_cbs->user_data[MF_PLAYER_CB_TYPE_STARTED]);
			break;

		case MF_PLAYER_CB_TYPE_PAUSED:
			if (g_player_cbs->paused_cb)
				g_player_cbs->paused_cb(g_player_cbs->user_data[MF_PLAYER_CB_TYPE_PAUSED]);
			break;*/

	case MF_PLAYER_CB_TYPE_COMPLETED:
		if (g_player_cbs->completed_cb) {
			g_player_cbs->completed_cb(g_player_cbs->user_data[MF_PLAYER_CB_TYPE_COMPLETED]);
		}
		break;

/*	case MF_PLAYER_CB_TYPE_INTURRUPTED:
		if (g_player_cbs->interrupted_cb) {
			g_player_cbs->interrupted_cb(extra_data->param.interrupted_code, g_player_cbs->user_data[MF_PLAYER_CB_TYPE_INTURRUPTED]);
		}
		break;
*/
	case MF_PLAYER_CB_TYPE_ERROR:
		if (g_player_cbs->error_cb) {
			g_player_cbs->error_cb(extra_data->param.error_code, g_player_cbs->user_data[MF_PLAYER_CB_TYPE_ERROR]);
		}
		break;

	case MF_PLAYER_CB_TYPE_BUFFERING:
		if (g_player_cbs->buffering_cb) {
			g_player_cbs->buffering_cb(extra_data->param.percent, g_player_cbs->user_data[MF_PLAYER_CB_TYPE_BUFFERING]);
		}
		break;
	case MF_PLAYER_CB_TYPE_PREPARE:
		if (g_player_cbs->prepare_cb) {
			g_player_cbs->prepare_cb(g_player_cbs->user_data[MF_PLAYER_CB_TYPE_PREPARE]);
		}
		break;

	default:
		ug_debug("Not suppoted callback type [%d]", extra_data->cb_type);
	}
}


static void
__mf_ug_list_play_mgr_completed_cb(void *userdata)
{
	UG_TRACE_BEGIN;
	MF_CHECK(g_player_pipe);

	mf_player_cb_extra_data extra_data;
	extra_data.cb_type = MF_PLAYER_CB_TYPE_COMPLETED;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mf_player_cb_extra_data));
}

/*static void
__mf_ug_list_play_mgr_interrupted_cb(player_interrupted_code_e code, void *userdata)
{
	UG_TRACE_BEGIN;
	MF_CHECK(g_player_pipe);

	int error = SOUND_MANAGER_ERROR_NONE;
	mf_player_cb_extra_data extra_data;
	ugData *ugd = userdata;
	error = sound_manager_release_focus(ugd->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
	if (error != SOUND_MANAGER_ERROR_NONE) {
		ug_error("failed to release focus error[%x]", error);
		return;
	}
	extra_data.cb_type = MF_PLAYER_CB_TYPE_INTURRUPTED;
	extra_data.param.interrupted_code = code;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mf_player_cb_extra_data));
}*/


static void
__mf_ug_list_play_mgr_error_cb(int error_code, void *userdata)
{
	UG_TRACE_BEGIN;
	MF_CHECK(g_player_pipe);

	mf_player_cb_extra_data extra_data;
	extra_data.cb_type = MF_PLAYER_CB_TYPE_ERROR;
	extra_data.param.error_code = error_code;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mf_player_cb_extra_data));
}

static bool __mf_ug_list_play_set_uri(player_h player, const char *uri)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(player == NULL, false, "player is NULL");
	ug_mf_retvm_if(uri == NULL, false, "uri is NULL");
	if (mf_ug_main_is_background()) {
		return false;
	}

	int ret = 0;
	ret = player_set_uri(player, uri);
	/*player_set_prelistening_mode(player, PLAYER_PRELISTENING_MODE_MEDIA);
	player_set_sound_type(ugd->ug_ListPlay.ug_Player, SOUND_TYPE_MEDIA);*/
	if (ret != PLAYER_ERROR_NONE) {
		ug_error(">>>>>>>>>>>>>g_err_name : %d\n", ret);
		UG_TRACE_END;
		return false;
	} else {
		UG_TRACE_END;
		return true;
	}

}

void mf_player_focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	ugData *ugd = user_data;
	bool reacquire_state;

	sound_stream_focus_state_e state_for_playback;
	sound_stream_focus_state_e state_for_recording;
	int ret = -1;
	ret = sound_manager_get_focus_state(ugd->stream_info, &state_for_playback, &state_for_recording);

	if (state_for_playback == SOUND_STREAM_FOCUS_STATE_RELEASED) {
		if (reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_ALARM && reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_NOTIFICATION
				&& reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_CALL) {
			__mf_ug_list_play_stop(ugd);
		}
	}
}

static bool __mf_ug_list_play_set_sound_type(ugData *ugd)
{
	UG_TRACE_BEGIN;
	player_h player = ugd->ug_ListPlay.ug_Player;
	ug_mf_retvm_if(player == NULL, false, "player is NULL");
	int error = SOUND_MANAGER_ERROR_NONE;
	bool reacquire_state;

	int ret = 0;
	sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, mf_player_focus_callback, ugd, &ugd->stream_info);
	sound_manager_get_focus_reacquisition(ugd->stream_info, &reacquire_state);
	 if (reacquire_state == EINA_TRUE) {
		 sound_manager_set_focus_reacquisition(ugd->stream_info, EINA_FALSE);
	 }

	ret = player_set_audio_policy_info(player, ugd->stream_info);
	/*player_set_sound_type(ugd->ug_ListPlay.ug_Player, SOUND_TYPE_MEDIA);*/
	if (ret != PLAYER_ERROR_NONE) {
		ug_error(">>>>>>>>>>>>>g_err_name : %d\n", ret);
		UG_TRACE_END;
		return false;
	} else {
		UG_TRACE_END;
		return true;
	}

}

static bool __mf_ug_list_play_create_player(player_h *player)
{
	UG_TRACE_BEGIN;
	if (mf_ug_main_is_background()) {
		return false;
	}

	int ret = 0;
	ret = player_create(player);
	/*player_set_sound_type(ugd->ug_ListPlay.ug_Player, SOUND_TYPE_MEDIA);*/
	if (ret != PLAYER_ERROR_NONE) {
		ug_error(">>>>>>>>>>>>>g_err_name : %d\n", ret);
		UG_TRACE_END;
		return false;
	} else {
		UG_TRACE_END;
		return true;
	}

}

static bool __mf_ug_list_play_create_player_mgr(void *data, const char *path)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	if (mf_ug_main_is_background()) {
		return false;
	}

	int path_len = strlen(path);
	int ret = 0;

	if (path_len > 0 && path_len < MYFILE_DIR_PATH_LEN_MAX) {
		if (ugd->ug_ListPlay.ug_Player == 0) {
			ret = __mf_ug_list_play_create_player(&ugd->ug_ListPlay.ug_Player);
			if (ret == false) {
				UG_TRACE_END;
				return false;
			}
			/*avsysaudiosink volume table setting */

			ret = __mf_ug_list_play_set_uri(ugd->ug_ListPlay.ug_Player, path);
			if (ret == false) {
				UG_TRACE_END;
				return false;
			}

			sound_type_e sound_type = __mf_ug_list_play_sound_type(ugd->ug_Status.ug_pEntryPath);
			ret = __mf_ug_list_play_set_sound_type(ugd);
			if (ret == false) {
				ug_error("set sound type failed");
				return ret;
			} else {
				ug_error("set sound type success");
			}

			/*player_set_session_prelistening(ugd->ug_ListPlay.ug_Player);*/

			UG_SAFE_FREE_CHAR(g_player_cbs);
			if (g_player_pipe) {
				ecore_pipe_del(g_player_pipe);
				g_player_pipe = NULL;
			}
			g_player_cbs = calloc(1, sizeof(mf_player_cbs));
			g_player_pipe = ecore_pipe_add(__mf_player_mgr_callback_pipe_handler, ugd);


			player_set_completed_cb(ugd->ug_ListPlay.ug_Player, __mf_ug_list_play_mgr_completed_cb, NULL);
//			player_set_interrupted_cb(ugd->ug_ListPlay.ug_Player, __mf_ug_list_play_mgr_interrupted_cb, NULL);
			player_set_error_cb(ugd->ug_ListPlay.ug_Player, __mf_ug_list_play_mgr_error_cb, NULL);
		} else {
			ug_debug("player handle is exist");

			ret = __mf_ug_list_play_set_uri(ugd->ug_ListPlay.ug_Player, path);
			if (ret == false) {
				UG_TRACE_END;
				return false;
			}
			UG_TRACE_END;
			return true;
		}
	} else {
		ug_debug("the path_len is too long");
		UG_TRACE_END;
		return false;
	}

	/*player_set_buffering_cb(ugd->ug_ListPlay.ug_Player, _mp_player_mgr_buffering_cb, NULL);*/
	return true;

}

/******************************
** Prototype    : _mp_player_mgr_set_msg_callback
** Description  :
** Input        : MMMessageCallback cb
**                gpointer user_data
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
static void __mf_ug_list_play_start(void *data)
{
	UG_TRACE_BEGIN;
	if (mf_ug_main_is_background()) {
		return;
	}
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");
	ugData *ugd = itemData->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	int error_code = -1;
	player_state_e state = PLAYER_STATE_NONE;

	error_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
	ug_debug("state is [%d]", state);
	if (error_code == 0 && state == PLAYER_STATE_PLAYING) {
		if (ugd->ug_ListPlay.ug_pPlayFilePath == NULL) {
			ugd->ug_ListPlay.ug_pPlayFilePath = g_strdup(itemData->ug_pItemName->str);
		}
		__mf_ug_list_play_set_play_start_status(ugd, itemData->ug_pItemName->str);
	} else if (ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PAUSED) {
		if (ugd->ug_ListPlay.ug_pPlayFilePath == NULL) {
			ugd->ug_ListPlay.ug_pPlayFilePath = g_strdup(itemData->ug_pItemName->str);
		}
		__mf_ug_list_play_set_play_resume_status(ugd);
	}

	UG_TRACE_END;
}

static void __mf_ug_list_play_pauset(void *data)
{
	UG_TRACE_BEGIN;

	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	__mf_ug_list_play_set_play_pause_status(ugd);
	UG_TRACE_END;
}

static void __mf_ug_list_play_complete_cb(void *data)
{
	UG_TRACE_BEGIN;

	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	mf_ug_list_play_destory_playing_file(ugd);
	mf_ug_list_disable_play_itc(ugd, true);
	ugd->ug_ListPlay.play_data = NULL;
	UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	UG_TRACE_END;
}

/*static void __mf_ug_list_play_interrupt_cb(player_interrupted_code_e code, void *data)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");

	ugData *ugd = (ugData *)itemData->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	switch (code) {
	case PLAYER_INTERRUPTED_BY_MEDIA:
		ug_debug("Interrupt :: PLAYER_INTERRUPTED_BY_MEDIA");
		break;
	case PLAYER_INTERRUPTED_BY_CALL:
		ug_debug("Interrupt :: PLAYER_INTERRUPTED_BY_CALL_START");
		break;
	case PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT:
		ug_debug("Interrupt :: PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT");
		break;
	case PLAYER_INTERRUPTED_BY_ALARM:
		ug_debug("Interrupt :: PLAYER_INTERRUPTED_BY_ALARM_START");
		break;
	case PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG:
		ug_debug("Interrupt :: PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG");
		break;
	case PLAYER_INTERRUPTED_COMPLETED:
		ug_debug("PLAYER_INTERRUPTED_COMPLETED");
		 ready to resume
		ug_debug("ugd->ug_ListPlay.ug_iPlayState is [%d]", ugd->ug_ListPlay.ug_iPlayState);
		if (ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PAUSED) {
			__mf_ug_list_play_control_cb(itemData);
		}
		return;
	default:
		break;
	}
	__mf_ug_list_play_set_play_pause_status(ugd);
	mf_ug_list_play_update_item_icon(ugd);
	UG_TRACE_END;

}*/

static void
__mf_list_play_control_prepare_cb(void *userdata)
{
	UG_TRACE_BEGIN;

	__mf_ug_list_play_play_current_file(userdata);
}


/******************************
** Prototype    : _mp_player_mgr_realize
** Description  :
** Input        : ugListItemData *data
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
static void
__mf_ug_list_play_prepare_cb(void *userdata)
{
	UG_TRACE_BEGIN;
	MF_CHECK(g_player_pipe);

	mf_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mf_player_cb_extra_data));
	extra_data.cb_type = MF_PLAYER_CB_TYPE_PREPARE;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mf_player_cb_extra_data));
}

static bool __mf_ug_list_play_realize_player_mgr(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;

	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	int error_code = 0;
	player_state_e state = PLAYER_STATE_NONE;

	if (ugd->ug_ListPlay.ug_Player != 0) {
		error_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
		ug_debug("state is [%d]", state);

		if (0 == error_code && PLAYER_STATE_IDLE == state) {
			ug_debug("player_prepare_async");
			if (player_prepare_async(ugd->ug_ListPlay.ug_Player, __mf_ug_list_play_prepare_cb, ugd) != PLAYER_ERROR_NONE) {
				ug_debug("Error when mp_player_mgr_realize\n");
				UG_TRACE_END;
				return FALSE;
			} else {
				return true;
			}
			/*ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_READY;*/

		} else {
			UG_TRACE_END;
			return false;
		}
	}
	UG_TRACE_END;

	return false;
}


/******************************
** Prototype    : __mf_ug_list_play_ready_new_file_play
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
/*void mf_player_mgr_set_started_cb(player_started_cb  callback, void *user_data)
{
	MF_CHECK(g_player_cbs);

	g_player_cbs->started_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_STARTED] = user_data;
}

void mf_player_mgr_set_paused_cb(player_paused_cb  callback, void *user_data)
{
	MF_CHECK(g_player_cbs);

	g_player_cbs->paused_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_PAUSED] = user_data;
}*/

void mf_player_mgr_set_completed_cb(player_completed_cb  callback, void *user_data)
{

	MF_CHECK(g_player_cbs);

	g_player_cbs->completed_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_COMPLETED] = user_data;
}

/*void mf_player_mgr_set_interrupted_cb(player_interrupted_cb  callback, void *user_data)
{

	MF_CHECK(g_player_cbs);

	g_player_cbs->interrupted_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_INTURRUPTED] = user_data;
}*/

void mf_player_mgr_set_error_cb(player_error_cb  callback, void *user_data)
{

	MF_CHECK(g_player_cbs);

	g_player_cbs->error_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_ERROR] = user_data;
}

void mf_player_mgr_set_buffering_cb(player_buffering_cb  callback, void *user_data)
{
	MF_CHECK(g_player_cbs);

	g_player_cbs->buffering_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_BUFFERING] = user_data;
}

void mf_player_mgr_set_prepare_cb(player_prepared_cb callback, void *user_data)
{
	MF_CHECK(g_player_cbs);

	g_player_cbs->prepare_cb = callback;
	g_player_cbs->user_data[MF_PLAYER_CB_TYPE_PREPARE] = user_data;
}

static Eina_Bool __mf_play_control_error(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	mf_ug_list_play_destory_playing_file(ugd);
	mf_ug_list_disable_play_itc(ugd, true);
	ugd->ug_ListPlay.play_data = NULL;
	UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	ugd->ug_ListPlay.playing_err_idler = NULL;
	return EINA_FALSE;
}

static void __mf_play_control_error_cb(int error_code, void *userdata)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = userdata;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");
	ugData *ugd = itemData->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	switch (error_code) {
	case PLAYER_ERROR_OUT_OF_MEMORY:
		ug_error("PLAYER_ERROR_OUT_OF_MEMORY");
		break;
	case PLAYER_ERROR_INVALID_PARAMETER:
		ug_error("PLAYER_ERROR_INVALID_PARAMETER");
		break;
	case PLAYER_ERROR_NOT_SUPPORTED_FILE:	/*can receive error msg while playing.*/
		ug_error("receive MM_ERROR_PLAYER_CODEC_NOT_FOUND\n");
		ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNSUPPORT_FILE_TYPE, NULL, NULL, NULL, NULL, NULL);
		break;
	case PLAYER_ERROR_CONNECTION_FAILED:
		ug_error("MM_ERROR_PLAYER_STREAMING_CONNECTION_FAIL");

		ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_CONNECT_FAILED, NULL, NULL, NULL, NULL, NULL);
		break;
	default:
		ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNSUPPORT_FILE_TYPE, NULL, NULL, NULL, NULL, NULL);
		ug_error("error_code: %d", error_code);
	}

	/*if (ugd->ug_MainWindow.ug_pRadioGroup && elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup) == itemData->ug_iGroupValue) {
		//ugd->ug_Status.ug_iRadioOn = 0;
		//itemData->ug_bChecked = false;
		//elm_radio_value_set(ugd->ug_MainWindow.ug_pRadioGroup, 0);
		//mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	}*/

	if (!ugd->ug_ListPlay.playing_err_idler) {
		ugd->ug_ListPlay.playing_err_idler = ecore_idler_add((Ecore_Task_Cb)__mf_play_control_error, ugd);
	}
}


static bool __mf_ug_list_play_ready_new_file_play(void *data)
{

	UG_TRACE_BEGIN;

	if (mf_ug_main_is_background()) {
		return false;
	}

	ugListItemData *itemData = data;
	ug_mf_retvm_if(itemData == NULL, false, "itemData is NULL");

	ugData *ugd = (ugData *)itemData->ug_pData;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	char *path = strdup(itemData->ug_pItemName->str);
	if (mf_ug_is_default_ringtone(ugd, path)) {
		UG_SAFE_FREE_CHAR(path);
		path = g_strdup(ugd->ug_UiGadget.default_ringtone);
	}

	/*check if file is exist */
	if (path != NULL) {
		if (!mf_file_exists(path)) {
			ug_debug("Error file %s is not exist\n", path);
			free(path);
			path = NULL;
			UG_TRACE_END;
			return false;
		}
		if (!__mf_ug_list_play_create_player_mgr(ugd, path)) {
			free(path);
			path = NULL;
			ug_error("ERROR HERE !!!!!!!");
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_ERROR_OCCURRED, NULL, NULL, NULL, NULL, NULL);
			UG_TRACE_END;
			return false;
		}

		/*mf_player_mgr_set_started_cb(__mf_ug_list_play_start_cb, itemData);
		mf_player_mgr_set_paused_cb(__mf_ug_list_play_pauset_cb, ugd);*/
		mf_player_mgr_set_completed_cb(__mf_ug_list_play_complete_cb, ugd);
//		mf_player_mgr_set_interrupted_cb(__mf_ug_list_play_interrupt_cb, itemData);
		mf_player_mgr_set_prepare_cb(__mf_list_play_control_prepare_cb, itemData);
		mf_player_mgr_set_error_cb(__mf_play_control_error_cb, itemData);
		/*mf_player_mgr_set_buffering_cb(_mp_play_control_buffering_cb, ad);*/

		if (!__mf_ug_list_play_realize_player_mgr(ugd)) {
			free(path);
			path = NULL;
			ug_error("ERROR HERE !!!!!!!");
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_ERROR_OCCURRED, NULL, NULL, NULL, NULL, NULL);
			UG_TRACE_END;
			return false;
		}
		free(path);
		path = NULL;
		UG_TRACE_END;
		return true;
	} else {
		return false;
	}

}

/******************************
** Prototype    : _mp_player_mgr_play
** Description  :
** Input        : ugListItemData *data
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
static bool __mf_ug_list_play_play(void *data)
{

	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retvm_if(itemData == NULL, false, "itemData is NULL");
	ugData *ugd = itemData->ug_pData;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");
	if (mf_ug_main_is_background()) {
		return false;
	}

	int err = 0;
	int error = 0;
	int error_code = 0;
	player_state_e state = PLAYER_STATE_NONE;

	error = sound_manager_acquire_focus(ugd->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
	if (error != SOUND_MANAGER_ERROR_NONE) {
		ug_error("failed to acquire focus error[%x]", error);
		return false;
	}

	if (ugd->ug_ListPlay.ug_Player != 0) {
		error_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
		ug_debug("state is [%d]", state);

		if (0 == error_code && PLAYER_STATE_READY == state) {
			err = player_start(ugd->ug_ListPlay.ug_Player);
			if (err != PLAYER_ERROR_NONE) {

				if (err == PLAYER_ERROR_SOUND_POLICY) {
					ug_error("PLAYER_ERROR_SOUND_POLICY error");
					ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_DURING_CALL, NULL,
					                                     NULL, NULL, NULL, NULL);
				} else {
					ug_error("error is [%d]", err);
					ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_ERROR_OCCURRED, NULL,
					                                     NULL, NULL, NULL, NULL);
				}
				ug_error("Error when _mp_player_mgr_play. err[%x]\n", err);
				UG_TRACE_END;
				return false;
			} else {
				__mf_ug_list_play_start(itemData);
				mf_ug_list_disable_play_itc(ugd, false);
				UG_TRACE_END;
				return true;
			}
		} else {
			UG_TRACE_END;
			return false;
		}
	} else {
		UG_TRACE_END;
		return false;
	}
}

/******************************
** Prototype    : __mf_ug_list_play_stop
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
static bool __mf_ug_list_play_stop(ugData *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	player_state_e state = PLAYER_STATE_NONE;
	int error_code = 0;
	int error = 0;

	if (ugd->ug_ListPlay.ug_Player != 0) {
		error_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
		ug_debug("state is [%d]", state);

		if (0 == error_code && (PLAYER_STATE_PLAYING == state || PLAYER_STATE_PAUSED == state)) {
			if (player_stop(ugd->ug_ListPlay.ug_Player) != 0) {
				ug_debug("Error when __mf_ug_list_play_stop\n");
				UG_TRACE_END;
				return false;
			} else {

				ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_STOP;
				error = sound_manager_release_focus(ugd->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
				if (error != SOUND_MANAGER_ERROR_NONE) {
					ug_error("failed to release focus error[%x]", error);
					return false;
				}

				UG_TRACE_END;
				return true;
			}

		} else {
			UG_TRACE_END;
			return false;
		}
	}
	UG_TRACE_END;
	return false;
}

/******************************
** Prototype    : __mf_ug_list_play_unrealize
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
static bool __mf_ug_list_play_unrealize(ugData *data)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	if (ugd->ug_ListPlay.ug_Player != 0) {
		/*/unrealize can be invoked at any state */
		int ret = player_unprepare(ugd->ug_ListPlay.ug_Player);
		if (ret != 0) {
			ug_error("Error when __mf_ug_list_play_unrealize %d", ret);
			UG_TRACE_END;
			return false;
		} else {
			UG_TRACE_END;
			return true;
		}
	}
	UG_TRACE_END;
	return false;
}

/******************************
** Prototype    : __mf_ug_list_play_destory
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
static bool __mf_ug_list_play_destory(ugData *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	if (ugd->ug_ListPlay.ug_Player != 0) {
		/*/destroy can be invoked at any state */
		if (player_destroy(ugd->ug_ListPlay.ug_Player) != 0) {
			ug_debug("Error when __mf_ug_list_play_destory\n");
			UG_TRACE_END;
			return false;
		} else {
			ugd->ug_ListPlay.ug_Player = 0;
			ugd->ug_ListPlay.ug_iPlayState = PLAY_STATE_INIT;
			UG_TRACE_END;
			return true;
		}
	}

	UG_TRACE_END;
	return false;
}

static void __mf_ug_list_play_pipe_destory()
{
	UG_SAFE_FREE_CHAR(g_player_cbs);
	if (g_player_pipe) {
		ecore_pipe_del(g_player_pipe);
		g_player_pipe = NULL;
	}

}
/******************************
** Prototype    : __mf_ug_list_play_play_current_file
** Description  : Samsung
** Input        : ugData *data
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
static bool __mf_ug_list_play_play_current_file(void *data)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retvm_if(itemData == NULL, false, "itemData is NULL");
	ugData *ugd = itemData->ug_pData;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");
	int err_code = 0;
	player_state_e state = PLAYER_STATE_NONE;

	err_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
	ug_debug("state : [%d] , error code : [%d]", state, err_code);

	if (state != PLAYER_STATE_READY) {
		UG_TRACE_END;
		return false;
	}
	if (!__mf_ug_list_play_play(itemData)) {
		mf_ug_list_play_destory_playing_file(ugd);
		mf_ug_list_disable_play_itc(ugd, true);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
		UG_TRACE_END;
		return false;
	}

	return true;
}


/******************************
** Prototype    : __mf_ug_list_play_resume
** Description  :
** Input        : ugListItemData *data
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
static bool __mf_ug_list_play_resume(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	player_state_e state = PLAYER_STATE_NONE;
	int error_code = 0;
	int err = -1;

	if (ugd->ug_ListPlay.ug_Player != 0) {
		error_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
		ug_debug("state is [%d]", state);

		if (0 == error_code && PLAYER_STATE_PAUSED == state) {
			err = player_start(ugd->ug_ListPlay.ug_Player);

			if (err != PLAYER_ERROR_NONE) {

				if (err == PLAYER_ERROR_SOUND_POLICY) {
					ug_error("ERROR HERE !!!!!!!");
					ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_DURING_CALL, NULL,
					                                     NULL, NULL, NULL, NULL);
				} else {
					ug_error("ERROR HERE !!!!!!!");
					ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_ERROR_OCCURRED, NULL,
					                                     NULL, NULL, NULL, NULL);
				}
				ug_error("Error when _mp_player_mgr_play. err[%x]\n", err);
				UG_TRACE_END;
				return false;
			} else {
				__mf_ug_list_play_set_play_start_status(ugd, ugd->ug_ListPlay.ug_pPlayFilePath);
				mf_ug_list_disable_play_itc(ugd, false);
				UG_TRACE_END;
				return true;
			}
		} else {
			UG_TRACE_END;
			return false;
		}
	}
	UG_TRACE_END;
	return false;

}


/******************************
** Prototype    : __mf_ug_list_play_pause
** Description  :
** Input        : ugListItemData *data
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
static bool __mf_ug_list_play_pause(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	int err = 0;
	player_state_e state = PLAYER_STATE_NONE;
	int error_code = 0;

	if (ugd->ug_ListPlay.ug_Player) {
		error_code = player_get_state(ugd->ug_ListPlay.ug_Player, &state);
		ug_debug("state is [%d]", state);

		if (0 == error_code && PLAYER_STATE_PLAYING == state) {
			err = player_pause(ugd->ug_ListPlay.ug_Player);
			if (err != 0) {
				ug_debug("Error when _ug_player_mgr_pause. err[%x]\n", err);
				UG_TRACE_END;
				return false;
			} else {
				UG_TRACE_END;
				return true;
			}
		} else {
			UG_TRACE_END;
			return false;
		}
	}
	UG_TRACE_END;
	return false;
}


/******************************
** Prototype    : mp_play_control_cb
** Description  :
** Input        : ugListItemData *data
**                int  state
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
bool mf_ug_list_play_pause(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");
	int state = ugd->ug_ListPlay.ug_iPlayState;

	if (state == PLAY_STATE_PLAYING) {
		if (__mf_ug_list_play_pause(ugd)) {
			__mf_ug_list_play_pauset(ugd);
			mf_ug_list_disable_play_itc(ugd, false);
			return true;
		}
		return false;
	}
	return false;
}
static void __mf_ug_list_play_control_cb(void *data)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");

	ugData *ugd = (ugData *)itemData->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	int state = ugd->ug_ListPlay.ug_iPlayState;

	if (state == PLAY_STATE_PLAYING) {
		if (__mf_ug_list_play_pause(ugd)) {
			__mf_ug_list_play_pauset(ugd);
			mf_ug_list_disable_play_itc(ugd, false);
		}
	} else {
		if (__mf_ug_list_play_resume(ugd)) {
			__mf_ug_list_play_start(itemData);
		}

	}
	UG_TRACE_END;
}

static bool __mf_ug_list_play_play_new_file(ugListItemData *data)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retvm_if(itemData == NULL, false, "itemData is NULL");

	ugData *ugd = (ugData *)itemData->ug_pData;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	__mf_ug_list_play_init_data(ugd);

	if (mf_ug_main_is_background()) {
		return false;
	}
	if (!__mf_ug_list_play_ready_new_file_play(itemData)) {
		UG_TRACE_END;
		return false;
	}

	UG_TRACE_END;

	return true;
}


/******************************
** Prototype    : _music_item_play
** Description  :
** Input        : ugListItemData *param
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
void mf_ug_list_play_reset_playing_file(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	__mf_ug_list_play_unrealize(ugd);
}

void mf_ug_list_play_play_music_item(ugListItemData *data)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");
	ugData *ugd = itemData->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ug_debug("ugd->ug_ListPlay.ug_iPlayState is [%d]", ugd->ug_ListPlay.ug_iPlayState);

	if (ugd->ug_ListPlay.ug_pPlayFilePath != NULL) {
		ug_debug();
		if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, itemData->ug_pItemName->str) != 0) {
			/*mf_ug_list_play_destory_playing_file(ugd);*/
			mf_ug_list_play_reset_playing_file(ugd);
			mf_ug_list_disable_play_itc(ugd, true);
			if (!__mf_ug_list_play_play_new_file(itemData)) {
				mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_ERROR_OCCURRED,
				                   NULL, NULL, NULL, NULL, NULL);
			}
		} else {
			/*/ playing the same file */
			__mf_ug_list_play_control_cb(itemData);
		}
	} else {
		mf_ug_list_disable_play_itc(ugd, false);
		if (!__mf_ug_list_play_play_new_file(itemData)) {
			mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_UNABLE_TO_PLAY_ERROR_OCCURRED,
			                   NULL, NULL, NULL, NULL, NULL);
		}
	}

	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_list_play_destory_playing_file
** Description  :
** Input        : ugListItemData *data
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
void mf_ug_list_play_destory_playing_file(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	__mf_ug_list_play_pipe_destory();
	__mf_ug_list_play_stop(ugd);
	__mf_ug_list_play_unrealize(ugd);
	__mf_ug_list_play_destory(ugd);
	UG_TRACE_END;
}

static sound_type_e mf_ug_player_get_sound_type()
{
	UG_TRACE_BEGIN;
	sound_type_e type = SOUND_TYPE_SYSTEM;
	int ret = 0;
	ret = sound_manager_get_current_sound_type(&type);
	ug_debug("ret is [%d]", ret);
	UG_TRACE_END;
	return type;
}

static int mf_ug_player_get_volume(sound_type_e type)
{
	UG_TRACE_BEGIN;
	int volume = 0;
	int ret = 0;
	ret = sound_manager_get_volume(SOUND_TYPE_MEDIA, &volume);
	/*ret = sound_manager_get_volume(type, &volume);*/
	ug_debug("ret is [%d]", ret);
	UG_TRACE_END;
	return volume;

}

static void mf_ug_player_vol_type_set(mf_player_volume_type type)
{
	UG_TRACE_BEGIN;
	sound_type_e current_type;
	int volume = 0;
	current_type = mf_ug_player_get_sound_type();
	volume = mf_ug_player_get_volume(current_type);

	/*Fix the P130902-01617, refer to the android galaxy S4.*/
	if (g_init_volume == -1) {
		g_init_current_type = current_type;
		g_init_volume = volume;
	}

	ug_debug("current type is [%d] volume is [%d] type is [%d]", current_type, volume, type);

	switch (type) {
	case	MF_VOLUME_ALERT:
		/*sound_manager_set_volume_key_type(VOLUME_KEY_TYPE_ALARM);*/
		break;
	case	MF_VOLUME_NOTIFICATION:
		/*sound_manager_set_volume_key_type(VOLUME_KEY_TYPE_NOTIFICATION);*/
		break;
	case	MF_VOLUME_RINGTONE:
		/*sound_manager_set_volume_key_type(VOLUME_KEY_TYPE_RINGTONE);*/
		break;
	default:
		/*sound_manager_set_volume_key_type(type);*/
		break;
	}
	UG_TRACE_END;
}

void mf_ug_player_vol_reset_default_value(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_none) {
		return;
	}

	sound_type_e current_type;
	current_type = mf_ug_player_get_sound_type();
	if (g_init_current_type != current_type) {
		mf_ug_player_vol_type_set(g_init_current_type);
	}
}

void mf_ug_player_vol_set(void *data, const char *path)
{
	ug_mf_retm_if(path == NULL, "path is NULL");
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_none) {
		return;
	}

	if (g_strcmp0(path, UG_SETTING_MSG_ALERTS_PATH) == 0) {
		mf_ug_player_vol_type_set(MF_VOLUME_NOTIFICATION);
	} else if (g_strcmp0(path, UG_SETTING_RINGTONE_PATH) == 0) {
		mf_ug_player_vol_type_set(MF_VOLUME_RINGTONE);
	} else if (g_strcmp0(path, UG_SETTING_ALERTS_PATH) == 0 || g_strcmp0(path, UG_SETTING_SMART_ALRAMS) == 0) {
		mf_ug_player_vol_type_set(MF_VOLUME_ALERT);
	} else {
		mf_ug_player_vol_type_set(MF_VOLUME_NONE);
	}
}

bool mf_ug_is_default_ringtone(void *data, const char *path)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");
	if (ugd->ug_UiGadget.default_ringtone) {
		if (g_strcmp0(MF_UG_LABEL_DEFAULT_RINGTONE, path) == 0) {
			return true;
		}
	}

	return false;
}

bool mf_ug_is_silent(void *data, const char *path)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");
	if (ugd->ug_UiGadget.default_ringtone) {
		if (g_strcmp0(MF_UG_LABEL_SILENT, path) == 0) {
			return true;
		}
	}

	return false;
}

void mf_ug_cb_earjack_changed_cb(runtime_info_key_e key, void *data)
{
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	int earjack = 0;
	int retcode = -1;

	retcode = runtime_info_get_value_int(RUNTIME_INFO_KEY_AUDIO_JACK_STATUS, &earjack);
	if (RUNTIME_INFO_ERROR_NONE != retcode) {
		ug_error("runtime_info_get_init failed.");
		earjack = RUNTIME_INFO_AUDIO_JACK_STATUS_UNCONNECTED;
	}
	if (earjack > RUNTIME_INFO_AUDIO_JACK_STATUS_UNCONNECTED) {
		if (ugd->ug_ListPlay.ug_pPlayFilePath && ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PLAYING) {
			__mf_ug_list_play_pauset(ugd);
			mf_ug_list_disable_play_itc(ugd, false);

		}
	}
}

int mf_ug_list_play_earjack_monitor(void *data)
{
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, UG_ERROR_RETURN, "ugd is NULL");

	return runtime_info_set_changed_cb(RUNTIME_INFO_KEY_AUDIO_JACK_STATUS, mf_ug_cb_earjack_changed_cb, ugd);
}

void mf_ug_destory_earjack_monitor(void)
{
	UG_TRACE_BEGIN;
	int retcode = -1;

	retcode = runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_AUDIO_JACK_STATUS);
	if (retcode != RUNTIME_INFO_ERROR_NONE) {
		ug_error("runtime_info_unset failed.");
	}
	UG_TRACE_END;
}

