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

#include <efl_extension.h>

#include "mf-ug-winset.h"
#include "mf-ug-cb.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-util.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-resource.h"
#include "mf-ug-widget.h"
#include "mf-ug-music.h"
#include "mf-ug-view.h"
#include "mf-ug-db-handle.h"
#include "mf-ug-file-util.h"

#define PROGRESSBAR_W	100
#define PROGRESSBAR_H	72

/******************************
** Prototype    : mf_ug_navi_bar_create_group_radio_box
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
void mf_ug_navi_bar_create_group_radio_box(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Evas_Object *group_radio = NULL;

	if (ugd->ug_MainWindow.ug_pRadioGroup != NULL) {
		evas_object_del(ugd->ug_MainWindow.ug_pRadioGroup);
		ugd->ug_MainWindow.ug_pRadioGroup = NULL;
	}

	group_radio = elm_radio_add(ugd->ug_MainWindow.ug_pMainLayout);
	ug_mf_retm_if(group_radio == NULL, "ugd is NULL");

	elm_radio_value_set(group_radio, 0);
	evas_object_hide(group_radio);
	/*/Internal/invisible Radio Objects used to group the radio buttons in the list. */
	ugd->ug_MainWindow.ug_pRadioGroup = group_radio;
	UG_TRACE_END;
}


/******************************
** Prototype    : mf_ug_navi_bar_push_content
** Description  : Samsung
** Input        : ugData *data
**                Evas_Object *NaviContent
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
void mf_ug_navi_bar_push_content(void *data, Evas_Object *NaviContent)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "pNavi_s is NULL");
	ugData *ugd = (ugData *)data;

	Evas_Object *NaviBar = ugd->ug_MainWindow.ug_pNaviBar;
	ug_error("NaviBar is [%p]", NaviBar);
	ugd->ug_MainWindow.ug_pPreNaviItem = ugd->ug_MainWindow.ug_pNaviItem;


	if (ugd->ug_MainWindow.ug_pPreNaviItem) {
		ugd->ug_MainWindow.ug_pNaviItem = elm_naviframe_item_insert_after(NaviBar, ugd->ug_MainWindow.ug_pPreNaviItem, NULL, NULL, NULL, NaviContent, NULL);
	} else {
		ugd->ug_MainWindow.ug_pNaviItem = elm_naviframe_item_push(NaviBar, "", NULL, NULL, NaviContent, NULL);
	}
	UG_TRACE_END;
}


/******************************
** Prototype    : mf_ug_navi_bar_set_ctrl_item_disable
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
void mf_ug_navi_bar_set_ctrl_item_disable(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	bool disable = false;

	if (ugd->ug_Status.ug_bNoContentFlag == true) {
		mf_ug_navi_bar_button_set_disable(ugd, true);
	} else {
		disable = mf_ug_util_is_genlist_selected(ugd);
		mf_ug_navi_bar_button_set_disable(ugd, !disable);
	}

	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_navi_bar_create_navi_bar
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
Evas_Object *mf_ug_navi_bar_create_navi_bar(Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	Evas_Object *navi_bar = NULL;
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	navi_bar = elm_naviframe_add(parent);
	ug_mf_retvm_if(navi_bar == NULL, NULL, "Failed elm_navigationbar_add");
	elm_naviframe_prev_btn_auto_pushed_set(navi_bar, EINA_FALSE);

	evas_object_show(navi_bar);
	eext_object_event_callback_add(navi_bar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(navi_bar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	UG_TRACE_END;
	return navi_bar;
}

/******************************
** Prototype    : mf_ug_navi_bar_set_new_content
** Description  : Samsung
** Input        : Evas_Object *pLayout
**                Evas_Object *NaviContent
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
void mf_ug_navi_bar_set_new_content(Evas_Object *pLayout, Evas_Object *NaviContent)
{
	UG_TRACE_BEGIN;
	Evas_Object *unUsed = elm_object_part_content_unset(pLayout, "part1");
	evas_object_del(unUsed);
	elm_object_part_content_set(pLayout, "part1", NaviContent);
	UG_TRACE_END;
}

Evas_Object *mf_ug_navi_bar_create_box(Evas_Object * parent)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	ug_mf_retvm_if(box == NULL, NULL, "box is NULL");
	elm_object_focus_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_clear(box);
	evas_object_show(box);
	UG_TRACE_END;
	return box;

}

Evas_Object *mf_ug_navi_bar_create_layout(Evas_Object *parent, const char *edj, const char *grp_name)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	ug_mf_retvm_if(edj == NULL, NULL, "edj is NULL");
	ug_mf_retvm_if(grp_name == NULL, NULL, "grp_name is NULL");

	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);
	ug_mf_retvm_if(layout == NULL, NULL, "layout is NULL");
	elm_object_focus_set(layout, EINA_FALSE);
	elm_layout_file_set(layout, edj, grp_name);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);
	UG_TRACE_END;
	return layout;

}

/******************************
** Prototype    : mf_ug_navi_bar_create_default_view
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
static void __mf_ug_search_select_all_check_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	Evas_Object *genlist = ugd->ug_MainWindow.ug_pNaviGenlist;
	if (ugd->ug_Status.ug_bSelectAllChecked) {
		ugd->ug_Status.ug_iCheckedCount = ugd->ug_Status.ug_iTotalCount;
	} else {
		ugd->ug_Status.ug_iCheckedCount = 0;
	}

	ugListItemData *it_data;
	Elm_Object_Item *it;
	int count = 0;

	it = elm_genlist_first_item_get(genlist);
	while (it) {
		it_data = elm_object_item_data_get(it);
		if (it_data) {
			it_data->ug_bChecked = ugd->ug_Status.ug_bSelectAllChecked;
			count++;
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_genlist_realized_items_update(genlist);

	if (count > 0 && ugd->ug_Status.ug_bSelectAllChecked) {
		char *label = NULL;
		/*1 TODO: need to update for multi-language */

		label = g_strdup_printf(mf_ug_widget_get_text(MF_UG_LABEL_SELECTED), count);

		elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "elm.text.title", label);
		UG_SAFE_FREE_CHAR(label);

	} else {
		if (mf_ug_util_is_import_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
			mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_IMPORT_CHAP, "elm.text.title");
		} else if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
			mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_DOCUMENTS, "elm.text.title");
		} else {
			char *label = NULL;
			label = g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_SELECT_ITEMS));
			ug_error("label = %s", label);
			elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "elm.text.title", label);
			UG_SAFE_FREE_CHAR(label);
		}
	}
	mf_ug_navi_bar_set_ctrl_item_disable(ugd);

	UG_TRACE_END;
}

