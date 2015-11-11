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




#include <Eina.h>
#include "mf-ug-main.h"
#include "mf-ug-util.h"
#include "mf-ug-winset.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-resource.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-cb.h"
#include "mf-ug-widget.h"
#include <time.h>


#define MF_SEARCH_OPTION_DEF (MF_SEARCH_OPT_EXT)
#define MF_SEARCH_ROOT_NUM 3
#define MF_SEARCH_TIMER_INTERVAL 0.5

extern int flagSearchMsg;
extern pthread_mutex_t gLockSearchMsg;
extern pthread_cond_t gCondSearchMsg;

time_t searchtime_begin = 0;
time_t searchtime_end = 0;

typedef struct {
	char *size;
	char *create_date;
} mf_search_detail_infor_s;


void mf_ug_search_view_item_append(void *data, void *user_data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)user_data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	char *item_name = (char *)data;
	ug_mf_retm_if(item_name == NULL, "input item_name error");

	ugListItemData *m_TempItem = NULL;
	GString *search_path = g_string_new(item_name);


	g_string_free(search_path, TRUE);
	search_path = NULL;

	m_TempItem = (ugListItemData *) malloc(sizeof(ugListItemData));

	if (m_TempItem == NULL) {
		return;
	}
	memset(m_TempItem, 0, sizeof(ugListItemData));

	m_TempItem->ug_pItemName = g_string_new(item_name);
	m_TempItem->ug_iGroupValue = 0;
	m_TempItem->ug_pRadioBox = NULL;
	m_TempItem->ug_bChecked = FALSE;
	m_TempItem->ug_pCheckBox = NULL;
	m_TempItem->ug_pThumbPath = NULL;
	m_TempItem->ug_bRealThumbFlag = FALSE;
	m_TempItem->ug_pData = ugd;
	m_TempItem->ug_pItem = NULL;
	m_TempItem->storage_type = mf_ug_fm_svc_wapper_get_location(item_name);

	/*delete timer if exists*/

	Elm_Object_Item *it = NULL;

	if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE) {
		it = elm_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, &ugd->ug_Status.ug_1text1icon_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, ugd);
	} else if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE) {
		if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {
			static int GroupValue = 1;
			m_TempItem->ug_iGroupValue = GroupValue;
			GroupValue++;
		}
		it = elm_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, &ugd->ug_Status.ug_1text2icon_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, ugd);
	} else if (ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		it = elm_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, &ugd->ug_Status.ug_1text2icon_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, ugd);
	} else {
		it = elm_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, &ugd->ug_Status.ug_1text3icon_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, ugd);
	}
	m_TempItem->ug_pItem = it;
	UG_TRACE_END;

}


Evas_Object *mf_widget_create_select_all_layout(Evas_Object *parent)
{
	UG_TRACE_BEGIN

	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *select_all_ic = elm_image_add(parent);
	elm_image_file_set(select_all_ic, UG_EDJ_IMAGE, UG_TITLE_ICON_SELECT_ALL);
	elm_image_resizable_set(select_all_ic, EINA_TRUE, EINA_TRUE);
	evas_object_show(select_all_ic);

	Evas_Object *btn;

	btn = elm_button_add(parent);

	elm_object_style_set(btn, "naviframe/title_icon");

	if (select_all_ic) {
		elm_object_content_set(btn, select_all_ic);
	}

	evas_object_propagate_events_set(btn, EINA_FALSE);

	evas_object_show(btn);

	UG_TRACE_END

	return btn;

}

