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

#include <notification.h>
#include <efl_extension.h>

#include "mf-ug-util.h"
#include "mf-ug-winset.h"
#include "mf-ug-widget.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-cb.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-resource.h"
#include "mf-ug-music.h"
#include "mf-ug-db-handle.h"

#define MF_UG_POPUP_BTN_STYLE	"popup_button/default"
#define MF_UG_POPUP_STYLE_MIN_MENUSTYLE "min_menustyle"
#define MF_UG_POPUP_MENUSTYLE_WIDTH (614*elm_config_scale_get())
#define MF_UG_POPUP_MENUSTYLE_HEIGHT(x) ((113*x-1)*elm_config_scale_get())
#define MF_UG_POPUP_MENUSTYLE_HEIGHT_MAX (408*elm_config_scale_get())
#define MF_UG_ITEM_COUNT	4

#define MF_CTXPOPUP_OBJ_DATA_KEY "mf_ctxpopup_data_key"
#define MF_CTXPOPUP_OBJ_MORE_BTN_KEY "mf_ctxpopup_more_btn_key"
#define MF_CTXPOPUP_OBJ_ROTATE_KEY "mf_ctxpopup_rotate_key"
#define MF_CTXPOPUP_STYLE_MORE "more/default"

static Eina_Bool present_flag = EINA_FALSE;

static Evas_Object *global_progress = NULL;

void mf_ug_progress_set(Evas_Object *progress)
{
	global_progress = progress;
}

Evas_Object *mf_ug_progress_get()
{
	return global_progress;
}

void mf_ug_popup_present_flag_set(Eina_Bool flag)
{
	present_flag = flag;

}

Eina_Bool mf_ug_popup_present_flag_get()
{
	return present_flag;
}

void mf_ug_popup_present_del(const char *fullpath, const char *present_path)
{
	ug_mf_retm_if(fullpath == NULL, "fullpath is NULL");
	ug_mf_retm_if(present_path == NULL, "present_path is NULL");

	if (g_strcmp0(fullpath, present_path) == 0) {
		mf_ug_popup_present_flag_set(EINA_TRUE);
	}
}
static void __mf_popup_new_folder_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNewFolderPopup);
	ugd->ug_Status.ug_iMore = UG_MORE_DEFAULT;

}
static void __mf_popup_search_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_MainWindow.ug_pSearchPopup = NULL;
	ugd->ug_MainWindow.ug_pSearchLabel = NULL;

}
static void __mf_popup_normal_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_MainWindow.ug_pNormalPopup = NULL;

}

/******************************
** Prototype    : mfPopupCreate
** Description  :
** Input        : void *data
**                ePopMode popupMode
**                char *title
**                char *context
**                char *first_btn_text
**                char *second_btn_text
**                char *third_btn_text
**                Evas_Smart_Cb func
**                void* param
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
Evas_Object *mf_ug_popup_create_search(void *data, Evas_Smart_Cb back_func, void *back_param)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");
	/*To fix P131203-07786 by wangyan[S Note]"Processing..." information is not centered on displayed popup.*/
	Evas_Object *popup;
	Evas_Object *progressbar;
	Evas_Object *layout;

	popup = elm_popup_add(ugd->ug_MainWindow.ug_pMainLayout);
	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, UG_EDJ_NAVIGATIONBAR, "popup_processingview_1button");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/*get the processing object
	label = elm_layout_add(layout);
	elm_object_part_content_set(layout, "elm.swallow.text", label);
	ugd->ug_MainWindow.ug_pSearchLabel = label;*/

	mf_ug_widget_object_text_set(popup, MF_UG_LABEL_SEARCH, "title,text");
	mf_ug_widget_object_text_set(layout, MF_UG_LABEL_PROCESSING, "elm.text");

	progressbar = elm_progressbar_add(popup);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_style_set(progressbar, "process_large");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);
	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);

	elm_object_content_set(popup, layout);
	if (back_func) {
		Evas_Object *btn1 = mf_ug_widget_create_button(popup,
		                    "popup_button/default",
		                    MF_UG_LABEL_CANCEL,
		                    NULL,
		                    back_func,
		                    back_param,
		                    EINA_FALSE);
		elm_object_part_content_set(popup, "button1", btn1);
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, back_func, back_param);
	} else {
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, data);
	}
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_search_del_cb, ugd);
	evas_object_show(popup);
	return popup;
}