void mf_ug_select_all_layout_mouse_down_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_Status.ug_bSelectAllChecked = !ugd->ug_Status.ug_bSelectAllChecked;
	__mf_ug_search_select_all_check_changed_cb(ugd, NULL, NULL);
	UG_TRACE_END;
}

Evas_Object *__mf_ug_navibar_btn_create(Evas_Object *parent, const char *text)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "naviframe/title_text");
	mf_ug_widget_object_text_set(btn, text, NULL);
	return btn;
}

int mf_ug_navibar_get_ringtone_count(int mode)
{
	int count = 0;
	if (mode == mf_ug_sound_mode_ringtone) {
		count = mf_ug_db_handle_ringtone_get_count();
	} else if (mode == mf_ug_sound_mode_alert) {
		count = mf_ug_db_handle_alert_get_count();
	}
	return count;

}
bool mf_ug_navibar_ringtone_in_db(int mode, const char *path)
{
	if (mode == mf_ug_sound_mode_ringtone) {
		return mf_ug_db_handle_find_ringtone(path);
	} else if (mode == mf_ug_sound_mode_alert) {
		return mf_ug_db_handle_find_alert(path);
	}
	return false;
}

void mf_ug_navi_bar_title_set(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_error("ugd->ug_Status.ug_iViewType = %d", ugd->ug_Status.ug_iViewType);
	if (mf_ug_util_is_import_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
		mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_IMPORT_CHAP, "elm.text.title");
	} else if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_DOCUMENTS, "elm.text.title");
	} else if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
		ug_error("~~~~~~~~~~~~~~~~~~~~~~~~~");
		Evas_Object *pSelectAllLayout = NULL;
		ugd->ug_Status.ug_bSelectAllChecked = false;
		ugd->ug_Status.ug_iTotalCount = elm_genlist_items_count(ugd->ug_MainWindow.ug_pNaviGenlist);
		ugd->ug_Status.ug_iCheckedCount = 0;

		pSelectAllLayout = mf_widget_create_select_all_layout(ugd->ug_MainWindow.ug_pNaviBox);
		ug_mf_retm_if(pSelectAllLayout == NULL, "pSelectAllLayout is NULL");
		ugd->ug_MainWindow.ug_pSelectAllLayout = pSelectAllLayout;
		evas_object_smart_callback_add(pSelectAllLayout, "clicked", mf_ug_select_all_layout_mouse_down_cb, ugd);

		evas_object_show(pSelectAllLayout);
		elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_RIGHT_BTN, pSelectAllLayout);
		mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_SELECT_ITEMS, "elm.text.title");
		if (ugd->ug_Status.ug_iTotalCount == 0/* || ugd->ug_Status.ug_bDisableSelectAll == EINA_TRUE*/) {
			elm_object_disabled_set(pSelectAllLayout, EINA_TRUE);
		}
	} else if (ugd->ug_Status.ug_iViewType != mf_ug_view_root && ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_none) {
#if 0/*Don't support home button at Kiran*/
		Evas_Object *home_ic = elm_image_add(ugd->ug_MainWindow.ug_pNaviBar);
		elm_image_file_set(home_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_HOME);
		elm_image_resizable_set(home_ic, EINA_TRUE, EINA_TRUE);

		Evas_Object *home_btn = mf_ug_widget_create_button(ugd->ug_MainWindow.ug_pNaviBar,
		                        "naviframe/title_icon",
		                        NULL,
		                        home_ic,
		                        mf_ug_cb_home_button_cb,
		                        ugd,
		                        EINA_FALSE);
		evas_object_smart_callback_add(home_btn, "pressed", mf_ug_cb_home_button_pressed_cb, home_ic);
		evas_object_smart_callback_add(home_btn, "unpressed", mf_ug_cb_home_button_unpressed_cb, home_ic);
		/*elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_LEFT_BTN, home_btn);*/
		elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_LEFT_BTN, home_btn);