static void __mf_ug_search_list_item_append(void *data, void *user_data)
{
	Eina_List **list = (Eina_List **)user_data;
	if (data) {
		*list = eina_list_append(*list, g_strdup((char *)data));
	}
}
static void __mf_ug_search_view_result_cb(mf_search_result_t *result, void *user_data)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(result == NULL, "result is NULL");

	ugData *ugd = (ugData *)user_data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Evas_Object *playout = ugd->ug_MainWindow.ug_pNaviLayout;
	ug_mf_retm_if(playout == NULL, "get conformant failed");
	Evas_Object *newContent = NULL;
	Evas_Object *unUsed = elm_object_part_content_unset(playout, "part1");
	if (unUsed) {
		evas_object_del(unUsed);
		unUsed = NULL;
	}
	ugd->ug_UiGadget.ug_pSearchFileList = NULL;
	int total_count = g_list_length(result->dir_list) + g_list_length(result->file_list);
	if (total_count == 0) {
		ug_ecore_idler_del(ugd->ug_Status.popup_create_idler);
		newContent = mf_ug_widget_nocontent_create(playout, MF_UG_LABEL_NO_RESULT, UG_ICON_MULTI_NO_CONTENTS);
		elm_object_part_content_set(playout, "part1", newContent);
		mf_ug_navi_bar_button_set_disable(ugd, true);
	} else {



		/*newContent = mf_ug_genlist_create_content_list_view(ugd);*/
		newContent = __mf_ug_genlist_create_gl(ugd);
		ugd->ug_MainWindow.ug_pNaviGenlist = newContent;

		elm_object_part_content_set(playout, "part1", newContent);
		if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {
		} else {
			if (result->file_list != NULL) {
				/*Evas_Object *pSelectAllLayout = NULL;
				Evas_Object *pSelectAllCheckBox = NULL;*/
				ugd->ug_Status.ug_bSelectAllChecked = false;
				ugd->ug_Status.ug_iTotalCount = g_list_length(result->file_list);
				ugd->ug_Status.ug_iCheckedCount = 0;
				if (!ugd->ug_UiGadget.ug_MaxSetFlag || ugd->ug_Status.ug_iTotalCount <= ugd->ug_UiGadget.ug_iMaxLength) {
					/*pSelectAllLayout = mf_widget_create_select_all_layout(ugd->ug_MainWindow.ug_pNaviBox);
					ug_mf_retm_if(pSelectAllLayout == NULL, "pSelectAllLayout is NULL");
					ugd->ug_MainWindow.ug_pSelectAllLayout = pSelectAllLayout;
					evas_object_smart_callback_add(pSelectAllLayout, "clicked", mf_ug_select_all_layout_mouse_down_cb, ugd);
					ugd->ug_MainWindow.ug_pSelectAllCheckBox = pSelectAllCheckBox;

					evas_object_show(pSelectAllLayout);
					elm_object_item_part_content_set(ugd->ug_MainWindow.ug_pNaviItem, TITLE_RIGHT_BTN, pSelectAllLayout);*//*don't need the select all button.*/
				}

			}
		}
		if (result->dir_list != NULL) {
			g_list_foreach(result->dir_list, mf_ug_search_view_item_append, ugd);
		}
		if (result->file_list != NULL) {
			g_list_foreach(result->file_list, __mf_ug_search_list_item_append, &ugd->ug_UiGadget.ug_pSearchFileList);
			g_list_foreach(result->file_list, mf_ug_search_view_item_append, ugd);
		}
	}
	UG_TRACE_END;
}


static Eina_Bool __mf_ug_search_view_stop(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, ECORE_CALLBACK_CANCEL, "ugd is NULL");

	ugd->ug_MainWindow.ug_pSearchLabel = NULL;
	__mf_ug_search_view_result_cb(((ms_handle_t *) ugd->ug_Status.search_handler)->result, ugd);

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

	ugd->ug_Status.flagSearchStart = EINA_FALSE;

	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pSearchPopup);


	UG_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}

static void __mf_ug_search_view_stop_cb(void *data, Evas_Object *obj, void *event_info)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	/*P131202-00454 by wangyan [Fatal error] It pop up force close when tap searching popup cancel.*/
	UG_SAFE_DEL_ECORE_TIMER(ugd->ug_Status.pSearchTimer);
	__mf_ug_search_view_stop(ugd);

	UG_TRACE_END;
}