void mf_ug_popup_del_by_timeout(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	if (ugd->ug_MainWindow.ug_pNormalPopup) {/*Add the protection*/
		evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = NULL;
	}
	if (ugd->ug_MainWindow.ug_pWindow) {
		elm_object_focus_set(ugd->ug_MainWindow.ug_pWindow, EINA_TRUE);
	}
	UG_TRACE_END;

}

Evas_Object *mf_ug_popup_create(void *data, mf_ug_popup_mode popupMode, char *title, const char *context, const char *first_btn_text, const char *second_btn_text,
                                const char *third_btn_text, Evas_Smart_Cb func, void *param)
{
	Evas_Object *popup;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");

	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	popup = elm_popup_add(ugd->ug_MainWindow.ug_pMainLayout);

	ugd->ug_MainWindow.ug_pNormalPopup = popup;
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title) {
		mf_ug_widget_object_text_set(popup, title, "title,text");
	}
	if (context && popupMode != UG_POPMODE_PROGRESSBAR) {
		mf_ug_widget_object_text_set(popup, context, NULL);
	}
	switch (popupMode) {
	case UG_POPMODE_TEXT:
	case UG_POPMODE_TITLE_TEXT:
		elm_popup_timeout_set(popup, 3);
		if (func) {
			evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb) func, param);
		} else {
			evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb) mf_ug_popup_del_by_timeout, ugd);
		}
		break;
	case UG_POPMODE_TEXT_TWO_BTN:
	case UG_POPMODE_TITLE_TEXT_TWO_BTN:

		btn1 = mf_ug_widget_create_button(popup,
		                                  MF_UG_POPUP_BTN_STYLE,
		                                  first_btn_text,
		                                  NULL,
		                                  func,
		                                  param,
		                                  EINA_FALSE);
		btn2 = mf_ug_widget_create_button(popup,
		                                  MF_UG_POPUP_BTN_STYLE,
		                                  second_btn_text,
		                                  NULL,
		                                  func,
		                                  param,
		                                  EINA_FALSE);
		elm_object_part_content_set(popup, "button1", btn1);
		elm_object_part_content_set(popup, "button2", btn2);
		break;
	case UG_POPMODE_TEXT_BTN:
	case UG_POPMODE_TITLE_TEXT_BTN:
		btn1 = mf_ug_widget_create_button(popup,
		                                  MF_UG_POPUP_BTN_STYLE,
		                                  MF_UG_LABEL_OK,
		                                  NULL,
		                                  func,
		                                  param,
		                                  EINA_TRUE);
		elm_object_part_content_set(popup, "button1", btn1);
		break;
	case UG_POPMODE_SEARCH:
		btn1 = mf_ug_widget_create_button(popup,
		                                  MF_UG_POPUP_BTN_STYLE,
		                                  MF_UG_LABEL_CANCEL,
		                                  NULL,
		                                  func,
		                                  param,
		                                  EINA_TRUE);
		elm_object_part_content_set(popup, "button1", btn1);
		break;
	default:
		evas_object_del(popup);
		return NULL;
	}
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_normal_del_cb, ugd);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, data);
	evas_object_show(popup);
	return popup;
}

void mf_ug_popup_indicator_popup(char *text)
{
	ug_mf_retm_if(text == NULL, "text is NULL");
	int ret = notification_status_message_post(text);
	ug_debug("status_message_post()... [0x%x]!", ret);
	if (ret != 0) {
		ug_debug("status_message_post()... [0x%x]!", ret);
	}
	return ;
}

static void _move_more_ctxpopup(void *data, Evas_Object *win, Evas_Object *ctxpopup)
{
	ug_mf_retm_if(data == NULL, "data is NULL");
	Evas_Coord w, h;
	int pos = -1;
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);
	switch (pos) {
	case 0:
	case 180:
		evas_object_move(ctxpopup, 0, h);
		break;
	case 90:
		evas_object_move(ctxpopup, 0, w);
		break;
	case 270:
		evas_object_move(ctxpopup, h, w);
		break;
	}
}