#if 0
		Evas_Object *up_ic = elm_image_add(ugd->ug_MainWindow.ug_pNaviBar);
		elm_image_file_set(up_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_UPPER);
		elm_image_resizable_set(up_ic, EINA_TRUE, EINA_TRUE);

		Evas_Object *up_btn = mf_ug_widget_create_button(ugd->ug_MainWindow.ug_pNaviBar,
		                      "naviframe/title_icon",
		                      NULL,
		                      up_ic,
		                      mf_ug_cb_upper_click_cb,
		                      ugd,
		                      EINA_FALSE);

		evas_object_smart_callback_add(up_btn, "pressed", mf_ug_cb_upper_button_pressed_cb, up_ic);
		evas_object_smart_callback_add(up_btn, "unpressed", mf_ug_cb_upper_button_unpressed_cb, up_ic);
		elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_RIGHT_BTN, up_btn);
#endif
#endif/*Don't support home button at Kiran*/
		mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, ugd->ug_MainWindow.ug_pNaviTitle, "elm.text.title");
	} else  if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
			Evas_Object *add_ic = elm_image_add(ugd->ug_MainWindow.ug_pNaviBar);
			elm_image_file_set(add_ic, UG_EDJ_IMAGE, UG_ICON_ADD);
			elm_image_resizable_set(add_ic, EINA_TRUE, EINA_TRUE);

			Evas_Object *add_btn = mf_ug_widget_create_button(ugd->ug_MainWindow.ug_pNaviBar,
			                       "naviframe/title_icon",
			                       NULL,
			                       add_ic,
			                       mf_ug_music_launch_cb,
			                       ugd,
			                       EINA_FALSE);
			/*elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_LEFT_BTN, home_btn);*/
			elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_LEFT_BTN, add_btn);

			Evas_Object *del_ic = elm_image_add(ugd->ug_MainWindow.ug_pNaviBar);
			elm_image_file_set(del_ic, UG_EDJ_IMAGE, UG_ICON_DELETE);
			elm_image_resizable_set(del_ic, EINA_TRUE, EINA_TRUE);

			Evas_Object *del_btn = mf_ug_widget_create_button(ugd->ug_MainWindow.ug_pNaviBar,
			                       "naviframe/title_icon",
			                       NULL,
			                       del_ic,
			                       mf_ug_ringtone_del_cb,
			                       ugd,
			                       EINA_FALSE);
			elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_RIGHT_BTN, del_btn);
			/*P131205-01044 by wangyan Dec 13,if 0, or only setted ringtone in db , do not add it in delete genlist to avoid to be deleted,*/
			int count = mf_ug_navibar_get_ringtone_count(ugd->ug_UiGadget.ug_iSoundMode);
			if (count == 0) {
				elm_object_disabled_set(del_btn, EINA_TRUE);
			} else if (count == 1 && mf_ug_navibar_ringtone_in_db(ugd->ug_UiGadget.ug_iSoundMode, ugd->ug_Status.mark_mode)) {
				elm_object_disabled_set(del_btn, EINA_TRUE);
			}
			/*end*/

		}
		if (ugd->ug_UiGadget.title) {
			elm_object_item_domain_translatable_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "elm.text.title", ugd->ug_UiGadget.domain, ugd->ug_UiGadget.title);
		} else if (ugd->ug_MainWindow.ug_pNaviTitle) {
			mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, ugd->ug_MainWindow.ug_pNaviTitle, "elm.text.title");
		}
	} else if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_MYFILES, "elm.text.title");
	}
	/*elm_naviframe_item_title_visible_set(ugd->ug_MainWindow.ug_pNaviItem, EINA_TRUE);*/
}

