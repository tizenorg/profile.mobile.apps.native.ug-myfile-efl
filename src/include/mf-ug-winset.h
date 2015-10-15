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

#ifndef __DEF_MF_UG_WINSET_H_
#define __DEF_MF_UG_WINSET_H_

#include <stdio.h>
#include <assert.h>
#include <Elementary.h>
#include <glib.h>

#include "mf-ug-main.h"
#include "mf-ug-conf.h"
#include "mf-ug-fs-util.h"


/******************Navigation Bar Definition ***********/
#define NAVI_MORE_BUTTON_PART		"toolbar_more_btn"
#define NAVI_BOTTOM_BUTTON_1_PART	"toolbar_button1"
#define NAVI_BOTTOM_BUTTON_2_PART	"toolbar_button2"
#define NAVI_BUTTON_STYLE		"naviframe/toolbar/default"
#define NAVI_BUTTON_EDIT		"naviframe/more/default"

#define TITLE_BTN_STYLE	"elm/button/base/naviframe/title_icon"
#define TITLE_LEFT_BTN		"title_left_btn"
#define TITLE_RIGHT_BTN		"title_right_btn"

#define MF_UG_NAVI_STYLE_ENABLE "basic"
#define	MF_UG_UPPER_HEIGHT	(113*elm_config_scale_get())
#define MF_UG_UPPER_WIDTH	(480*elm_config_scale_get())

Evas_Object *mf_ug_navi_bar_create_navi_bar(Evas_Object *parent);
void mf_ug_navi_bar_set_new_content(Evas_Object *pLayout, Evas_Object *NaviContent);
void mf_ug_navi_bar_create_default_view(void *data);

void mf_ug_navi_bar_remove_previous_contents(void *data);
void mf_ug_navi_bar_title_set(void *data);
void mf_ug_navi_bar_create_group_radio_box(void *data);
void mf_ug_navi_bar_button_set_disable(void *data, bool disable);
void mf_ug_navi_bar_set_path_state(Evas_Object *layout, Eina_Bool flag);
Evas_Object *mf_ug_navi_bar_create_layout(Evas_Object *parent, const char *edj, const char *grp_name);
Evas_Object *mf_genlist_create_path_info(Evas_Object *parent, const char *info, Evas_Object **pathinfo);
Evas_Object *mf_ug_navi_bar_create_box(Evas_Object * parent);
void mf_ug_navi_add_back_button(void *data);
void mf_ug_navi_bar_create_delete_view(void *data);
int mf_ug_navibar_get_ringtone_count(int mode);


/********** Control Bar Definition ********/

#define UG_TITLE_ICON_HOME	            			"myfile_controlbar_cion_home.png"
#define UG_TITLE_ICON_HOME_PRESS	            		"myfile_controlbar_cion_home_press.png"
#define UG_TITLE_ICON_UPPER	            			"myfile_controlbar_cion_up_folder.png"
#define UG_TITLE_ICON_UPPER_PRESS	            		"myfile_controlbar_cion_up_folder_press.png"
#define UG_TITLE_ICON_SELECT_ALL					"myfile_icon_select_all.png"
#define UG_TITLE_ICON_SELECT_ALL_PRESS				"myfile_icon_select_all_selected.png"

void mf_ug_navi_bar_set_ctrl_item_disable(void *data);

/***********	Popup Definition	************/
typedef enum _mf_ug_popup_mode mf_ug_popup_mode;
enum _mf_ug_popup_mode {
	UG_POPMODE_MIN = 0,
	UG_POPMODE_TEXT,
	UG_POPMODE_TITLE_TEXT,
	UG_POPMODE_TEXT_TWO_BTN,
	UG_POPMODE_TITLE_TEXT_TWO_BTN,
	UG_POPMODE_TEXT_BTN,
	UG_POPMODE_TITLE_TEXT_BTN,
	UG_POPMODE_TITLE_TEXT_THREE_BTN,
	UG_POPMODE_PROGRESSBAR,
	UG_POPMODE_SEARCH,
	UG_POPMODE_LIST_BY,
	UG_POPMPDE_MAX
};

typedef enum _mf_ug_popup_event_type mf_ug_popup_event_type;
enum _mf_ug_popup_event_type {
	UG_ELM_POPUP_NONE,
	UG_ELM_POPUP_YES,
	UG_ELM_POPUP_NO,
	UG_ELM_POPUP_OK,
	UG_ELM_POPUP_CANCLE,
	UG_ELM_POPUP_AUTO_RENAME,
	UG_ELM_POPUP_REPLACE,
	UG_ELM_POPUP_PHONE,
	UG_ELM_POPUP_MEMORY,
	UG_ELM_POPUP_MAX
};

/***********	Popup API		************/
Evas_Object *mf_ug_popup_create(void *data, mf_ug_popup_mode popupMode, char *title, const char *context, const char *first_btn_text, const char *second_btn_text,
			const char *third_btn_text, Evas_Smart_Cb func, void *param);
void mf_ug_popup_indicator_popup(char *text);
Evas_Object *mf_ug_popup_create_new_folder_popup(void *data, char *context);
Evas_Object *mf_ug_popup_create_search(void *data, Evas_Smart_Cb func, void *param);
Evas_Object *mf_ug_progress_get();
void mf_ug_progress_set(Evas_Object *progress);
Eina_Bool mf_ug_popup_present_flag_get();

void mf_ug_context_popup_create_more(void *data, Evas_Object *parent);
void mf_ug_resize_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);


/******************* Genlist API      *********************/

Evas_Object *mf_ug_genlist_create_content_list_view(void *data);
Evas_Object *__mf_ug_genlist_create_gl(void *data);
Evas_Object *mf_ug_genlist_create_path_info(Evas_Object *parent, Elm_Genlist_Item_Class *itc, char *info);
Elm_Object_Item *mf_ug_genlist_item_append(Evas_Object *parent,
				      char *real_name,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc);
void mf_ug_genlist_selected_gl(void *data, Evas_Object *obj, void *event_info);
void mf_ug_genlist_item_remove(Evas_Object *parent, int storage);
Evas_Object *mf_ug_genlist_create_checkbox(Evas_Object *parent);

void mf_ug_navi_bar_push_content(void *data, Evas_Object *NaviContent);
Evas_Object *mf_popup_center_processing(Evas_Object *parent,
				   const char *context,
				   Evas_Smart_Cb func,
				   void *param);
void mf_ug_genlist_first_item_append(void *data, char *fullpath);
void mf_ug_select_all_layout_mouse_down_cb(void *data, Evas_Object *obj, void *event_info);
void mf_ug_genlist_item_bringin_top(void *data, const char *music_path);
Evas_Object *mf_ug_genlist_delete_style_create(void *data);
int mf_ug_genlist_ringtone_delete_items_add(void *data, int value);
void mf_ug_genlist_first_item_insert(void *data, char *fullpath, Elm_Object_Item *insert_afer);
Elm_Object_Item *mf_ug_genlist_default_item_get();
void mf_ug_genlist_show_select_info(void *data);

#endif