static void __mf_ctxpopup_hide_cb(void *data, Evas_Object *obj, void *ei)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(!data, "data is NULL");
	ug_mf_retm_if(!obj, "obj is NULL");
	ugData *ugd = (ugData *)data;

	bool ct_rotate = (bool)evas_object_data_get(obj,
	                 MF_CTXPOPUP_OBJ_ROTATE_KEY);

	if (!ct_rotate) {
		ug_debug("ctxpopup is dismissed");
		evas_object_del(obj);
		ugd->ug_MainWindow.ug_pContextPopup = NULL;
	} else {
		ug_debug("ctxpopup is not dismissed");
		/* when "dismissed" cb is called next time,
		  * ctxpopup should be dismissed if device is not rotated. */
		evas_object_data_set(obj, MF_CTXPOPUP_OBJ_ROTATE_KEY,
		                     (void *)false);
		/* If ctxpopup is not dismissed, then it must be shown again.
		  * Otherwise "dismissed" cb will be called one more time. */
		if (ugd->ug_MainWindow.ug_pContextPopup) {
			_move_more_ctxpopup(ugd, ugd->ug_MainWindow.ug_pWindow, ugd->ug_MainWindow.ug_pContextPopup);
			evas_object_show(ugd->ug_MainWindow.ug_pContextPopup);
		}
	}
}

static void __mf_ctxpopup_parent_resize_cb(void *data, Evas *e,
        Evas_Object *obj, void *ei)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(!data, "data is NULL");
	evas_object_data_set((Evas_Object *)data, MF_CTXPOPUP_OBJ_ROTATE_KEY,
	                     (void *)true);
}
/*
static void __mf_ctxpopup_items_update_cb(void *data, Evas_Object *obj, void *ei)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(!data, "data is NULL");
	int (*update_cb)(void *data, Evas_Object *parent);
	update_cb = evas_object_data_get((Evas_Object *)data,
					 "mf_ctxpopup_update_items_cb");
	mf_sdbg("callback: %p", update_cb);
	if (update_cb)
		update_cb(ei, (Evas_Object *)data);
}
*/

static void __mf_ctxpopup_rotate_cb(void *data, Evas_Object *obj, void *ei)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(!data, "data is NULL");
	ugData *ugd = (ugData *)data;

	/*Evas_Object *more_btn = NULL;
	more_btn = (Evas_Object *)evas_object_data_get(ugd->ug_MainWindow.ug_pContextPopup,
						       MF_CTXPOPUP_OBJ_MORE_BTN_KEY);
	ug_mf_retm_if(!more_btn, "more_btn is NULL");*/
	if (ugd->ug_MainWindow.ug_pContextPopup) {
		_move_more_ctxpopup(ugd, ugd->ug_MainWindow.ug_pWindow, ugd->ug_MainWindow.ug_pContextPopup);
		evas_object_show(ugd->ug_MainWindow.ug_pContextPopup);
	}
	/*__mf_ctxpopup_show(data, more_btn, ugd->ug_MainWindow.ug_pContextPopup);*/
}

static void __mf_ctxpopup_del_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(!data, "data is NULL");
	ug_mf_retm_if(!obj, "obj is NULL");
	Evas_Object *ctxpopup = obj;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(!ugd->ug_MainWindow.ug_pWindow, "ugd->ug_MainWindow.ug_pWindow is NULL");

	evas_object_data_del(ctxpopup, MF_CTXPOPUP_OBJ_MORE_BTN_KEY);
	evas_object_data_del(ctxpopup, MF_CTXPOPUP_OBJ_ROTATE_KEY);
	evas_object_smart_callback_del(ctxpopup, "dismissed",
	                               __mf_ctxpopup_hide_cb);
	evas_object_event_callback_del(ctxpopup, EVAS_CALLBACK_DEL,
	                               __mf_ctxpopup_del_cb);
	evas_object_event_callback_del(ugd->ug_MainWindow.ug_pWindow,
	                               EVAS_CALLBACK_RESIZE,
	                               __mf_ctxpopup_parent_resize_cb);
	/*evas_object_smart_callback_del(ugd->maininfo.naviframe,
				       "ctxpopup,items,update",
				       __mf_ctxpopup_items_update_cb);*/
	evas_object_smart_callback_del(elm_object_top_widget_get(ctxpopup),
	                               "rotation,changed",
	                               __mf_ctxpopup_rotate_cb);

	ug_debug("done");
}