Eina_Bool mf_ug_navi_search_idler_cb(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");
	ugData *ugd = (ugData *)data;

	mf_ug_search_view_enter_search_routine(ugd, NULL, NULL);
	ugd->ug_Status.search_idler = NULL;

	return ECORE_CALLBACK_CANCEL;
}

Evas_Object *__mf_ug_navi_bar_backbutton_create(Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *btn = NULL;
	btn = elm_button_add(parent);
	elm_object_style_set(btn, "naviframe/end_btn/default");

	elm_access_info_set(btn, ELM_ACCESS_INFO, mf_ug_widget_get_text(MF_UG_LABEL_BACK));
	evas_object_show(btn);
	return btn;

}

void mf_ug_navi_add_back_button(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

//	Evas_Object *pBackButton = NULL;
//
//	pBackButton = __mf_ug_navi_bar_backbutton_create(ugd->ug_MainWindow.ug_pNaviBar);
//	if (pBackButton) {
//		elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, "prev_btn", pBackButton);
//		elm_naviframe_item_pop_cb_set(ugd->ug_MainWindow.ug_pNaviItem, mf_ug_cb_back_button_cb, ugd);
//	}
	elm_naviframe_item_pop_cb_set(ugd->ug_MainWindow.ug_pNaviItem, mf_ug_cb_back_button_cb, ugd);
}

Evas_Object *mf_naviframe_left_cancel_button_create(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
        Evas_Smart_Cb pFunc, void *pUserData)
{
	Evas_Object *btn = elm_button_add(pParent);
	elm_object_style_set(btn, "naviframe/title_left");
	mf_ug_widget_object_item_translate_set(btn, MF_UG_LABEL_CANCEL_CAP);
	evas_object_smart_callback_add(btn, "clicked", pFunc, pUserData);
	elm_object_item_part_content_set(pNaviItem, "title_left_btn", btn);

	evas_object_show(btn);

	return btn;
}

Evas_Object *mf_naviframe_right_save_button_create(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
        Evas_Smart_Cb pFunc, void *pUserData)
{
	Evas_Object *btn = elm_button_add(pParent);
	elm_object_style_set(btn, "naviframe/title_right");
	mf_ug_widget_object_item_translate_set(btn, MF_UG_LABEL_DONE_CAP);
	evas_object_smart_callback_add(btn, "clicked", pFunc, pUserData);
	elm_object_item_part_content_set(pNaviItem, "title_right_btn", btn);

	evas_object_show(btn);

	return btn;
}

