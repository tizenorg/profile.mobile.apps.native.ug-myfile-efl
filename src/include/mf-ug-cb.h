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


#ifndef __DEF_MF_UG_CB_H_
#define __DEF_MF_UG_CB_H_

#include <Elementary.h>
#include <device/power.h>
#include <system_settings.h>
#include <storage.h>
#include "mf-ug-main.h"
#include "mf-ug-util.h"
#include "mf-ug-inotify-handle.h"

Eina_Bool mf_ug_cb_back_button_cb(void *data, Elm_Object_Item *it);
void mf_ug_cb_add_button_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_cb_mass_storage_popup_cb(void *data);
void mf_ug_cb_tab_bar_cb(void *data, char *path);
void mf_ug_cb_list_play_cb(ugListItemData *data, Evas_Object *obj, void *event_info);
void mf_ug_cb_select_info_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_ug_cb_select_info_hide_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_ug_cb_select_info_timeout_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_cb_mmc_changed_cb(int storage_id, storage_state_e state, void *user_data);
void mf_ug_cb_dir_update_cb(mf_ug_inotify_event event, char *name, void *data);
void mf_ug_cb_dir_pipe_cb(void *data, void *buffer, unsigned int nbyte);
int mf_ug_cb_set_mass_storage_state_cb(void *data);
void mf_ug_cb_upper_button_pressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_upper_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_upper_click_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_home_button_pressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_home_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_home_button_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_search_view_enter_search_routine(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_warning_popup_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_reach_max_len_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
void mf_ug_cb_save_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_cb_cancel_new_folder_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_create_new_folder(void *data, Evas_Object * obj, void *event_info);
//void mf_ug_cb_lcd_state_changed_cb(power_state_e state, void *user_data);
Eina_Bool mf_ug_cb_popup_del_idler_cb(void *data);
void mf_ug_cb_entry_button_pressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_entry_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_play_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_play_button_pressed_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_more_cb(void *data, Evas_Object * obj, void *event_info);
void mf_ug_cb_default_ringtone_changed_cb(system_settings_key_e key, void *data);
void mf_ug_cb_cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
bool mf_ug_cb_create_thumbnail(void *data, media_thumbnail_completed_cb callback);
void mf_ug_cb_thumb_created_cb(media_content_error_e error, const char *path, void *user_data);
void mf_ug_cb_delete_button_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_ringtone_del_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_cb_delete_button_popup_create(void *data, Evas_Object *obj, void *event_info);
void mf_ug_cb_delete_button_confirm_cb(void *data, Evas_Object *obj, void *event_info);
void __mf_ug_popup_show_vk(void *data, Evas_Object *obj, void *event_info);
void mf_ug_select_all_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_item_sel_all_press_cb(void *data, Evas_Object *obj, void *event_info);
#endif