static int __mf_ug_search_view_idle_search_msg_cope_finished(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	if (ugd == NULL || ugd->ug_Status.msg_finish_idler == NULL) {
		return ECORE_CALLBACK_CANCEL;
	}
	pthread_mutex_lock(&gLockSearchMsg);
	if (flagSearchMsg == 0) {
		flagSearchMsg = 1;
		pthread_cond_signal(&gCondSearchMsg);
	}
	pthread_mutex_unlock(&gLockSearchMsg);
	ugd->ug_Status.msg_finish_idler = NULL;

	return ECORE_CALLBACK_CANCEL;
}
static int __mf_ug_search_result_show(mf_search_result_t *result)
{
	UG_TRACE_BEGIN;
	ugData *ugd = mf_ug_ugdata();
	if (ugd == NULL) {
		ug_debug("input ugd is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	ug_debug("get ugd is [%p]", ugd);
	UG_SAFE_FREE_OBJ(ugd->ug_MainWindow.ug_pSearchPopup);
	ugd->ug_MainWindow.ug_pSearchLabel = NULL;
	__mf_ug_search_view_result_cb((mf_search_result_t *) result, (void *)ugd);

	UG_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}
static void __mf_ug_search_view_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	if (ugd == NULL) {
		ug_debug("input ugd is NULL");
		return;
	}

	mf_search_pipe_msg *pSearchMsg = (mf_search_pipe_msg *) buffer;
	if (pSearchMsg == NULL) {
		ug_debug("received message is NULL");
		goto MF_CONTINURE_SEARCH;
	}

	if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_RESULT_REPORT) {
		ug_debug("result get");
	} else if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_ROOT_CHANGE) {
		ug_debug("root change ");

		/*char *new_desc = mf_ug_fm_svc_wrapper_translate_path(pSearchMsg->current_path);*/
		if (ugd->ug_MainWindow.ug_pSearchLabel) {
			/*elm_object_text_set(ugd->ug_MainWindow.ug_pSearchLabel, new_desc);*/
		}
		if (pSearchMsg->current_path) {
			free(pSearchMsg->current_path);
			pSearchMsg->current_path = NULL;
		}
		/*if (new_desc != NULL) {
			free(new_desc);
			new_desc = NULL;
		}*/
	} else if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_FINISHED) {
		ugd->ug_Status.flagSearchStart = EINA_FALSE;
		/*fix P131122-06150 by wangyan,[Contacts] Import from Device - Processing glimpse.
		To keep the processing popup showing not less then 2 seconds*/
		time(&searchtime_end);
		int delay = (int)(searchtime_end - searchtime_begin);
		ug_error("searchtime_end is [%d], searchtime_begin is [%d], delay is [%d]", searchtime_end, searchtime_begin, delay);
		delay = (delay >= 2) ? 0 : (2 - delay);
		ug_error("delay is [%d]", delay);

		UG_SAFE_DEL_ECORE_TIMER(ugd->ug_Status.pSearchTimer);
		ugd->ug_Status.pSearchTimer = ecore_timer_add(delay, (Ecore_Task_Cb)__mf_ug_search_result_show, (mf_search_result_t *) pSearchMsg->report_result);
	}

MF_CONTINURE_SEARCH:
	ug_ecore_idler_del(ugd->ug_Status.msg_finish_idler);
	ugd->ug_Status.msg_finish_idler = ecore_idler_add((Ecore_Task_Cb)__mf_ug_search_view_idle_search_msg_cope_finished, ugd);
	/*__mf_ug_search_view_idle_search_msg_cope_finished(NULL);*/
	UG_TRACE_END;
}

