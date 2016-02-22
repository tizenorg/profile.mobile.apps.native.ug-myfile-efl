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


#ifndef __MF_UG_LIST_PLAY_H
#define __MF_UG_LIST_PLAY_H

#include <player.h>
#include <sound_manager.h>

typedef enum {
	MF_PLAYER_CB_TYPE_STARTED,
	MF_PLAYER_CB_TYPE_PAUSED,
	MF_PLAYER_CB_TYPE_COMPLETED,
	MF_PLAYER_CB_TYPE_INTURRUPTED,
	MF_PLAYER_CB_TYPE_ERROR,
	MF_PLAYER_CB_TYPE_BUFFERING,
	MF_PLAYER_CB_TYPE_PREPARE,
	MF_PLAYER_CB_TYPE_NUM,
} mf_player_cb_type;

typedef enum {
	MF_VOLUME_NONE,
	MF_VOLUME_ALERT,
	MF_VOLUME_NOTIFICATION,
	MF_VOLUME_RINGTONE,
	MF_VOLUME_NUM
} mf_player_volume_type;


typedef struct __mf_player_cbs mf_player_cbs;
struct __mf_player_cbs{
	/* player callbacks */
	/*note: start callback and paused callback for player have been removed*/
	/*player_started_cb started_cb;*/
	/*player_paused_cb paused_cb;*/
	player_completed_cb completed_cb;
	player_interrupted_cb interrupted_cb;
	player_error_cb error_cb;
	player_buffering_cb buffering_cb;
	player_prepared_cb prepare_cb;

	/* callback user data */
	void *user_data[MF_PLAYER_CB_TYPE_NUM];
};


typedef struct {
	mf_player_cb_type cb_type;

	union {
		player_interrupted_code_e interrupted_code;
		int error_code;
		int percent;
	} param;
} mf_player_cb_extra_data;

void mf_ug_player_vol_set(void* data, const char *path);
void mf_ug_player_vol_reset_default_value(void* data);

bool mf_ug_list_play_pause(void *data);
void mf_ug_list_disable_play_itc(void *data, bool disable);
bool mf_ug_is_default_ringtone(void *data, const char *path);
int mf_ug_list_play_earjack_monitor(void *data);
void mf_ug_destory_earjack_monitor();
bool mf_ug_is_silent(void *data, const char *path);
void mf_player_focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data);

#endif
