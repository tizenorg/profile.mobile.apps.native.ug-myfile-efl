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



#include "mf-ug-main.h"
#include "mf-ug-widget.h"
#include "mf-ug-winset.h"
#include "mf-ug-resource.h"
#include "mf-ug-dlog.h"
#include "mf-ug-util.h"
#include "mf-ug-ringtone-view.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-db-handle.h"
#include "mf-ug-list-play.h"
#include "mf-ug-file-util.h"

static Ecore_Idler *ug_ringtone_idler = NULL;
extern bool g_ug_bDefaultItem;

Evas_Object *mf_ug_ringtone_list_create(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");
	Evas_Object *genlist = NULL;
	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;
	int groupValue = 1;
	g_ug_bDefaultItem = false;

	genlist = __mf_ug_genlist_create_gl(ugd);
	ug_error("=========================================== block count set");
	elm_genlist_block_count_set(genlist, 3);

	mf_ug_fm_svc_wapper_get_file_list_by_filter(ugd, ugd->ug_Status.ug_pPath, &dir_list, &file_list);
	ugd->ug_UiGadget.ug_pFilterList = file_list;
	mf_ug_util_sort_the_file_list(ugd);

	ugd->ug_MainWindow.ug_pNaviGenlist = genlist;
	ugFsNodeInfo *pNode = NULL;
	char *real_name = NULL;
	Eina_List *l = NULL;

	if (ugd->ug_UiGadget.default_ringtone) {
		mf_ug_genlist_default_ringtone_item_append(genlist, ugd, groupValue, &ugd->ug_Status.ug_1text2icon4_itc);
		groupValue++;
	}

	if (ugd->ug_UiGadget.silent) {
		mf_ug_genlist_silent_item_append(genlist, ugd, groupValue, &ugd->ug_Status.ug_1text2icon4_itc);
		groupValue++;
	}

	groupValue = mf_ug_genlist_ringtone_items_add(ugd, groupValue);


	bool exist_flag = true;
	/*To fix P131210-01329 wangyan, check whether the passed file exists*/
	if (ugd->ug_Status.mark_mode && mf_file_exists(ugd->ug_Status.mark_mode)) {
		if (g_strcmp0(ugd->ug_Status.mark_mode, DEFAULT_RINGTONE_MARK) && g_strcmp0(ugd->ug_Status.mark_mode, SILENT)) {

			if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
				exist_flag = mf_ug_db_handle_find_ringtone(ugd->ug_Status.mark_mode);
			} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
				exist_flag = mf_ug_db_handle_find_alert(ugd->ug_Status.mark_mode);
			}
			if (!exist_flag) {
				if (mf_ug_util_find_item_from_pnode_list(ugd->ug_UiGadget.ug_pFilterList, ugd->ug_Status.mark_mode) != false) {
					exist_flag = true;
				} else {
					exist_flag = false;
				}
			}

			if (!exist_flag) {
				mf_ug_genlist_item_append(genlist, ugd->ug_Status.mark_mode, ugd, groupValue, &ugd->ug_Status.ug_1text2icon4_itc);
				groupValue++;
				/*To fix P131128-01215 by wangyan,phone>menu>call settings>Ringtone and keypad tones>Ringtones
				>menu>delete setpath-> (not set but)back-> go to ringtone again ->delete menu is gray.
				the ringtone or alert is not in db ,add it */
				int location = mf_ug_fm_svc_wapper_get_location(ugd->ug_Status.mark_mode);
				if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
					mf_ug_db_handle_add_ringtone(ugd->ug_Status.mark_mode, NULL, location);
				} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
					mf_ug_db_handle_add_alert(ugd->ug_Status.mark_mode, NULL, location);
				}
			}
		}
	}
	g_ug_bDefaultItem = true;
	EINA_LIST_FOREACH(ugd->ug_UiGadget.ug_pFilterList, l, pNode) {
		if (pNode) {
			if (pNode->path && pNode->name) {
				real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
			}
		} else {
			continue;
		}
		mf_ug_genlist_item_append(genlist, real_name, ugd, groupValue, &ugd->ug_Status.ug_1text2icon4_itc);
		groupValue++;

		UG_SAFE_FREE_CHAR(real_name);
	}
	g_ug_bDefaultItem = false;
	return genlist;
}