int mf_ug_search_item_type_get(char *path)
{
	char *ext = NULL;
	mf_ug_file_attr_get_file_ext(path, &ext);
	if (ext) {
		if (strcasecmp("DOCM", ext) == 0
		        || strcasecmp("XLT", ext) == 0
		        || strcasecmp("XLSM", ext) == 0) {
			UG_SAFE_FREE_CHAR(ext);
			return MF_SEARCH_CATEGORY_NONE;
		}
		UG_SAFE_FREE_CHAR(ext);
	}
	int type = mf_ug_file_attr_get_file_type_by_mime(path);
	if (type == UG_FILE_TYPE_GUL) {
		mf_ug_fs_file_type file_type = UG_FILE_TYPE_NONE;
		mf_ug_file_attr_get_file_category(path, &file_type);
		if (file_type == UG_FILE_TYPE_TXT) {
			type = UG_FILE_TYPE_TXT;
		}
	}
	int category = MF_SEARCH_CATEGORY_NONE;
	switch (type) {
	case UG_FILE_TYPE_IMAGE:
		category = MF_SEARCH_CATEGORY_IMAGE;
		break;
	case UG_FILE_TYPE_VIDEO:
	case UG_FILE_TYPE_MP4_VIDEO:
		category = MF_SEARCH_CATEGORY_VIDEO;
		break;
	case UG_FILE_TYPE_SOUND:
	case UG_FILE_TYPE_MUSIC:
	case UG_FILE_TYPE_MP4_AUDIO:
		category = MF_SEARCH_CATEGORY_SOUND;
		break;
	case UG_FILE_TYPE_DOC:
	case UG_FILE_TYPE_PDF:
	case UG_FILE_TYPE_PPT:
	case UG_FILE_TYPE_EXCEL:
	case UG_FILE_TYPE_TXT:
		category = MF_SEARCH_CATEGORY_DOCUMENT;
		break;
	case UG_FILE_TYPE_DIR:
		category = MF_SEARCH_CATEGORY_NONE;
		break;
	default:
		category = MF_SEARCH_CATEGORY_OTHERS;
		break;
	}
	return category;
}