void mf_ug_navi_bar_set_ctrl_button(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	Elm_Object_Item *navi_it = ugd->ug_MainWindow.ug_pNaviItem;/*ap->mf_MainWindow.pNaviItem;*/
	Evas_Object *pNavi = ugd->ug_MainWindow.ug_pNaviBar;
	/*Evas_Object *ctrlbar = NULL;*/
	if (mf_ug_util_is_import_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
		/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);
		mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_DONE, mf_ug_cb_add_button_cb, ugd);*/
		mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
		mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_add_button_cb, ugd);
	} else if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
		/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);
		mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_CANCEL, mf_ug_cb_cancel_button_cb, ugd);
		mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_DELETE, mf_ug_cb_delete_button_popup_create, ugd);*/
		mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
		mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_delete_button_popup_create, ugd);
	} else if (ugd->ug_Status.ug_iViewType != mf_ug_view_root) {
		if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE) {
			/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);
			Evas_Object *more_bt = mf_ug_widget_create_button(pNavi, NAVI_BUTTON_EDIT, NULL, NULL, mf_ug_cb_more_cb, ugd, EINA_FALSE);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_CANCEL, mf_ug_cb_cancel_button_cb, ugd);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_EXPORT, mf_ug_cb_add_button_cb, ugd);
			if (more_bt) {
				elm_object_item_part_content_set(navi_it, NAVI_MORE_BUTTON_PART, more_bt);
			}*/
			mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
			mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_add_button_cb, ugd);
		} else if (ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE) {
			/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);
			Evas_Object *more_bt = mf_ug_widget_create_button(pNavi, NAVI_BUTTON_EDIT, NULL, NULL, mf_ug_cb_more_cb, ugd, EINA_FALSE);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_CANCEL, mf_ug_cb_cancel_button_cb, ugd);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_SAVE_HERE, mf_ug_cb_add_button_cb, ugd);
			if (more_bt) {
				elm_object_item_part_content_set(navi_it, NAVI_MORE_BUTTON_PART, more_bt);
			}*/
			mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
			mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_add_button_cb, ugd);
		} else if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
			/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);

			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_CANCEL, mf_ug_cb_cancel_button_cb, ugd);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_SET, mf_ug_cb_add_button_cb, ugd);*/
			mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
			mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_add_button_cb, ugd);
		} else if (!(ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE)) {
			/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_CANCEL, mf_ug_cb_cancel_button_cb, ugd);
			mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_DONE, mf_ug_cb_add_button_cb, ugd);*/
			mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
			mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_add_button_cb, ugd);
		}
	}
	if (ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		/*ctrlbar = mf_ug_widget_toolbar_create(pNavi);
		mf_ug_widget_item_tabbar_item_append(ctrlbar, NULL, MF_UG_LABEL_DONE, mf_ug_cb_add_button_cb, ugd);*/
		mf_naviframe_left_cancel_button_create(pNavi, navi_it, mf_ug_cb_cancel_button_cb, ugd);
		mf_naviframe_right_save_button_create(pNavi, navi_it, mf_ug_cb_add_button_cb, ugd);
	}
	/*if (ctrlbar) {
		ugd->ug_MainWindow.ug_pNaviCtrlBar = ctrlbar;
		elm_object_item_part_content_set(navi_it, "toolbar", ctrlbar);
	}*/
	mf_ug_navi_add_back_button(ugd);
}

static void _index_clicked(void *data, Evas_Object *obj, const char *em, const char *src)
{
	if (!obj) {
		return;
	}
	elm_object_signal_emit(obj, "elm,state,slide,start", "");
}

Evas_Object *mf_genlist_create_path_info(Evas_Object *parent, const char *info, Evas_Object **pathinfo)
{
	UG_TRACE_BEGIN

	Evas_Object *bx = elm_box_add(parent);
	Evas_Object *ly = elm_layout_add(parent);
	elm_layout_theme_set(ly, "genlist/item", "groupindex", "default");
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(ly, -1, -1);
	mf_ug_widget_object_text_set(ly, info, "elm.text");
	elm_layout_signal_callback_add(ly, "mouse,clicked,1", "*", _index_clicked, NULL);
	evas_object_show(ly);
	elm_box_pack_end(bx, ly);
	if (pathinfo) {
		*pathinfo = ly;
	}
	return bx;
}

void mf_ug_navi_bar_set_path_state(Evas_Object *layout, Eina_Bool flag)
{
	if (!flag) {
		ug_error();
		elm_object_signal_emit(layout, "elm.pathinfo.hide", "elm");
	} else {
		ug_error();
		elm_object_signal_emit(layout, "elm.pathinfo.show", "elm");
	}
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

	ugd->show = NULL;
	return ECORE_CALLBACK_CANCEL;
}