static Eina_Bool __selected_item_show(void *data)
{
	ugData *ugd = (ugData *)data;

	Elm_Object_Item *defaultitem = NULL;

	if (!ugd->ug_Status.ug_bNoContentFlag && (ugd->ug_Status.ug_iRadioOn > 1)) {
		defaultitem  = elm_genlist_nth_item_get(ugd->ug_MainWindow.ug_pNaviGenlist, ugd->ug_Status.ug_iRadioOn - 1);
		if (NULL != defaultitem) {
			ug_error("ugd->ug_Status.ug_iRadioOn = %d", ugd->ug_Status.ug_iRadioOn);
			elm_genlist_item_show(defaultitem, ELM_GENLIST_ITEM_SCROLLTO_TOP);
		}
	}
	/*if (mf_ug_main_is_help_mode()) {
		ug_error("=============== create help===================");
		mf_ug_genlist_show_help_on_item(ugd);
	}*/

	ugd->show = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool mf_ug_ringtone_view_idler(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;

	Evas_Object *newContent = NULL;

	newContent = mf_ug_ringtone_list_create(ugd);
	if (!ugd->show) {
		ugd->show = ecore_idler_add(__selected_item_show, ugd);
	}
	ugd->ug_MainWindow.ug_pNaviGenlist = newContent;
	UG_SAFE_FREE_CHAR(ugd->ug_MainWindow.ug_pNaviTitle);
	/*set title segment or title */

	/*navigation view integration */
	evas_object_show(newContent);
	elm_object_part_content_set(ugd->ug_MainWindow.ug_pNaviLayout, "part1", newContent);
	ug_ringtone_idler = NULL;
	/*mf_ug_navi_bar_set_ctrl_item_disable(ugd);*/
	mf_ug_main_update_ctrl_in_idle(ugd);
	return ECORE_CALLBACK_CANCEL;

}
void mf_ug_create_rintone_view(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Evas_Object *pNaviLayout = NULL;
	/*Evas_Object *newContent = NULL;
	Evas_Object *top_layout = NULL;*/

	pNaviLayout = mf_ug_navi_bar_create_layout(ugd->ug_MainWindow.ug_pNaviBar, UG_EDJ_NAVIGATIONBAR, UG_GRP_NAVI_VIEW);
	ugd->ug_MainWindow.ug_pNaviLayout = pNaviLayout;
	mf_ug_navi_bar_set_path_state(pNaviLayout, EINA_FALSE);
	/*set content */
	mf_ug_navi_bar_create_group_radio_box(ugd);

	if (ugd->ug_UiGadget.title) {
		ugd->ug_MainWindow.ug_pNaviTitle = g_strdup(ugd->ug_UiGadget.title);
	}
	mf_ug_navi_bar_push_content(ugd, pNaviLayout);

	mf_ug_navi_bar_set_ctrl_button(ugd);

	mf_ug_navi_bar_title_set(ugd);
	mf_ug_navi_bar_remove_previous_contents(ugd);
	/*ug_ecore_idler_del(ug_ringtone_idler);
	ug_ringtone_idler = ecore_idler_add(mf_ug_ringtone_view_idler, ugd);*/
	mf_ug_ringtone_view_idler(ugd);
}

bool mf_ug_ringtone_is_default(int mode, const char *path)
{
	bool default_flag = false;
	if (mode == mf_ug_sound_mode_ringtone) {
		ug_error();
		default_flag = mf_ug_db_handle_find_ringtone(path);
	} else if (mode == mf_ug_sound_mode_alert) {
		ug_error();
		default_flag = mf_ug_db_handle_find_alert(path);
	}
	ug_error("default_flag is [%d]", default_flag);
	return !default_flag;
}

void mf_ug_ringtone_list_resume(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ug_error();
	Evas_Object *genlist = ugd->ug_MainWindow.ug_pNaviGenlist;
	Elm_Object_Item *it;
	it = elm_genlist_first_item_get(genlist);

	int index = elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup);
	int delete_index = -1;
	while (it) {
		ugListItemData *ug_ItemData = elm_object_item_data_get(it);
		if (ug_ItemData && ug_ItemData->ug_pItemName && ug_ItemData->ug_pItemName->str) {
			if (mf_ug_is_default_ringtone(ugd, ug_ItemData->ug_pItemName->str)) {
				it = elm_genlist_item_next_get(it);
				continue;
			}
			if (!mf_file_exists(ug_ItemData->ug_pItemName->str)) {
				ug_error();
				delete_index = ug_ItemData->ug_iGroupValue;
				Elm_Object_Item *temp = it;
				it = elm_genlist_item_next_get(it);
				elm_object_item_del(temp);
				if (delete_index == index) {
					ugd->ug_Status.ug_iRadioOn = 0;
					elm_radio_value_set(ugd->ug_MainWindow.ug_pRadioGroup, 0);
					/*delete_index = -1;*/
					mf_ug_navi_bar_button_set_disable(ugd, true);
				}
				continue;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
}