static void __mf_ug_search_view_idle_search_start(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	char *text = NULL;
	if (ugd->ug_Status.flagSearchStart == EINA_TRUE) {
		return;
	} else {
		ugd->ug_Status.flagSearchStart = EINA_TRUE;
		time(&searchtime_begin);
	}

	if (ugd->ug_UiGadget.ug_pExtension) {
		text = g_strdup(ugd->ug_UiGadget.ug_pExtension);
	}

	int root_num = 0;
	const char *SearchRoot[MF_SEARCH_ROOT_NUM] = {0};
	ug_debug("ugd->ug_UiGadget.ug_iSelectMode is [%d]text is [%s] path is [%s]", ugd->ug_UiGadget.ug_iSelectMode, text, ugd->ug_Status.ug_pPath->str);
	if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {
		if (g_strcmp0(ugd->ug_Status.ug_pPath->str, PHONE_FOLDER) == 0) {
			ug_error("*********************");
			SearchRoot[root_num] = PHONE_FOLDER;
			root_num++;
		} else if (g_strcmp0(ugd->ug_Status.ug_pPath->str, MEMORY_FOLDER) == 0) {
			ug_error("*********************");
			SearchRoot[root_num] = MEMORY_FOLDER;
			root_num++;
		} else {
			ug_error("*********************");
			SearchRoot[root_num] = PHONE_FOLDER;
			root_num++;

			SearchRoot[root_num] = MEMORY_FOLDER;
			root_num++;
		}
	} else if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE
	           || ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE
	           || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		SearchRoot[root_num] = PHONE_FOLDER;
		root_num++;

		SearchRoot[root_num] = MEMORY_FOLDER;
		root_num++;
	} else {
		root_num = 1;
		SearchRoot[0] = ugd->ug_Status.ug_pPath->str;

	}
	/*const char *SearchRoot[MF_SEARCH_ROOT_NUM] = { ugd->ug_Status.ug_pPath->str};
	ug_debug("search path is [%s]", ugd->ug_Status.ug_pPath->str);*/
	/*Start Search routine*/
	if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
		if (text) {
			if (!mf_ug_search_start(ugd->ug_Status.search_handler, SearchRoot, root_num, \
			                        /*((text) ? text : NULL), MF_SEARCH_OPT_FILE, (void *)ugd, mf_ug_search_item_type_get, MF_SEARCH_CATEGORY_DOCUMENT)) {*/
			                        text, MF_SEARCH_OPT_MULTI_EXT, (void *)ugd, (mf_search_filter_cb)NULL, 0)) {
				ugd->ug_MainWindow.ug_pSearchPopup = mf_ug_popup_create_search(ugd, __mf_ug_search_view_stop_cb, ugd);
				ug_debug("start success");
			} else {
				ugd->ug_Status.flagSearchStart = EINA_FALSE;
				ug_debug("start failed");
			}
		} else {
			if (!mf_ug_search_start(ugd->ug_Status.search_handler, SearchRoot, root_num, \
			                        text, MF_SEARCH_OPT_FILE, (void *)ugd, (mf_search_filter_cb)mf_ug_search_item_type_get, MF_SEARCH_CATEGORY_DOCUMENT)) {
				ugd->ug_MainWindow.ug_pSearchPopup = mf_ug_popup_create_search(ugd, __mf_ug_search_view_stop_cb, ugd);
				ug_debug("start success");
			} else {
				ugd->ug_Status.flagSearchStart = EINA_FALSE;
				ug_debug("start failed");
			}
		}

	} else {
		if (!mf_ug_search_start(ugd->ug_Status.search_handler, SearchRoot, root_num, \
		                        ((text) ? text : NULL), MF_SEARCH_OPTION_DEF, (void *)ugd, (mf_search_filter_cb)NULL, 0)) {
			ug_debug("start success");
			/*generate the popup used to show search path
			**it's sure that new_desc is not NULL even if original path is NULL*/
			ugd->ug_MainWindow.ug_pSearchPopup = mf_ug_popup_create_search(ugd, __mf_ug_search_view_stop_cb, ugd);
		} else {
			ugd->ug_Status.flagSearchStart = EINA_FALSE;
			ug_debug("start failed");
		}

	}

	if (text != NULL) {
		free(text);
		text = NULL;
	}

}

void mf_ug_search_view_enter_search_routine(void *data, Evas_Object * obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	/*ugd->ug_Status.ug_iMore = UG_MORE_SEARCH;*/

	if (ugd->ug_Status.search_handler > 0) {
		mf_ug_search_finalize(&ugd->ug_Status.search_handler);
	}

	int ret = mf_ug_search_init(&ugd->ug_Status.search_handler);
	ug_mf_retm_if(ret < 0, "Fail to mf_ug_search_init()");

	if (ugd->ug_UiGadget.ug_pSyncPipe != NULL) {
		ecore_pipe_del(ugd->ug_UiGadget.ug_pSyncPipe);
		ugd->ug_UiGadget.ug_pSyncPipe = NULL;
	}
	ugd->ug_UiGadget.ug_pSyncPipe = ecore_pipe_add(__mf_ug_search_view_pipe_cb, ugd);

	if (ugd->ug_UiGadget.ug_pSyncPipe == NULL) {
		ug_debug("add pipe failed");
	}
	/*this is to init global variable to ensure the first message can be transmitted correctly*/
	/*flagSearchMsg is to indicate the condition wait to sync data of threads*/
	pthread_mutex_lock(&gLockSearchMsg);
	flagSearchMsg = 1;
	pthread_mutex_unlock(&gLockSearchMsg);

	/*delete guide text label in the box*/
	/*evas_object_del(elm_object_content_unset(pNavi_s->pNaviConform));*/
	__mf_ug_search_view_idle_search_start(ugd);

	UG_TRACE_END;
}