void mf_ug_navi_bar_create_default_view(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Evas_Object *pNaviLayout = NULL;
	Evas_Object *newContent = NULL;
	Evas_Object *box = NULL;

	/*if the muisc is playing, destory the play */
	if (0 != ugd->ug_ListPlay.ug_Player) {
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}
	ugd->ug_Status.ug_iCheckedCount = 0;
	box = mf_ug_navi_bar_create_box(ugd->ug_MainWindow.ug_pNaviBar);
	pNaviLayout = mf_ug_navi_bar_create_layout(box, UG_EDJ_NAVIGATIONBAR, UG_GRP_NAVI_VIEW);
	ugd->ug_MainWindow.ug_pNaviLayout = pNaviLayout;


	ug_error("ugd->ug_UiGadget.ug_iSelectMode = %d", ugd->ug_UiGadget.ug_iSelectMode);
	if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE
	        || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE
	        || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE
	        || ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE
	        || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {
			mf_ug_navi_bar_create_group_radio_box(ugd);
		}
		mf_ug_navi_bar_set_path_state(pNaviLayout, EINA_FALSE);
		newContent = __mf_ug_genlist_create_gl(ugd);
		ugd->ug_MainWindow.ug_pNaviGenlist = newContent;
	} else {
		if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
			mf_ug_navi_bar_set_path_state(pNaviLayout, EINA_FALSE);
		} else if (ugd->ug_Status.ug_iViewType != mf_ug_view_root) {
			Evas_Object *pathinfo_layout = NULL;
			char *info = NULL;
			pathinfo_layout = mf_ug_tabbar_create_path_tab(pNaviLayout, ugd->ug_Status.ug_pPath->str);/*mf_genlist_create_path_info(top_layout,info,&ugd->ug_MainWindow.pPathinfo);*/
			elm_object_part_content_set(pNaviLayout, "pathinfo", pathinfo_layout);
			UG_SAFE_FREE_CHAR(info);
		} else {
			mf_ug_navi_bar_set_path_state(pNaviLayout, EINA_FALSE);
		}

		/*set content */
		if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE
		        || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
			mf_ug_navi_bar_create_group_radio_box(ugd);
		}

		newContent = mf_ug_genlist_create_content_list_view(ugd);
		if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
			if (!ugd->show) {
				ugd->show = ecore_idler_add(__selected_item_show, ugd);
			}
		}
		ugd->ug_MainWindow.ug_pNaviGenlist = newContent;
		UG_SAFE_FREE_CHAR(ugd->ug_MainWindow.ug_pNaviTitle);
		/*set title segment or title */
		if (ugd->ug_UiGadget.title) {
			ugd->ug_MainWindow.ug_pNaviTitle = g_strdup(ugd->ug_UiGadget.title);
		} else {
			if (mf_ug_fm_svc_wapper_is_root_path(ugd->ug_Status.ug_pPath->str) || ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
				ugd->ug_MainWindow.ug_pNaviTitle = g_strdup(MF_UG_LABEL_MYFILES);
			} else {
				ugd->ug_MainWindow.ug_pNaviTitle = g_strdup(mf_file_get(ugd->ug_Status.ug_pPath->str));
			}

		}
	}

	/*navigation view integration */
	evas_object_show(newContent);
	elm_box_pack_end(box, newContent);
	ugd->ug_MainWindow.ug_pNaviBox = box;
	elm_object_part_content_set(pNaviLayout, "part1", box);

	if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE ||
	        ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
		mf_ug_navi_bar_push_content(ugd, pNaviLayout);
	} else {
		if (eina_list_count(ugd->ug_UiGadget.ug_pFilterList) != 0) {
			if (ugd->ug_MainWindow.ug_pNaviBox) {
				Evas_Object *pSelectAllLayout = NULL;
				Evas_Object *pSelectAllCheckBox = NULL;
				mf_ug_object_create_select_all_layout(ugd->ug_MainWindow.ug_pNaviBar, mf_ug_select_all_cb, (Evas_Object_Event_Cb)mf_ug_item_sel_all_press_cb , ugd, &pSelectAllCheckBox, &pSelectAllLayout);
				ugd->ug_MainWindow.ug_pSelectAllCheckBox = pSelectAllCheckBox;
				if (pSelectAllLayout && pSelectAllCheckBox) {
					elm_box_pack_start(ugd->ug_MainWindow.ug_pNaviBox, pSelectAllLayout);
				}
			}
		}
		mf_ug_navi_bar_push_content(ugd, pNaviLayout);
	}
	/*add control bar for navigation bar */
	/*mf_ug_ctrl_bar_set(ugd, pNavi_s);*/
	mf_ug_navi_bar_set_ctrl_button(ugd);

	mf_ug_navi_bar_title_set(ugd);
	mf_ug_navi_bar_remove_previous_contents(ugd);

	if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE
	        || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE
	        || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE
	        || ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE
	        || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		ug_ecore_idler_del(ugd->ug_Status.search_idler);
		ugd->ug_Status.search_idler = ecore_idler_add(mf_ug_navi_search_idler_cb, ugd);
	}
	UG_SAFE_DEL_ECORE_TIMER(ugd->ug_Status.play_timer);
	UG_TRACE_END;
}