static int __mf_ctxpopup_add_callbacks(void *data, Evas_Object *ctxpopup)
{
	ug_mf_retvm_if(!data, -1, "data is NULL");
	ug_mf_retvm_if(!ctxpopup, -1, "ctxpopup is NULL");
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(!ugd->ug_MainWindow.ug_pWindow, -1, "ugd->ug_MainWindow.ug_pWindow is NULL");

	/*evas_object_event_callback_del(ugd->ug_MainWindow.ug_pWindow, EVAS_CALLBACK_RESIZE, mf_ug_resize_more_ctxpopup_cb);
	evas_object_event_callback_add(ugd->ug_MainWindow.ug_pWindow, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)mf_ug_resize_more_ctxpopup_cb, ugd);*/

	evas_object_smart_callback_add(ctxpopup, "dismissed",
	                               __mf_ctxpopup_hide_cb, data);
	evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_DEL,
	                               __mf_ctxpopup_del_cb, data);
	evas_object_event_callback_add(ugd->ug_MainWindow.ug_pWindow,
	                               EVAS_CALLBACK_RESIZE,
	                               __mf_ctxpopup_parent_resize_cb,
	                               ctxpopup);
	/*evas_object_smart_callback_add(ugd->ug_MainWindow.ug_pWindow,
				       "ctxpopup,items,update",
				       __mf_ctxpopup_items_update_cb, ctxpopup);*/
	evas_object_smart_callback_add(elm_object_top_widget_get(ctxpopup),
	                               "rotation,changed",
	                               __mf_ctxpopup_rotate_cb, data);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
	ug_debug("done");
	return 0;
}

void mf_ug_context_popup_create_more(void *data, Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;
	Evas_Object *image = NULL;

	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pContextPopup);

	Evas_Object *ctxpopup = elm_ctxpopup_add(ugd->ug_MainWindow.ug_pWindow);
	elm_object_style_set(ctxpopup, "more/default");

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
	                                    ELM_CTXPOPUP_DIRECTION_UNKNOWN,
	                                    ELM_CTXPOPUP_DIRECTION_UNKNOWN,
	                                    ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	UG_TRACE_END;
	ugd->ug_MainWindow.ug_pContextPopup = ctxpopup;
	Elm_Object_Item *it = NULL;
	if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE) {
		/*Search*/
		image = elm_image_add(ctxpopup);
		elm_image_file_set(image, UG_EDJ_IMAGE, UG_ICON_CREATE_FOLDER);

		it = elm_ctxpopup_item_append(ctxpopup, MF_UG_LABEL_CREATE, image, mf_ug_cb_create_new_folder, ugd);
		mf_ug_widget_object_item_translate_set(it, MF_UG_LABEL_CREATE);
	} else if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
		image = elm_image_add(ctxpopup);
		elm_image_file_set(image, UG_EDJ_IMAGE, UG_ICON_ADD);
		it = elm_ctxpopup_item_append(ctxpopup, MF_UG_LABEL_ADD, image, mf_ug_music_launch_cb, ugd);
		mf_ug_widget_object_item_translate_set(it, MF_UG_LABEL_ADD);

		image = elm_image_add(ctxpopup);
		elm_image_file_set(image, UG_EDJ_IMAGE, UG_ICON_DELETE);
		it = elm_ctxpopup_item_append(ctxpopup, MF_UG_LABEL_DELETE, image, mf_ug_ringtone_del_cb, ugd);
		mf_ug_widget_object_item_translate_set(it, MF_UG_LABEL_DELETE);

		Evas_Object *content = ugd->ug_MainWindow.ug_pNaviGenlist;
		Elm_Object_Item *gli = elm_genlist_first_item_get(content);
		Elm_Object_Item *nli = NULL;
		ugListItemData *selected_data = NULL;

		while (gli) {
			ugListItemData *params = (ugListItemData *)elm_object_item_data_get(gli);
			ug_mf_retm_if(params == NULL, "params is NULL");
			if (params->ug_pRadioBox) {
				if (elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup) == params->ug_iGroupValue) {
					selected_data = params;
					break;
				}
			}
			nli = elm_genlist_item_next_get(gli);
			gli = nli;
		}

		if (selected_data) {
			char *select_item = selected_data->ug_pItemName->str;
			if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
				if (mf_ug_db_handle_ringtone_in_db(select_item)) {
					elm_object_item_disabled_set(it, EINA_FALSE);
				} else {
					elm_object_item_disabled_set(it, EINA_TRUE);
				}
			} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
				if (mf_ug_db_handle_alert_in_db(select_item)) {
					elm_object_item_disabled_set(it, EINA_FALSE);
				} else {
					elm_object_item_disabled_set(it, EINA_TRUE);
				}
			}
		} else {
			elm_object_item_disabled_set(it, EINA_TRUE);
		}
	}
	_move_more_ctxpopup(ugd, ugd->ug_MainWindow.ug_pWindow, ctxpopup);
	evas_object_show(ctxpopup);

	if (ctxpopup) {
		/*evas_object_smart_callback_add(ctxpopup,"dismissed", _ctxpopup_hide_cb, ugd);*/
		__mf_ctxpopup_add_callbacks(ugd, ctxpopup);
	}
	UG_TRACE_END;
}

static Evas_Object *__mf_ug_popup_entry_create(Evas_Object *parent)
{
	Evas_Object *en = NULL;

	en = elm_entry_add(parent);/*Using the style to instead of the entry, it will include the other style.*/
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_select_all(en);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,
	                        ELM_SCROLLER_POLICY_AUTO);

	elm_entry_single_line_set(en, EINA_TRUE);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(en);

	return en;

}

static char *__new_folder_text_get(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");

	char *fileName = NULL;
	char *fullpath = (char *)malloc(sizeof(char) * MYFILE_FILE_PATH_LEN_MAX);
	if (fullpath == NULL) {
		return NULL;
	}
	GString *dest_fullpath = NULL;

	memset(fullpath, 0, MYFILE_FILE_PATH_LEN_MAX);
	fileName = elm_entry_markup_to_utf8(mf_ug_widget_get_text(MF_UG_LABEL_FOLDER));
	snprintf(fullpath, (MYFILE_FILE_PATH_LEN_MAX), "%s/%s", (char *)ugd->ug_Status.ug_pPath->str, fileName);

	dest_fullpath = g_string_new(fullpath);


	if (mf_ug_fm_svc_wrapper_detect_duplication(dest_fullpath)) {
		mf_ug_fm_svc_wrapper_file_auto_rename(ugd, dest_fullpath, FILE_NAME_WITH_BRACKETS, &dest_fullpath);
		memset(fullpath, 0, strlen(fullpath));
		int len = strlen(ugd->ug_Status.ug_pPath->str) + 1;
		strncpy(fullpath, dest_fullpath->str + len, MYFILE_FILE_PATH_LEN_MAX);
		/*memset(fileName,0,strlen(fileName));*/

		UG_SAFE_FREE_CHAR(fileName);
		fileName = elm_entry_markup_to_utf8(fullpath);
	}
	/*elm_entry_entry_set(entry, ecore_file_file_get(params->m_ItemName->str));*/
	UG_SAFE_FREE_CHAR(fullpath);
	UG_SAFE_FREE_GSTRING(dest_fullpath);
	return fileName;
}

static void __mf_ug_popup_show_vk_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	if (ugd->ug_MainWindow.ug_pNormalPopup) { /*add protection*/
		evas_object_del(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = NULL;
	}

	if (ugd->ug_MainWindow.ug_pEntry) { /*add protection*/
		elm_entry_cursor_end_set(ugd->ug_MainWindow.ug_pEntry);
		elm_object_focus_set(ugd->ug_MainWindow.ug_pEntry, EINA_TRUE);
	}
	UG_TRACE_END;

}