void mf_ug_navi_bar_create_delete_view(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Evas_Object *pNaviLayout = NULL;
	Evas_Object *newContent = NULL;
	Evas_Object *box = NULL;

	/*if the muisc is playing, destory the play */
	if (0 != ugd->ug_ListPlay.ug_Player) {
		mf_ug_list_play_destory_playing_file(ugd);
		ugd->ug_ListPlay.play_data = NULL;
		UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
	}

	box = mf_ug_navi_bar_create_box(ugd->ug_MainWindow.ug_pNaviBar);
	pNaviLayout = mf_ug_navi_bar_create_layout(box, UG_EDJ_NAVIGATIONBAR, UG_GRP_NAVI_VIEW);
	ugd->ug_MainWindow.ug_pNaviLayout = pNaviLayout;

	ug_error("ugd->ug_UiGadget.ug_iSelectMode = %d", ugd->ug_UiGadget.ug_iSelectMode);
	mf_ug_navi_bar_set_path_state(pNaviLayout, EINA_FALSE);
	newContent = mf_ug_genlist_delete_style_create(ugd);
	ugd->ug_MainWindow.ug_pNaviGenlist = newContent;
	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		/*P131205-01044 by wangyan*/
		mf_ug_genlist_ringtone_delete_items_add(ugd, 0);
	}

	/*navigation view integration */
	evas_object_show(newContent);
	elm_object_part_content_set(pNaviLayout, "part1", newContent);
	elm_box_pack_end(box, pNaviLayout);
	ugd->ug_MainWindow.ug_pNaviBox = box;
	mf_ug_navi_bar_push_content(ugd, box);

	mf_ug_navi_bar_set_ctrl_button(ugd);

	mf_ug_navi_bar_title_set(ugd);
	mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	mf_ug_navi_bar_remove_previous_contents(ugd);
	UG_SAFE_DEL_ECORE_TIMER(ugd->ug_Status.play_timer);
	UG_TRACE_END;
}


void mf_ug_navi_bar_remove_previous_contents(void *data)
{
	UG_TRACE_BEGIN;

	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	UG_SAFE_DEL_NAVI_ITEM(&ugd->ug_MainWindow.ug_pPreNaviItem);
	UG_TRACE_END;
}


void mf_ug_navi_bar_button_set_disable(void *data, bool disable)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Elm_Object_Item *navi_it = ugd->ug_MainWindow.ug_pNaviItem;
#if 0
	Evas_Object *ctrlbar = elm_object_item_part_content_get(navi_it, "toolbar");
	Elm_Object_Item *item = NULL;
	const char *button_label = NULL;
	item = elm_toolbar_first_item_get(ctrlbar);

	while (item) {
		button_label = elm_object_item_part_text_get(item, "elm.text");

		if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
			if (g_strcmp0(button_label, mf_ug_widget_get_text(MF_UG_LABEL_DELETE)) == 0
			        || g_strcmp0(button_label, MF_UG_LABEL_DELETE) == 0) {
				elm_object_item_disabled_set(item, disable);
			}
		} else if (mf_ug_util_is_import_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
			if (g_strcmp0(button_label, mf_ug_widget_get_text(MF_UG_LABEL_DONE)) == 0
			        || g_strcmp0(button_label, MF_UG_LABEL_DONE) == 0) {
				elm_object_item_disabled_set(item, disable);
			}
		} else if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
			if (g_strcmp0(button_label, mf_ug_widget_get_text(MF_UG_LABEL_SET)) == 0
			        || g_strcmp0(button_label, MF_UG_LABEL_SET) == 0) {
				elm_object_item_disabled_set(item, disable);
			}
		} else {
			if (g_strcmp0(button_label, mf_ug_widget_get_text(MF_UG_LABEL_DONE)) == 0
			        || g_strcmp0(button_label, MF_UG_LABEL_DONE) == 0) {
				elm_object_item_disabled_set(item, disable);
			}
		}
		button_label = NULL;
		item = elm_toolbar_item_next_get(item);

	}
#endif
	Evas_Object *btn = elm_object_item_part_content_get(navi_it, "title_right_btn");
	if (btn) {
		elm_object_disabled_set(btn, disable);
	}
}