static void __mf_ug_popup_create_folder_imf_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	const char *entry_data = NULL;
	char *name = NULL;
	char new_str[MYFILE_FILE_NAME_LEN_MAX] = { '\0', };

	entry_data = elm_entry_entry_get(ugd->ug_MainWindow.ug_pEntry);
	ug_mf_retm_if(entry_data == NULL, "entry_data is null");
	name = elm_entry_markup_to_utf8(entry_data);
	ug_mf_retm_if(name == NULL, "name is null");

	SECURE_DEBUG("name is [%s]", name);
	if (mf_ug_file_attr_is_valid_name(name) != MYFILE_ERR_NONE) {
		strncpy(new_str, name, MYFILE_FILE_NAME_LEN_MAX - 1);
		if (strlen(name) > 0) {
			new_str[strlen(name) - 1] = '\0';
		}
		elm_entry_entry_set(ugd->ug_MainWindow.ug_pEntry, new_str);
		elm_entry_cursor_end_set(ugd->ug_MainWindow.ug_pEntry);
		elm_object_focus_set(ugd->ug_MainWindow.ug_pEntry, EINA_FALSE);

		UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pNormalPopup);
		ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL,
		                                     MF_UG_LABEL_ILLEGAL_CHAR, NULL, NULL,
		                                     NULL, __mf_ug_popup_show_vk_cb, ugd);
	}

	UG_SAFE_FREE_CHAR(name);
	UG_TRACE_END;
}

Evas_Object *mf_ug_popup_create_new_folder_popup(void *data, char *context)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ugData *ugd = (ugData *)data;

	Evas_Object *popup;
	Evas_Object *layout;
	char *text = NULL;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	popup = elm_popup_add(ugd->ug_MainWindow.ug_pMainLayout);

	elm_object_focus_set(popup, EINA_FALSE);

	layout = elm_layout_add(popup);
	elm_object_focus_set(layout, EINA_FALSE);
	elm_layout_file_set(layout, UG_EDJ_NAVIGATIONBAR, "popup_new_folder");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *en = NULL;
	en = __mf_ug_popup_entry_create(layout);

	text = __new_folder_text_get(ugd);
	limit_filter_data.max_char_count = MYFILE_FILE_NAME_LEN_MAX;
	elm_entry_entry_set(en, text);
	elm_entry_cursor_end_set(en);
	UG_SAFE_FREE_CHAR(text);

	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size,
	                               &limit_filter_data);
	elm_object_part_content_set(layout, "elm.swallow.content", en);
	mf_ug_widget_object_text_set(popup, context, "title,text");

	evas_object_smart_callback_add(en, "maxlength,reached", mf_ug_cb_reach_max_len_cb, ugd);
	evas_object_smart_callback_add(en, "changed", __mf_ug_popup_create_folder_imf_changed_cb, ugd);
	evas_object_smart_callback_add(en, "activated", mf_ug_cb_save_cb, ugd);
	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	ugd->ug_MainWindow.ug_pEntry = en;

	elm_object_content_set(popup, layout);

	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	btn1 = mf_ug_widget_create_button(popup,
	                                  MF_UG_POPUP_BTN_STYLE,
	                                  MF_UG_LABEL_CANCEL,
	                                  NULL,
	                                  mf_ug_cb_cancel_new_folder_cb,
	                                  ugd,
	                                  EINA_FALSE);
	btn2 = mf_ug_widget_create_button(popup,
	                                  MF_UG_POPUP_BTN_STYLE,
	                                  MF_UG_LABEL_OK,
	                                  NULL,
	                                  mf_ug_cb_save_cb,
	                                  ugd,
	                                  EINA_FALSE);

	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_new_folder_del_cb, ugd);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, data);

	evas_object_show(popup);

	return popup;
}

Evas_Object *mf_popup_center_processing(Evas_Object *parent,
                                        const char *context,
                                        Evas_Smart_Cb func,
                                        void *param)
{
	Evas_Object *popup;
	Evas_Object *progressbar;
	Evas_Object *layout;
	popup = elm_popup_add(parent);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, UG_EDJ_NAVIGATIONBAR, "popup_progress_text");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_style_set(progressbar, "list_process");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);
	mf_ug_progress_set(progressbar);

	mf_ug_widget_object_text_set(layout, context, "elm.title");

	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	/*elm_object_part_content_set(layout, "elm.swallow.text", label);*/

	elm_object_content_set(popup, layout);
	evas_object_show(popup);


	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, func, param);
	return popup;
}

