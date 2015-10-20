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
#include "mf-ug-winset.h"
#include "mf-ug-widget.h"
#include "mf-ug-util.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-resource.h"
#include "mf-ug-cb.h"
#include "mf-ug-db-handle.h"
#include "mf-ug-list-play.h"
#include "mf-ug-file-util.h"

#define MF_LIST_THUMBNAIL_SIZE		72

#define MF_UG_SELECTED_ITEM_BRING_UP_COUNT	7
#define MF_UG_SELECTED_ITEM_BRING_UP_COUNT_MAX	8

#define MF_UG_LIST_PLAY_TIME_OUT	0.01

#define MF_UG_GENLIST_REALIZE_ITEM_COUNT	10

static Elm_Object_Item *default_item = NULL;
static int global_radio_max = 0;
bool g_ug_bDefaultItem = false;
void mf_ug_main_sound_title_in_idle(void *data);
int __mf_update_sound_title(void *app_data);

void mf_ug_radio_max_set(int value)
{
	global_radio_max = value;
}

int mf_ug_radio_max_get()
{
	return global_radio_max;
}

Elm_Object_Item *mf_ug_genlist_default_item_get()
{
	return default_item;
}

void mf_ug_genlist_default_item_set(Elm_Object_Item *item)
{
	default_item = item;

}

/******************************
** Prototype    : __mf_ug_genlist_is_file_marked
** Description  : Samsung
** Input        : Eina_List *source
**                GString *path
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
static gboolean __mf_ug_genlist_is_file_marked(Eina_List *source, GString *path)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(source == NULL, false, "source is NULL");
	ug_mf_retvm_if(path == NULL, false, "path is NULL");

	bool flag = false;
	Eina_List *l = NULL;
	void *data = NULL;

	EINA_LIST_FOREACH(source, l, data) {
		char *source_path = strdup(data);
		if (source_path != NULL) {
			if (strcmp(source_path, path->str) == 0) {
				flag = true;
				free(source_path);
				source_path = NULL;
				break;
			} else {
				free(source_path);
				source_path = NULL;
			}
		}
	}
	UG_TRACE_END;
	return flag;
}


/******************************
** Prototype    : __mf_ug_genlist_set_marked_path
** Description  : Samsung
** Input        : Eina_List **source
**                GString *path
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
static void __mf_ug_genlist_set_marked_path(Eina_List **source, GString *path)
{
	UG_TRACE_BEGIN;
	bool file_exist_flag = false;
	char *source_path;
	Eina_List *l = NULL;
	void *data = NULL;

	ug_mf_retm_if(source == NULL, "source is NULL");
	ug_mf_retm_if(path == NULL, "path is NULL");

	if (NULL == *source || eina_list_count(*source) == 0) {
		*source = eina_list_append(*source, strdup(path->str));
	} else {

		EINA_LIST_FOREACH(*source, l, data) {
			source_path = (char *)data;
			if (source_path != NULL) {
				if (strcmp(source_path, path->str) == 0) {
					file_exist_flag = true;
					break;
				}
			}
		}
		if (file_exist_flag == true) {
			*source = eina_list_remove(*source, source_path);
		} else {
			*source = eina_list_append(*source, strdup(path->str));
		}
	}
	UG_TRACE_END;
}


/******************************
** Prototype    : mf_ug_genlist_create_checkbox
** Description  : Samsung
** Input        : Evas_Object *parent
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
Evas_Object *mf_ug_genlist_create_checkbox(Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	Evas_Object *ck_box = NULL;

	ck_box = elm_check_add(parent);
	elm_object_style_set(ck_box, "default");
	/*evas_object_size_hint_weight_set(ck_box, 1.0, 1.0);
	evas_object_size_hint_align_set(ck_box, -1.0, 0.5);*/
	elm_check_state_set(ck_box, 0);
	evas_object_show(ck_box);
	evas_object_repeat_events_set(ck_box, 0);
	UG_TRACE_END;
	return ck_box;
}

void mf_ug_genlist_show_select_info(void *data)
{
	UG_TRACE_BEGIN;

	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	int iDirCount = 0, iFileCount = 0;
	Elm_Object_Item *it = NULL;
	ugListItemData *itemData = NULL;

	Evas_Object *pGenlist = ugd->ug_MainWindow.ug_pNaviGenlist;

	it = elm_genlist_first_item_get(pGenlist);
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->ug_bChecked) {
			if (mf_is_dir(itemData->ug_pItemName->str)) {
				iDirCount++;
			} else {
				iFileCount++;
			}
		}

		it = elm_genlist_item_next_get(it);
	}
	int count = 0;
	count = iFileCount;
	if (count > 0) {
		char *label = NULL;
		label = g_strdup_printf(mf_ug_widget_get_text(MF_UG_LABEL_SELECTED), count);
		elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "elm.text.title", label);
		UG_SAFE_FREE_CHAR(label);
	} else {
		if (mf_ug_util_is_import_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
			mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_IMPORT_CHAP, "elm.text.title");
		} else if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE || ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
			mf_ug_widget_object_item_text_set(ugd->ug_MainWindow.ug_pNaviItem, MF_UG_LABEL_DOCUMENTS, "elm.text.title");
		} else {

			/*ug_error("ugd->ug_UiGadget.ug_iSelectMode is [%d]", ugd->ug_UiGadget.ug_iSelectMode);*/
			if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE ||
					ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE || ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE) {
				elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "elm.text.title", mf_file_get(ugd->ug_Status.ug_pPath->str));
			} else {
				char *label = NULL;
				label = g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_SELECT_ITEMS));
				elm_object_item_part_text_set(ugd->ug_MainWindow.ug_pNaviItem, "elm.text.title", label);
				UG_SAFE_FREE_CHAR(label);
			}
		}
	}
	UG_TRACE_END;

}

/******************************
** Prototype    : __mf_ug_genlist_checkbox_cb
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
static void __mf_ug_genlist_checkbox_cb(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");
	ugData *ugd = (ugData *)itemData->ug_pData;

	if (itemData->ug_bChecked == false) {
		itemData->ug_bChecked = true;
	} else {
		itemData->ug_bChecked = false;
	}

	if (itemData->ug_pData->ug_UiGadget.ug_iMarkedMode == MARKED_ON) {
		__mf_ug_genlist_set_marked_path(&itemData->ug_pData->ug_UiGadget.ug_pMultiSelectFileList, itemData->ug_pItemName);
	}

	struct stat stFileInfo;
	if (itemData->ug_bChecked) {
		ugd->ug_Status.ug_iCheckedCount++;
		stat(itemData->ug_pItemName->str, &stFileInfo);
		if (ugd->limitsize > 0 && ugd->selsize + stFileInfo.st_size > ugd->limitsize) {
			char *noti = NULL;
			noti = mf_ug_widget_get_text(MF_UG_LABEL_MAXIMUM_SIZE);
			mf_ug_popup_indicator_popup(noti);
			if (itemData->ug_bChecked == 0) {
				itemData->ug_bChecked = 1;
			} else {
				itemData->ug_bChecked = 0;
			}
			elm_check_state_set(itemData->ug_pCheckBox, itemData->ug_bChecked);
			UG_SAFE_FREE_CHAR(noti);
			return;
		}
		ugd->selsize += stFileInfo.st_size;
	} else {
		ugd->ug_Status.ug_iCheckedCount--;
		stat(itemData->ug_pItemName->str, &stFileInfo);
		ugd->selsize = ugd->selsize - stFileInfo.st_size;
	}
	if (ugd->ug_Status.ug_iCheckedCount > ugd->ug_UiGadget.ug_iMaxLength) {
			char *reach_string = mf_ug_widget_get_text(MF_UG_LABEL_REACH_MAX_SHARE_COUNT);
			char *max_string = g_strdup_printf(reach_string, ugd->ug_UiGadget.ug_iMaxLength);
			ug_error("max_string is [%s]", max_string);
			/*UG_SAFE_FREE_CHAR(reach_string);*/
			mf_ug_popup_indicator_popup(max_string);
			UG_SAFE_FREE_CHAR(max_string);
			if (itemData->ug_bChecked == 0) {
				itemData->ug_bChecked = 1;
			} else {
				itemData->ug_bChecked = 0;
			}
			if (itemData->ug_bChecked) {
				ugd->ug_Status.ug_iCheckedCount++;
			} else {
				ugd->ug_Status.ug_iCheckedCount--;
			}
			elm_check_state_set(itemData->ug_pCheckBox, itemData->ug_bChecked);
			return;
	}
	ugd->ug_Status.ug_iTotalCount = eina_list_count(ugd->ug_UiGadget.ug_pFilterList);
	if (ugd->ug_Status.ug_iTotalCount == ugd->ug_Status.ug_iCheckedCount) {
		ugd->ug_Status.ug_bSelectAllChecked = EINA_TRUE;
	} else {
		ugd->ug_Status.ug_bSelectAllChecked = EINA_FALSE;
	}
	if (ugd->ug_Status.ug_bSelectAllChecked) {
		elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, 1);
	} else {
		elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, 0);
	}

	ug_error("=========  ugd->ug_Status.ug_iCheckedCount is [%d]", ugd->ug_Status.ug_iCheckedCount);

	mf_ug_genlist_show_select_info(ugd);
	mf_ug_navi_bar_set_ctrl_item_disable(itemData->ug_pData);

	UG_TRACE_END;
}


/******************************
** Prototype    : __mf_ug_genlist_create_radio_box
** Description  : Samsung
** Input        : Evas_Object *parent
**                ugListItemData *params
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
static Evas_Object *__mf_ug_genlist_create_radio_box(Evas_Object *parent, ugListItemData *params)
{
	UG_TRACE_BEGIN;

	ugListItemData *itemData = (ugListItemData *)params;
	ug_mf_retvm_if(itemData == NULL, NULL, "itemData is NULL");

	ugData *ugd = (ugData *)(itemData->ug_pData);
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");

	Evas_Object *radio_box = NULL;
	radio_box = elm_radio_add(parent);
	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		elm_object_style_set(radio_box, "silent");
	}
	elm_radio_state_value_set(radio_box, itemData->ug_iGroupValue);
	elm_radio_group_add(radio_box, ugd->ug_MainWindow.ug_pRadioGroup);
	elm_radio_value_set(ugd->ug_MainWindow.ug_pRadioGroup, ugd->ug_Status.ug_iRadioOn);
	UG_TRACE_END;
	return radio_box;
}

#if 0   /* Currently not used */
static bool __mf_ug_list_play_timer_cb(void *data)
{
	ugListItemData *item_data = (ugListItemData *)data;
	ugData *ugd = (ugData *)item_data->ug_pData;

	ugd->ug_ListPlay.hiden_flag = false;

	if (ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PLAYING || ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PAUSED) {
		if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, item_data->ug_pItemName->str) == 0) {
			mf_ug_list_play_destory_playing_file(ugd);
			ugd->ug_ListPlay.play_data = NULL;
			UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
		}
	}
	mf_ug_cb_list_play_cb(item_data, NULL, NULL);
	item_data->ug_pData->ug_Status.play_timer = NULL;
	ugd->ug_ListPlay.play_data = item_data->ug_pItem;
	elm_genlist_select_mode_set(ugd->ug_MainWindow.ug_pNaviGenlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	return ECORE_CALLBACK_CANCEL;
}
#endif

/******************************
** Prototype    : __mf_ug_genlist_radio_box_cb
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
static void __mf_ug_genlist_radio_box_cb(void *data, Evas_Object *obj, void *event_info)
{

	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retm_if(itemData == NULL, "ugListItemData is NULL");
	ugData *ugd = (ugData *)itemData->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	itemData->ug_bChecked = true;
	itemData->ug_pData->ug_Status.ug_iRadioOn = itemData->ug_iGroupValue;
	elm_radio_value_set(itemData->ug_pData->ug_MainWindow.ug_pRadioGroup, itemData->ug_iGroupValue);
	mf_ug_navi_bar_set_ctrl_item_disable(itemData->ug_pData);
	/*
	mf_ug_launch_type launch_type = LAUNCH_TYPE_UNSUPPORT;
	if (mf_ug_is_default_ringtone(ugd, itemData->ug_pItemName->str)) {
		launch_type = LAUNCH_TYPE_MUSIC;
	} else {
		launch_type = mf_ug_util_get_file_launch_type(itemData->ug_pItemName->str);
	}
	if (launch_type == LAUNCH_TYPE_MUSIC) {
		if (ugd->ug_Status.play_timer == NULL) {
			ug_error(">>>>>>>>>>>>>> entry list play");
			ugd->ug_Status.play_timer = ecore_timer_add(MF_UG_LIST_PLAY_TIME_OUT, (Ecore_Task_Cb)__mf_ug_list_play_timer_cb, itemData);
		} else {
			ug_error(">>>>>>>>>>>>>> Faild entry list play");

		}
	}
	*/
	UG_TRACE_END;
}


/******************************
** Prototype    : __mf_ug_genlist_get_gl_label
** Description  :
** Input        : const void *data
**                Evas_Object *obj
**                const char *part
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
static char *__mf_ug_genlist_get_gl_label(void *data, Evas_Object *obj, const char *part)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retvm_if(itemData == NULL, NULL, "itemData is NULL");
	if (strcmp(part, "elm.text.main.left") == 0) {
		UG_TRACE_END;
		if (g_strcmp0(itemData->ug_pItemName->str, PHONE_FOLDER) == 0) {
			return g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_PHONE));
		} else if (g_strcmp0(itemData->ug_pItemName->str, MEMORY_FOLDER) == 0) {
			return g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_MMC));
		} else {
			if (itemData->ug_pData->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
				if (g_strcmp0(MF_UG_LABEL_DEFAULT_RINGTONE, itemData->ug_pItemName->str) == 0) {
					if (itemData->ug_pData->ug_UiGadget.domain && 0 == g_strcmp0(itemData->ug_pData->ug_UiGadget.domain, MESSAGE)
						&& itemData->ug_pData->ug_Status.ug_launch_path && 0 == g_strcmp0(itemData->ug_pData->ug_Status.ug_launch_path, UG_SETTING_MSG_ALERTS_PATH)) {/*just for message-> more -> Notificaion sound*/
						return g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_DEFAULT_NOTIFICATION_SOUND));
					} else {
						return g_strdup(mf_ug_widget_get_text(itemData->ug_pItemName->str));
					}
				} else if (g_strcmp0(MF_UG_LABEL_SILENT, itemData->ug_pItemName->str) == 0) {
					return g_strdup(mf_ug_widget_get_text(itemData->ug_pItemName->str));
				}
				if (!itemData->sound_title) {
					/*char *fullpath = NULL;
					if (g_strcmp0(MF_UG_LABEL_DEFAULT_RINGTONE, itemData->ug_pItemName->str)==0) {
						fullpath = itemData->ug_pData->ug_UiGadget.default_ringtone;
					} else {
						fullpath = itemData->ug_pItemName->str;
					}
					itemData->sound_title = mf_ug_file_attr_sound_title_get(fullpath);*/
				}
				if (itemData->sound_title) {
					return g_strdup(itemData->sound_title);
				} else {
#if 0
					char *title = mf_ug_file_attr_sound_title_get(itemData->ug_pItemName->str);
					if (title) {
						return title;
					} else
#endif
					{
						char *filename = (char *)mf_file_get((const char *)itemData->ug_pItemName->str);
						char *newFileName = g_strdup(filename);
						char *newFileName1 = newFileName;
						for (; *newFileName1 != '\0'; newFileName1++) {
							if (*newFileName1 == '_')
								*newFileName1 = ' ';
						   }
						char *file_strip_ext = mf_strip_ext(newFileName);
						if (itemData->ug_bDefaultItem == true && file_strip_ext != NULL && g_str_has_prefix(file_strip_ext, "Alarm ")) {/*Fixed the plm bug(P140620-05330 ).*/
							int len = strlen(file_strip_ext);
							char *file_strip_ext1 = (char *)calloc(1, len);
							
							if (file_strip_ext1 != NULL) {
								strncpy(file_strip_ext1, &file_strip_ext[6], len);
								g_free(file_strip_ext);
								file_strip_ext = file_strip_ext1;
							}
						}
						g_free(newFileName);
						return file_strip_ext;
					}
				}
				/*UG_TRACE_END;*/
			}
			return strdup(mf_file_get(itemData->ug_pItemName->str));
		}
	} else if (strcmp(part, "elm.uptitle.text") == 0) {
		UG_TRACE_END;
		return strdup(("Upper Level"));
	} else {
		UG_TRACE_END;
		return strdup("");
	}
}


/******************************
** Prototype    : __mf_ug_genlist_get_thumbnail
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
static void __mf_ug_genlist_get_thumbnail(void *data)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");

	const char *dir_icon_path = UG_ICON_FOLDER;
	/*const char *dir_create_icon_path = UG_ICON_FOLDER_CREATED;*/
	char *icon_path = NULL;
	int error_code = MYFILE_ERR_NONE;

	if (itemData->ug_pThumbPath == NULL) {
		if (mf_is_dir(itemData->ug_pItemName->str)) {
			if (g_strcmp0(itemData->ug_pItemName->str, PHONE_FOLDER) == 0) {
				itemData->ug_pThumbPath = strdup(UG_ICON_ITEM_ROOT_PHONE);
				itemData->ug_bRealThumbFlag = true;
			} else if (g_strcmp0(itemData->ug_pItemName->str, MEMORY_FOLDER) == 0) {
				itemData->ug_pThumbPath = strdup(UG_ICON_ITEM_ROOT_MMC);
				itemData->ug_bRealThumbFlag = true;
			} else {
				itemData->ug_pThumbPath = strdup(dir_icon_path);
				itemData->ug_bRealThumbFlag = true;
			}
		} else {
			if (itemData->media) {
				if (itemData->thumbnail_create == EINA_TRUE) {
					media_info_cancel_thumbnail(itemData->media);
					itemData->thumbnail_create = EINA_FALSE;
				}
				media_info_destroy(itemData->media);
				itemData->media = NULL;
			}
			mf_ug_file_attr_get_file_icon(itemData->ug_pItemName->str, &error_code, &icon_path, &itemData->media);
			if (icon_path) {
				itemData->ug_pThumbPath = strdup(icon_path);
				itemData->ug_bRealThumbFlag = true;
				free(icon_path);
				icon_path = NULL;
			} else {	/*/must be image/video file */
				mf_ug_fs_file_type type = UG_FILE_TYPE_NONE;
				type = mf_ug_file_attr_get_file_type_by_mime(itemData->ug_pItemName->str);

				if (type == UG_FILE_TYPE_VIDEO) {
					itemData->ug_pThumbPath = strdup(UG_ICON_VIDEO);
				} else if (type == UG_FILE_TYPE_IMAGE) {
					itemData->ug_pThumbPath = strdup(UG_ICON_IMAGE);
				} else {
					itemData->ug_pThumbPath = strdup(UG_DEFAULT_ICON);
				}
				itemData->ug_bRealThumbFlag = false;
				error_code = MYFILE_ERR_GET_THUMBNAIL_FAILED;
			}
		}
	} else {
		if (strncmp(itemData->ug_pThumbPath, MF_IMAGE_HEAD, strlen(MF_IMAGE_HEAD)) == 0) {
			/*do nothing;*/
		} else {
			if (mf_file_exists(itemData->ug_pThumbPath)) {
				if (itemData->ug_bRealThumbFlag == false) {
					error_code = MYFILE_ERR_GET_THUMBNAIL_FAILED;
				}
			} else {
				free(itemData->ug_pThumbPath);
				itemData->ug_pThumbPath = NULL;
				/*/set default icon */
				itemData->ug_pThumbPath = strdup(UG_DEFAULT_ICON);
				itemData->ug_bRealThumbFlag = false;
				error_code = MYFILE_ERR_GET_THUMBNAIL_FAILED;
			}
		}
	}
	if (error_code != 0) {
		if (itemData->thumbnail_create == EINA_FALSE) {
			mf_ug_cb_create_thumbnail(itemData, mf_ug_cb_thumb_created_cb);
			itemData->thumbnail_create = EINA_TRUE;
		}
	}
	UG_TRACE_END;
}


/******************************
** Prototype    : __mf_ug_genlist_create_thumbnail
** Description  : Samsung
** Input        : void *data
**                Evas_Object *parent
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
static Evas_Object *__mf_ug_genlist_create_thumbnail(void *data, Evas_Object *obj)
{

	UG_TRACE_BEGIN;
	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retvm_if(listItemData == NULL, NULL, "listItemData is NULL");
	ug_mf_retvm_if(obj == NULL, NULL, "obj is NULL");

	Evas_Object *layout = NULL;
	Evas_Object *thumb = NULL;

	layout = elm_layout_add(obj);
	elm_layout_file_set(layout, UG_EDJ_NAVIGATIONBAR, UG_GRP_LIST);
	evas_object_repeat_events_set(layout, EINA_TRUE);

	thumb = elm_image_add(layout);
	elm_image_prescale_set(thumb, MF_LIST_THUMBNAIL_SIZE);
	elm_image_fill_outside_set(thumb, EINA_TRUE);
	elm_image_smooth_set(thumb, EINA_FALSE);

	if (listItemData->ug_pThumbPath && strncmp(listItemData->ug_pThumbPath, MF_IMAGE_HEAD, strlen(MF_IMAGE_HEAD)) == 0) {
		elm_image_file_set(thumb, UG_EDJ_IMAGE, listItemData->ug_pThumbPath); /*Todo*/
	} else {
		elm_image_file_set(thumb, listItemData->ug_pThumbPath, NULL); /*Todo*/
	}

	elm_object_part_content_set(layout, "thumbnail", thumb);
	if (!mf_ug_file_attr_is_dir(listItemData->ug_pItemName->str)) {
		mf_ug_fs_file_type type = UG_FILE_TYPE_NONE;
		type = mf_ug_file_attr_get_file_type_by_mime(listItemData->ug_pItemName->str);
		if (type == UG_FILE_TYPE_VIDEO) {
			elm_object_signal_emit(layout, "elm.video.show", "elm");
		}
	}
	UG_TRACE_END;
	return layout;
}

/******************************
** Prototype    : __mf_ug_genlist_add_checkbox
** Description  : Samsung
** Input        : void *data
**                Evas_Object *parent
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
static Evas_Object *__mf_ug_genlist_add_checkbox(void *data, Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retvm_if(itemData == NULL, NULL, "itemData is NULL");

	Evas_Object *check_box = mf_ug_genlist_create_checkbox(parent);
	evas_object_propagate_events_set(check_box, 0);

	elm_check_state_set(check_box, itemData->ug_bChecked);

	itemData->ug_pCheckBox = check_box;
	evas_object_smart_callback_add(check_box, "changed", __mf_ug_genlist_checkbox_cb, itemData);

	UG_TRACE_END;
	return check_box;
}

static void _myfile_popup_change_category_radio_toggle_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	__mf_ug_genlist_radio_box_cb(data, obj, NULL);
}

/******************************
** Prototype    : __mf_ug_genlist_add_radio_box
** Description  : Samsung
** Input        : void *data
**                Evas_Object *parent
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
static Evas_Object *__mf_ug_genlist_add_radio_box(void *data, Evas_Object *parent)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retvm_if(itemData == NULL, NULL, "itemData is NULL");

	Evas_Object *radio_box = __mf_ug_genlist_create_radio_box(parent, itemData);

	evas_object_propagate_events_set(radio_box, 0);
	itemData->ug_pRadioBox = radio_box;
/*	evas_object_smart_callback_add(radio_box, "changed", __mf_ug_genlist_radio_box_cb, itemData);*/
	elm_object_signal_callback_add(radio_box, "elm,action,radio,toggle", "",
			_myfile_popup_change_category_radio_toggle_cb,
						       itemData);

	UG_TRACE_END;
	return radio_box;
}

/******************************
** Prototype    : __mf_ug_genlist_get_gl_icon
** Description  : Samsung
** Input        : const void *data
**                Evas_Object *obj
**                const char *part
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
/*static Evas_Object *__mf_ug_genlist_ringtone_play_icon_create(void *data, Evas_Object *obj)
{
	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retv_if(listItemData == NULL, NULL);
	ugData *ugd = listItemData->ug_pData;
	ug_mf_retv_if(ugd == NULL, NULL);
	Evas_Object *music_icon = NULL;
	Evas_Object *music_button = NULL;
	if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, listItemData->ug_pItemName->str) == 0) {
		const char *play_icon = NULL;
		char *pause_icon = NULL;

		music_button = elm_button_add(obj);
		//elm_object_style_set(music_button, "circle/empty");
		elm_object_style_set(music_button, "myfile_play");
		music_icon = elm_image_add(music_button);
		if (ugd->ug_ListPlay.ug_pPlayFilePath) {
			if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, listItemData->ug_pItemName->str) == 0) {
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
		evas_object_size_hint_min_set(music_icon, 45, 45);
		evas_object_size_hint_aspect_set(music_icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
		elm_object_part_content_set(music_button, "icon", music_icon);
		evas_object_propagate_events_set(music_button, EINA_FALSE);
		evas_object_smart_callback_add(music_button, "clicked", (Evas_Smart_Cb)mf_ug_cb_list_play_cb, (void*)listItemData);
		evas_object_smart_callback_add(music_button, "pressed", mf_ug_cb_play_button_pressed_cb, ugd);
		evas_object_smart_callback_add(music_button, "unpressed", mf_ug_cb_play_button_unpressed_cb, ugd);
		listItemData->ug_pPlaybtn = music_button;
		UG_TRACE_END;
	}
	return music_button;
}*/

/*static Evas_Object *__mf_ug_genlist_normal_play_icon_create(void *data, Evas_Object *obj)
{
	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retv_if(listItemData == NULL, NULL);
	ugData *ugd = listItemData->ug_pData;
	ug_mf_retv_if(ugd == NULL, NULL);
	Evas_Object *music_icon = NULL;
	Evas_Object *music_button = NULL;

	const char *play_icon = NULL;
	char *pause_icon = NULL;

	music_button = elm_button_add(obj);
	//elm_object_style_set(music_button, "circle/empty");
	elm_object_style_set(music_button, "myfile_play");
	music_icon = elm_image_add(music_button);
	if (ugd->ug_ListPlay.ug_pPlayFilePath) {
		if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, listItemData->ug_pItemName->str) == 0) {
			ug_error("===================");
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
	evas_object_size_hint_min_set(music_icon, 45, 45);
	evas_object_size_hint_aspect_set(music_icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
	elm_object_part_content_set(music_button, "icon", music_icon);
	evas_object_propagate_events_set(music_button, EINA_FALSE);
	evas_object_smart_callback_add(music_button, "clicked", (Evas_Smart_Cb)mf_ug_cb_list_play_cb, (void*)listItemData);
	//evas_object_smart_callback_add(music_button, "pressed", mf_ug_cb_play_button_pressed_cb, ugd);
	//evas_object_smart_callback_add(music_button, "unpressed", mf_ug_cb_play_button_unpressed_cb, ugd);
	listItemData->ug_pPlaybtn = music_button;
	UG_TRACE_END;
	return music_button;
}*/

#if 0   /* Currently not used */
static Evas_Object *__mf_ug_genlist_play_icon_create(void *data, Evas_Object *obj)
{
	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retv_if(listItemData == NULL, NULL);
	ugData *ugd = listItemData->ug_pData;
	ug_mf_retv_if(ugd == NULL, NULL);

	Evas_Object *music_button = NULL;
	if (!mf_is_dir(listItemData->ug_pItemName->str)) {
		mf_ug_launch_type launch_type = LAUNCH_TYPE_UNSUPPORT;
		if (mf_ug_is_default_ringtone(ugd, listItemData->ug_pItemName->str)) {
			launch_type = LAUNCH_TYPE_MUSIC;
		} else {
			launch_type = mf_ug_util_get_file_launch_type(listItemData->ug_pItemName->str);
		}
		SECURE_DEBUG("item name is [%s]", listItemData->ug_pItemName->str);

		if (ugd->ug_ListPlay.hiden_flag == true) {
			return NULL;
		}
		if (launch_type == LAUNCH_TYPE_MUSIC) {
			if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
				music_button = __mf_ug_genlist_ringtone_play_icon_create(data, obj);
			} else {
				music_button = __mf_ug_genlist_normal_play_icon_create(data, obj);
			}
		}
	}
	return music_button;

}
#endif

static Evas_Object *__mf_ug_genlist_delete_get_gl_icon(void *data, Evas_Object *obj, const char *part)
{
	UG_TRACE_BEGIN;
	Evas_Object *selected_box = NULL;

	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retv_if(listItemData == NULL, NULL);
	ugData *ugd = listItemData->ug_pData;
	ug_mf_retv_if(ugd == NULL, NULL);

	if (!strcmp(part, "elm.icon.2")) {
		Evas_Object *content = elm_layout_add(obj);
		elm_layout_theme_set(content, "layout",
				     "list/C/type.2", "default");

		selected_box = __mf_ug_genlist_add_checkbox(listItemData, obj);
		evas_object_propagate_events_set(selected_box, EINA_FALSE);

		elm_layout_content_set(content, "elm.swallow.content", selected_box);
		return content;
	} else {
		UG_TRACE_END;
		return NULL;
	}
}

static Evas_Object *__mf_ug_genlist_get_gl_icon(void *data, Evas_Object *obj, const char *part)
{
	UG_TRACE_BEGIN;
	Evas_Object *thumb = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *selected_box = NULL;

	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retv_if(listItemData == NULL, NULL);
	ugData *ugd = listItemData->ug_pData;
	ug_mf_retv_if(ugd == NULL, NULL);

	if (!strcmp(part, "elm.icon.1")) {
		__mf_ug_genlist_get_thumbnail(listItemData);
		layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "layout",
				     "list/B/type.2", "default");
		thumb = __mf_ug_genlist_create_thumbnail(listItemData, obj);
		UG_TRACE_END;
		elm_layout_content_set(layout, "elm.swallow.content", thumb);
		return layout;
	} else if (!strcmp(part, "elm.icon.2")) {
		if (ugd->ug_UiGadget.ug_pMultiSelectFileList) {
			/*if (__mf_ug_genlist_is_file_marked(ugd->ug_UiGadget.ug_pMultiSelectFileList, listItemData->ug_pItemName)) {
				listItemData->ug_bChecked = true;
			}*/
		}
		if (mf_is_dir(listItemData->ug_pItemName->str)) {
			if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE) {
				selected_box = __mf_ug_genlist_add_checkbox(listItemData, obj);
			} else if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE) {
				selected_box = __mf_ug_genlist_add_radio_box(listItemData, obj);
			}
		} else {
			if (mf_ug_util_is_multi_select_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
				printf("============================== checkbox\n");
				selected_box = __mf_ug_genlist_add_checkbox(listItemData, obj);
				printf("==============================checkbox[%p] listItemData is [%s]\n", selected_box, listItemData->ug_pItemName->str);
			} else if (mf_ug_util_is_single_select_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
				printf("==============================radiobox\n");
				selected_box = __mf_ug_genlist_add_radio_box(listItemData, obj);
				printf("==============================radiobox[%p] listItemData is [%s]\n", selected_box, listItemData->ug_pItemName->str);
			}
		}
		evas_object_propagate_events_set(selected_box, EINA_FALSE);
		layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "layout",
				     "list/C/type.2", "default");
		elm_layout_content_set(layout, "elm.swallow.content", selected_box);
		return layout;
	/* } else if (!strcmp(part, "elm.icon.2")) { */
	/* 	__mf_ug_genlist_get_thumbnail(listItemData); */
	/* 	thumb = __mf_ug_genlist_create_thumbnail(listItemData, obj); */
	/* 	return thumb; */
	} else if (!strcmp(part, "elm.icon.right")) {
		/*Evas_Object *play_btn = __mf_ug_genlist_play_icon_create(listItemData, obj);
		return play_btn;*/
		UG_TRACE_END;

		return NULL;

	} else {
		UG_TRACE_END;
		return NULL;
	}
}

static Evas_Object *__mf_ug_genlist_ringtone_get_gl_icon(void *data, Evas_Object *obj, const char *part)
{
	UG_TRACE_BEGIN;
	Evas_Object *selected_box = NULL;

	ugListItemData *listItemData = (ugListItemData *)data;
	ug_mf_retv_if(listItemData == NULL, NULL);
	ugData *ugd = listItemData->ug_pData;
	ug_mf_retv_if(ugd == NULL, NULL);

	if (!strcmp(part, "elm.icon.2")) {
		if (mf_is_dir(listItemData->ug_pItemName->str)) {
			if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE) {
				selected_box = __mf_ug_genlist_add_checkbox(listItemData, obj);
			} else if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE) {
				selected_box = __mf_ug_genlist_add_radio_box(listItemData, obj);
			}
		} else {
			if (mf_ug_util_is_multi_select_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
				selected_box = __mf_ug_genlist_add_checkbox(listItemData, obj);
			} else if (mf_ug_util_is_single_select_mode(ugd->ug_UiGadget.ug_iSelectMode)) {
				selected_box = __mf_ug_genlist_add_radio_box(listItemData, obj);
			}
		}
		evas_object_propagate_events_set(selected_box, EINA_FALSE);
		Evas_Object *layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "layout",
				     "list/C/type.2", "default");
		elm_layout_content_set(layout, "elm.swallow.content", selected_box);
		return layout;
	} else if (!strcmp(part, "elm.icon.right")) {
#ifdef LIST_PLAY_SUPPORT
		if (!mf_is_dir(listItemData->ug_pItemName->str)) {
			Evas_Object *music_icon = NULL;
			Evas_Object *music_button = NULL;

			mf_ug_launch_type launch_type = LAUNCH_TYPE_UNSUPPORT;
			if (mf_ug_is_default_ringtone(ugd, listItemData->ug_pItemName->str)) {
				launch_type = LAUNCH_TYPE_MUSIC;
			} else {
				launch_type = mf_ug_util_get_file_launch_type(listItemData->ug_pItemName->str);
			}

			if (ugd->ug_ListPlay.hiden_flag == true) {
				return NULL;
			}
			if (launch_type == LAUNCH_TYPE_MUSIC) {
				if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, listItemData->ug_pItemName->str) == 0) {
					const char *play_icon = NULL;
					char *pause_icon = NULL;

					music_button = elm_button_add(obj);
					/*elm_object_style_set(music_button, "circle/empty");*/
					elm_object_style_set(music_button, "myfile_play");
					music_icon = elm_image_add(music_button);
					if (ugd->ug_ListPlay.ug_pPlayFilePath) {
						if (g_strcmp0(ugd->ug_ListPlay.ug_pPlayFilePath, listItemData->ug_pItemName->str) == 0) {
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
					evas_object_size_hint_aspect_set(music_icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
					elm_object_part_content_set(music_button, "icon", music_icon);
					evas_object_propagate_events_set(music_button, EINA_FALSE);
					evas_object_smart_callback_add(music_button, "clicked", (Evas_Smart_Cb)mf_ug_cb_list_play_cb, (void *)listItemData);
					evas_object_smart_callback_add(music_button, "pressed", mf_ug_cb_play_button_pressed_cb, ugd);
					evas_object_smart_callback_add(music_button, "unpressed", mf_ug_cb_play_button_unpressed_cb, ugd);
					listItemData->ug_pPlaybtn = music_button;
					UG_TRACE_END;
					return music_button;
				}
			}
		}
#endif
		UG_TRACE_END;

		return NULL;

	} else {
		UG_TRACE_END;
		return NULL;
	}
}

/******************************
** Prototype    : __mf_ug_genlist_get_gl_state
** Description  :
** Input        : const void *data
**                Evas_Object *obj
**                const char *part
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
static Eina_Bool __mf_ug_genlist_get_gl_state(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}


/******************************
** Prototype    : __mf_ug_genlist_del_gl
** Description  :
** Input        : const void *data
**                Evas_Object *obj
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
static void __mf_ug_genlist_del_gl(void *data, Evas_Object *obj)
{
	UG_TRACE_BEGIN;
	ugListItemData *itemData = (ugListItemData *)data;
	ug_mf_retm_if(itemData == NULL, "itemData is NULL");
	if (itemData->ug_pItemName && itemData->ug_pItemName->str) {
		g_string_free(itemData->ug_pItemName, TRUE);
		itemData->ug_pItemName = NULL;
	}
	if (itemData->ug_pThumbPath) {
		free(itemData->ug_pThumbPath);
		itemData->ug_pThumbPath = NULL;
	}
	if (itemData->media) {
		if (itemData->thumbnail_create == EINA_TRUE) {
			media_info_cancel_thumbnail(itemData->media);
		}
		media_info_destroy(itemData->media);
		itemData->media = NULL;
	}
	free(itemData);
	itemData = NULL;
	UG_TRACE_END;
	return;
}



/******************************
** Prototype    : mf_ug_genlist_selected_gl
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
void mf_ug_genlist_sel(void *data)
{
	UG_TRACE_BEGIN;
	Elm_Object_Item *item = (Elm_Object_Item *) data;
	ugListItemData *selected = (ugListItemData *)elm_object_item_data_get(item);
	ugData *ugd = (ugData *)selected->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/
	ug_mf_retm_if(ugd->ug == NULL, "ugd->ug is NULL");/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	{
		fprintf(stdout, "selected text %s\n", (char *)selected->ug_pItemName->str);
		elm_genlist_item_selected_set(item, EINA_FALSE);
		if (ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE) {
			ug_error();
			char *result = NULL;
			app_control_h app_control = NULL;
			result = g_strdup(selected->ug_pItemName->str);
			if (result) {
				int ret = 0;
				ret = app_control_create(&app_control);
				if (ret == APP_CONTROL_ERROR_NONE) {
					ug_error();
					app_control_add_extra_data(app_control, "result", result);
					ug_send_result(ugd->ug, app_control);
					app_control_destroy(app_control);
				}
				ug_debug("result is [%s]", result);
				UG_SAFE_FREE_CHAR(result);
				ug_destroy_me(ugd->ug);
				ugd->ug = NULL;
			}
			return;
		}
		if (!mf_is_dir(selected->ug_pItemName->str)) {

			if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE) {
				return;
			}
			if (mf_ug_util_is_single_select_mode(ugd->ug_UiGadget.ug_iSelectMode)) {

				if (elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup) != selected->ug_iGroupValue) {
					ugd->ug_Status.ug_iRadioOn = selected->ug_iGroupValue;
					selected->ug_bChecked = true;
					elm_radio_value_set(ugd->ug_MainWindow.ug_pRadioGroup, selected->ug_iGroupValue);
				}
				/*
				mf_ug_launch_type launch_type = LAUNCH_TYPE_UNSUPPORT;
				if (mf_ug_is_default_ringtone(ugd, selected->ug_pItemName->str)) {
					launch_type = LAUNCH_TYPE_MUSIC;
				} else {
					launch_type = mf_ug_util_get_file_launch_type(selected->ug_pItemName->str);
				}
				if (launch_type == LAUNCH_TYPE_MUSIC) {
					//elm_genlist_select_mode_set(ugd->ug_MainWindow.ug_pNaviGenlist, ELM_OBJECT_SELECT_MODE_NONE);
					if (ugd->ug_ListPlay.play_data) {
						ugd->ug_ListPlay.hiden_flag = true;
						elm_genlist_item_fields_update(ugd->ug_ListPlay.play_data, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
					}
					if (ugd->ug_Status.play_timer == NULL) {
						ug_error(">>>>>>>>>>>>>> entry list play");
						if (ugd->show) {
							ecore_idler_del(ugd->show);
							ugd->show = NULL;
						}
						ugd->ug_Status.play_timer = ecore_timer_add(MF_UG_LIST_PLAY_TIME_OUT, (Ecore_Task_Cb)__mf_ug_list_play_timer_cb, selected);
					} else {
						ug_error(">>>>>>>>>>>>>> Faild entry list play");

					}
				}
				*/
			} else {
				/*
				mf_ug_launch_type launch_type = LAUNCH_TYPE_UNSUPPORT;
				launch_type = mf_ug_util_get_file_launch_type(selected->ug_pItemName->str);
				if (launch_type == LAUNCH_TYPE_MUSIC) {
					//P131128-07671,Messages > Compose > Attach > Audio > click audio item , music should not play
					//mf_ug_cb_list_play_cb(selected, NULL, NULL);
				}
				*/
				if (selected->ug_bChecked == 0) {
					selected->ug_bChecked = 1;
				} else {
					selected->ug_bChecked = 0;
				}

				struct stat stFileInfo;
				stat(selected->ug_pItemName->str, &stFileInfo);
				if (selected->ug_bChecked) {
					ugd->ug_Status.ug_iCheckedCount++;
					if (ugd->limitsize > 0 && ugd->selsize + stFileInfo.st_size > ugd->limitsize) {
						char *noti = NULL;
						noti = mf_ug_widget_get_text(MF_UG_LABEL_MAXIMUM_SIZE);
						mf_ug_popup_indicator_popup(noti);
						if (selected->ug_bChecked == 0) {
							selected->ug_bChecked = 1;
						} else {
							selected->ug_bChecked = 0;
						}
						UG_SAFE_FREE_CHAR(noti);
						return;
					}
					ugd->selsize += stFileInfo.st_size;
				} else {
					ugd->ug_Status.ug_iCheckedCount--;
					ugd->selsize -= stFileInfo.st_size;
				}
				ug_error("=========  ugd->ug_Status.ug_iCheckedCount is [%d]", ugd->ug_Status.ug_iCheckedCount);
				if (ugd->ug_Status.ug_iCheckedCount > ugd->ug_UiGadget.ug_iMaxLength) {
					ug_error();
					char *reach_string = mf_ug_widget_get_text(MF_UG_LABEL_REACH_MAX_SHARE_COUNT);
					char *max_string = g_strdup_printf(reach_string, ugd->ug_UiGadget.ug_iMaxLength);
					/*UG_SAFE_FREE_CHAR(reach_string);*/
					ug_error("max_string is [%s]", max_string);
					mf_ug_popup_indicator_popup(max_string);
					UG_SAFE_FREE_CHAR(max_string);
					if (selected->ug_bChecked == 0) {
						selected->ug_bChecked = 1;
					} else {
						selected->ug_bChecked = 0;
					}
					if (selected->ug_bChecked) {
						ugd->ug_Status.ug_iCheckedCount++;
					} else {
						ugd->ug_Status.ug_iCheckedCount--;
					}
					return;
				} else {
					ug_error();
					elm_check_state_set(selected->ug_pCheckBox, selected->ug_bChecked);
				}
				ugd->ug_Status.ug_iTotalCount = eina_list_count(ugd->ug_UiGadget.ug_pFilterList);
				if (ugd->ug_Status.ug_iTotalCount == ugd->ug_Status.ug_iCheckedCount) {
					ugd->ug_Status.ug_bSelectAllChecked = EINA_TRUE;
				} else {
					ugd->ug_Status.ug_bSelectAllChecked = EINA_FALSE;
				}
				if(ugd->ug_Status.ug_bSelectAllChecked) {
					elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, 1);
				} else {
					elm_check_state_set(ugd->ug_MainWindow.ug_pSelectAllCheckBox, 0);
				}
				mf_ug_genlist_show_select_info(ugd);
			}
		} else if (mf_is_dir(selected->ug_pItemName->str)) {
			if (ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PLAYING || ugd->ug_ListPlay.ug_iPlayState == PLAY_STATE_PAUSED) {
				mf_ug_list_play_destory_playing_file(ugd);
				ugd->ug_ListPlay.play_data = NULL;
				UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
			}
			if (ugd->ug_Status.ug_pPath) {
				g_string_free(ugd->ug_Status.ug_pPath, TRUE);
				ugd->ug_Status.ug_pPath = NULL;
			}
			ug_error(">>>>>>>>>>>>>>>>>  selected->ug_pItemName->str is [%s]", selected->ug_pItemName->str);
			ugd->ug_Status.ug_pPath = g_string_new(selected->ug_pItemName->str);

			if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
				ugd->ug_Status.ug_iViewType = mf_ug_view_normal;
				mf_ug_navi_bar_create_default_view(ugd);
				mf_ug_util_path_push(ugd->ug_Status.ug_pPath->str, ugd->ug_Status.ug_iViewType);
			} else {
				mf_ug_navi_bar_create_default_view(ugd);
				mf_ug_util_path_push(ugd->ug_Status.ug_pPath->str, ugd->ug_Status.ug_iViewType);
			}
		}

		mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	}
	UG_TRACE_END;
}
void mf_ug_genlist_selected_gl(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item != NULL) {
	{
		ug_error();
		mf_ug_genlist_sel(item);
	}
	}
	UG_TRACE_END;
}

static void __mf_ug_genlist_lang_changed(void *data, Evas_Object *obj, void *ei)
{
	UG_TRACE_BEGIN
	ug_mf_retm_if(obj == NULL, "obj is NULL");
	elm_genlist_realized_items_update(obj);
	UG_TRACE_END
}

/******************************
** Prototype    : __mf_ug_genlist_create_gl
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
void mf_ug_genlist_delete_sel(void *data)
{
	UG_TRACE_BEGIN;
	Elm_Object_Item *item = (Elm_Object_Item *) data;
	ugListItemData *selected = (ugListItemData *)elm_object_item_data_get(item);
	ugData *ugd = (ugData *)selected->ug_pData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/
	ug_mf_retm_if(ugd->ug == NULL, "ugd->ug is NULL");/*Fixed the P131011-01548 by jian12.li, sometimes, if the ug is extised, we still send the result to other app.*/

	{
		fprintf(stdout, "selected text %s\n", (char *)selected->ug_pItemName->str);
		elm_genlist_item_selected_set(item, EINA_FALSE);
		if (!mf_is_dir(selected->ug_pItemName->str)) {

			if (selected->ug_bChecked == 0) {
				selected->ug_bChecked = 1;
			} else {
				selected->ug_bChecked = 0;
			}
			elm_check_state_set(selected->ug_pCheckBox, selected->ug_bChecked);

			if (selected->ug_bChecked) {
				ugd->ug_Status.ug_iCheckedCount++;
			} else {
				ugd->ug_Status.ug_iCheckedCount--;
			}

			if (ugd->ug_Status.ug_iTotalCount == ugd->ug_Status.ug_iCheckedCount) {
				ugd->ug_Status.ug_bSelectAllChecked = EINA_TRUE;
			} else {
				ugd->ug_Status.ug_bSelectAllChecked = EINA_FALSE;
			}
			mf_ug_genlist_show_select_info(ugd);
		}

		mf_ug_navi_bar_set_ctrl_item_disable(ugd);
	}
	UG_TRACE_END;
}

void mf_ug_genlist_delete_selected_gl(void *data, Evas_Object *obj, void *event_info)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	mf_ug_genlist_delete_sel(item);
	UG_TRACE_END;
}

Evas_Object *mf_ug_genlist_delete_style_create(void *data)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");


	Evas_Object *genlist;
	genlist = elm_genlist_add(ugd->ug_MainWindow.ug_pNaviBar);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_smart_callback_add(genlist, "selected", mf_ug_genlist_delete_selected_gl, ugd);
	evas_object_smart_callback_add(genlist, "language,changed", __mf_ug_genlist_lang_changed, ugd);

	ugd->ug_Status.ug_1text2icon4_itc.item_style = "1line";
	ugd->ug_Status.ug_1text2icon4_itc.func.text_get = __mf_ug_genlist_get_gl_label;
	ugd->ug_Status.ug_1text2icon4_itc.func.content_get = __mf_ug_genlist_delete_get_gl_icon;
	ugd->ug_Status.ug_1text2icon4_itc.func.state_get = __mf_ug_genlist_get_gl_state;
	ugd->ug_Status.ug_1text2icon4_itc.func.del = __mf_ug_genlist_del_gl;

	UG_TRACE_END;

	return genlist;
}

Evas_Object *__mf_ug_genlist_create_gl(void *data)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");

	Evas_Object *genlist;
	genlist = elm_genlist_add(ugd->ug_MainWindow.ug_pNaviBar);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_smart_callback_add(genlist, "selected", mf_ug_genlist_selected_gl, ugd);
	evas_object_smart_callback_add(genlist, "language,changed", __mf_ug_genlist_lang_changed, ugd);

	ugd->ug_Status.ug_1text3icon_itc.item_style = "1line";
	ugd->ug_Status.ug_1text3icon_itc.func.text_get = __mf_ug_genlist_get_gl_label;
	ugd->ug_Status.ug_1text3icon_itc.func.content_get = __mf_ug_genlist_get_gl_icon;
	ugd->ug_Status.ug_1text3icon_itc.func.state_get = __mf_ug_genlist_get_gl_state;
	ugd->ug_Status.ug_1text3icon_itc.func.del = __mf_ug_genlist_del_gl;

	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		/*ugd->ug_Status.ug_1text2icon4_itc.item_style = "1text.2icon.4";
		ugd->ug_Status.ug_1text2icon4_itc.item_style = "myfile.1text.2icon.6";*/
		ugd->ug_Status.ug_1text2icon4_itc.item_style = "1line";
		ugd->ug_Status.ug_1text2icon4_itc.func.text_get = __mf_ug_genlist_get_gl_label;
		ugd->ug_Status.ug_1text2icon4_itc.func.content_get = __mf_ug_genlist_ringtone_get_gl_icon;
		ugd->ug_Status.ug_1text2icon4_itc.func.state_get = __mf_ug_genlist_get_gl_state;
		ugd->ug_Status.ug_1text2icon4_itc.func.del = __mf_ug_genlist_del_gl;
	}
	ugd->ug_Status.ug_1text2icon_itc.item_style = "1line";
	ugd->ug_Status.ug_1text2icon_itc.func.text_get = __mf_ug_genlist_get_gl_label;
	ugd->ug_Status.ug_1text2icon_itc.func.content_get = __mf_ug_genlist_get_gl_icon;
	ugd->ug_Status.ug_1text2icon_itc.func.state_get = __mf_ug_genlist_get_gl_state;
	ugd->ug_Status.ug_1text2icon_itc.func.del = __mf_ug_genlist_del_gl;
	if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE ||
	    ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE ||
	    ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE ||
	    ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE ||
	    ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE ||
	    ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE ||
	    ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE ||
	    ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE
	    ) {
		ugd->ug_Status.ug_1text1icon_itc.item_style = "1line";
		ugd->ug_Status.ug_1text1icon_itc.func.text_get = __mf_ug_genlist_get_gl_label;
		ugd->ug_Status.ug_1text1icon_itc.func.content_get = __mf_ug_genlist_get_gl_icon;
		ugd->ug_Status.ug_1text1icon_itc.func.state_get = __mf_ug_genlist_get_gl_state;
		ugd->ug_Status.ug_1text1icon_itc.func.del = __mf_ug_genlist_del_gl;
	}

	UG_TRACE_END;

	return genlist;
}


/******************************
** Prototype    : __mf_ug_genlist_init_item_data
** Description  : Samsung
** Input        : void *data
**                ugListItemData **itemData
**                char *fullname
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
static void __mf_ug_genlist_init_item_data(void *data, ugListItemData **itemData, char *fullname)
{

	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ugListItemData **ug_ItemData = itemData;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(ug_ItemData == NULL, "ug_ItemData is NULL");

	*ug_ItemData = (ugListItemData *)malloc(sizeof(ugListItemData));
	if (*ug_ItemData == NULL) {
		ug_debug("ug_ItemData malloc failed");
	} else {
		memset(*ug_ItemData, 0, sizeof(ugListItemData));
		if (fullname) {
			(*ug_ItemData)->ug_pItemName = g_string_new(fullname);
		} else {
			(*ug_ItemData)->ug_pItemName = NULL;
		}
		(*ug_ItemData)->ug_bChecked = false;
		(*ug_ItemData)->ug_pRadioBox = NULL;
		(*ug_ItemData)->ug_pCheckBox = NULL;
		(*ug_ItemData)->ug_pPlaybtn = NULL;
		(*ug_ItemData)->ug_iGroupValue = 0;
		(*ug_ItemData)->ug_pItem = NULL;
		(*ug_ItemData)->ug_pThumbPath = NULL;
		(*ug_ItemData)->ug_bRealThumbFlag = false;
		(*ug_ItemData)->ug_pData = ugd;
	}
	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_genlist_create_content_list_view
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
Elm_Object_Item *mf_ug_genlist_default_ringtone_item_append(Evas_Object *parent,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc)
{
	ug_mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ug_mf_retvm_if(itc == NULL, NULL, "itc is NULL");

	ugData *ugd = (ugData *)data;
	ugListItemData *ug_ItemData = NULL;
	Elm_Object_Item *it = NULL;
	char *real_name = NULL;

	real_name = g_strdup(MF_UG_LABEL_DEFAULT_RINGTONE);
	__mf_ug_genlist_init_item_data(ugd, &ug_ItemData, real_name);
	if (ug_ItemData == NULL) {
		ug_debug("alloc memory error\n");
		if (real_name) {
			free(real_name);
			real_name = NULL;
		}
		return NULL;
	}
	ug_ItemData->ug_pThumbPath = g_strdup(UG_ICON_MUSIC);
	ug_ItemData->ug_bRealThumbFlag = true;

	ug_ItemData->ug_iGroupValue = groudValue;
	if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
		if (__mf_ug_genlist_is_file_marked(ugd->ug_UiGadget.ug_pMultiSelectFileList, ug_ItemData->ug_pItemName)) {
			ug_ItemData->ug_bChecked = true;
			ugd->ug_Status.ug_iRadioOn = ug_ItemData->ug_iGroupValue;
		}
	}
	it = elm_genlist_item_append(parent, itc, ug_ItemData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	ug_ItemData->ug_pItem = it;
	mf_ug_genlist_default_item_set(it);

	if (real_name) {
		free(real_name);
		real_name = NULL;
	}

	return  it;
}

Elm_Object_Item *mf_ug_genlist_silent_item_append(Evas_Object *parent,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc)
{
	ug_mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ug_mf_retvm_if(itc == NULL, NULL, "itc is NULL");

	ugData *ugd = (ugData *)data;
	ugListItemData *ug_ItemData = NULL;
	Elm_Object_Item *it = NULL;
	char *real_name = NULL;

	real_name = g_strdup(MF_UG_LABEL_SILENT);
	__mf_ug_genlist_init_item_data(ugd, &ug_ItemData, real_name);
	if (ug_ItemData == NULL) {
		ug_debug("alloc memory error\n");
		if (real_name) {
			free(real_name);
			real_name = NULL;
		}
		return NULL;
	}
	ug_ItemData->ug_pThumbPath = g_strdup(UG_ICON_MUSIC);
	ug_ItemData->ug_bRealThumbFlag = true;

	ug_ItemData->ug_iGroupValue = groudValue;
	if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
		if (__mf_ug_genlist_is_file_marked(ugd->ug_UiGadget.ug_pMultiSelectFileList, ug_ItemData->ug_pItemName)) {
			ug_ItemData->ug_bChecked = true;
			ugd->ug_Status.ug_iRadioOn = ug_ItemData->ug_iGroupValue;
		}
	}
	it = elm_genlist_item_append(parent, itc, ug_ItemData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	ug_ItemData->ug_pItem = it;

	if (real_name) {
		free(real_name);
		real_name = NULL;
	}

	return  it;
}

Elm_Object_Item *mf_ug_genlist_item_append(Evas_Object *parent,
				      char *real_name,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc)
{
	ug_mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	ug_mf_retvm_if(real_name == NULL, NULL, "real_name is NULL");
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ug_mf_retvm_if(itc == NULL, NULL, "itc is NULL");

	ugData *ugd = (ugData *)data;
	ugListItemData *ug_ItemData = NULL;
	Elm_Object_Item *it = NULL;

	__mf_ug_genlist_init_item_data(ugd, &ug_ItemData, real_name);
	if (ug_ItemData == NULL) {
		ug_debug("alloc memory error\n");
		return NULL;
	}
	ug_ItemData->ug_bDefaultItem = g_ug_bDefaultItem;

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		if (g_strcmp0(real_name, PHONE_FOLDER) == 0) {
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_FOLDER);
			ug_ItemData->ug_bRealThumbFlag = true;

		} else if (g_strcmp0(real_name, MEMORY_FOLDER) == 0) {
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_ITEM_MMC);
			ug_ItemData->ug_bRealThumbFlag = true;
		}
	}
	ug_ItemData->storage_type = mf_ug_fm_svc_wapper_get_location(real_name);

	ug_ItemData->ug_iGroupValue = groudValue;
	if (ugd->ug_Status.ug_iViewType != mf_ug_view_ringtone_del) {
		if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
			if (__mf_ug_genlist_is_file_marked(ugd->ug_UiGadget.ug_pMultiSelectFileList, ug_ItemData->ug_pItemName)) {
				ug_ItemData->ug_bChecked = true;
				ugd->ug_Status.ug_iRadioOn = ug_ItemData->ug_iGroupValue;
			}
		}
	} else {
		ug_ItemData->ug_bChecked = false;
	}
	it = elm_genlist_item_append(parent, itc, ug_ItemData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	ug_ItemData->ug_pItem = it;
	return  it;
}

Elm_Object_Item *mf_ug_genlist_item_prepend(Evas_Object *parent,
				      char *real_name,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc)
{
	ug_mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	ug_mf_retvm_if(real_name == NULL, NULL, "real_name is NULL");
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ug_mf_retvm_if(itc == NULL, NULL, "itc is NULL");

	ugData *ugd = (ugData *)data;
	ugListItemData *ug_ItemData = NULL;
	Elm_Object_Item *it = NULL;

	__mf_ug_genlist_init_item_data(ugd, &ug_ItemData, real_name);
	if (ug_ItemData == NULL) {
		ug_debug("alloc memory error\n");
		return NULL;
	}

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		if (g_strcmp0(real_name, PHONE_FOLDER) == 0) {
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_FOLDER);
			ug_ItemData->ug_bRealThumbFlag = true;

		} else if (g_strcmp0(real_name, MEMORY_FOLDER) == 0) {
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_ITEM_MMC);
			ug_ItemData->ug_bRealThumbFlag = true;
		}
	}

	ug_ItemData->ug_iGroupValue = groudValue;
	ug_ItemData->ug_bChecked = true;
	ugd->ug_Status.ug_iRadioOn = ug_ItemData->ug_iGroupValue;
	it = elm_genlist_item_prepend(parent, itc, ug_ItemData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	ug_ItemData->ug_pItem = it;
	return  it;
}

Elm_Object_Item *mf_ug_genlist_item_insert(Evas_Object *parent,
				      char *real_name,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc,
				      Elm_Object_Item *after_item)
{
	ug_mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	ug_mf_retvm_if(real_name == NULL, NULL, "real_name is NULL");
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ug_mf_retvm_if(itc == NULL, NULL, "itc is NULL");

	ugData *ugd = (ugData *)data;
	ugListItemData *ug_ItemData = NULL;
	Elm_Object_Item *it = NULL;

	__mf_ug_genlist_init_item_data(ugd, &ug_ItemData, real_name);
	if (ug_ItemData == NULL) {
		ug_debug("alloc memory error\n");
		return NULL;
	}

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		if (g_strcmp0(real_name, PHONE_FOLDER) == 0) {
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_FOLDER);
			ug_ItemData->ug_bRealThumbFlag = true;

		} else if (g_strcmp0(real_name, MEMORY_FOLDER) == 0) {
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_ITEM_MMC);
			ug_ItemData->ug_bRealThumbFlag = true;
		}
	}

	ug_ItemData->ug_iGroupValue = groudValue;
	ug_ItemData->ug_bChecked = true;
	ugd->ug_Status.ug_iRadioOn = ug_ItemData->ug_iGroupValue;
	it = elm_genlist_item_insert_after(parent, itc, ug_ItemData, NULL, after_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	ug_ItemData->ug_pItem = it;
	return  it;
}

void mf_ug_genlist_shortcuts_append(void *data, Evas_Object *parent, Elm_Genlist_Item_Class *itc)
{
	ugData *ugd = (ugData *)data;

	Eina_List *shortcut = NULL;
	mf_ug_db_handle_get_shortcut_files(&shortcut);
	if (shortcut) {
		Eina_List *l = NULL;
		char *filename = NULL;
		EINA_LIST_FOREACH(shortcut, l, filename) {
			ugListItemData *ug_ItemData = NULL;
			Elm_Object_Item *it = NULL;
			__mf_ug_genlist_init_item_data(ugd, &ug_ItemData, filename);
			ug_ItemData->ug_pThumbPath = strdup(UG_ICON_ITEM_SHORTCUT);
			ug_ItemData->ug_bRealThumbFlag = true;
			it = elm_genlist_item_append(parent, itc, ug_ItemData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			ug_ItemData->ug_pItem = it;

		}

	}
}

void mf_ug_genlist_delete_ringtone_items_add(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		Eina_List *ringtone_list = NULL;
		if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
			mf_ug_db_handle_get_ringtone_files(&ringtone_list);
		} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
			mf_ug_db_handle_get_alert_files(&ringtone_list);
		}

		Eina_List *l = NULL;
		char *filename = NULL;
		EINA_LIST_FOREACH(ringtone_list, l, filename) {
			if (filename) {
				mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, filename, ugd, 0, &ugd->ug_Status.ug_1text2icon4_itc);
			}

		}
	}
}
int mf_ug_genlist_ringtone_delete_items_add(void *data, int value)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, -1, "ugd is NULL");
	int groupValue = value;

	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		Eina_List *ringtone_list = NULL;
		if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
			mf_ug_db_handle_get_ringtone_files(&ringtone_list);
		} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
			mf_ug_db_handle_get_alert_files(&ringtone_list);
		}

		Eina_List *l = NULL;
		char *filename = NULL;
		EINA_LIST_FOREACH(ringtone_list, l, filename) {
				if (filename) {
					/*P131205-01044 by wangyan , if setted ringtone in db , do not add it in delete genlist to avoid to be deleted*/
					if (g_strcmp0(filename, ugd->ug_Status.mark_mode) != 0) {
						mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, filename, ugd, groupValue, &ugd->ug_Status.ug_1text2icon4_itc);
						groupValue++;
					}
				}
			}
	}
	return groupValue;
}

int mf_ug_genlist_ringtone_items_add(void *data, int value)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, -1, "ugd is NULL");
	int groupValue = value;

	if (ugd->ug_UiGadget.ug_iSoundMode != mf_ug_sound_mode_none) {
		Eina_List *ringtone_list = NULL;
		if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_ringtone) {
			mf_ug_db_handle_get_ringtone_files(&ringtone_list);
		} else if (ugd->ug_UiGadget.ug_iSoundMode == mf_ug_sound_mode_alert) {
			mf_ug_db_handle_get_alert_files(&ringtone_list);
		}

		Eina_List *l = NULL;
		char *filename = NULL;
		EINA_LIST_FOREACH(ringtone_list, l, filename) {
			if (filename) {
				mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, filename, ugd, groupValue, &ugd->ug_Status.ug_1text2icon4_itc);
				groupValue++;
			}

		}
	}
	return groupValue;
}

void mf_ug_genlist_first_item_append(void *data, char *fullpath)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(fullpath == NULL, "fullpath is NULL");
	Evas_Object *genlist = ugd->ug_MainWindow.ug_pNaviGenlist;
	int value = mf_ug_radio_max_get();
	Elm_Object_Item *it = NULL;

	it = mf_ug_genlist_item_prepend(genlist, fullpath, ugd, value+1, &ugd->ug_Status.ug_1text2icon4_itc);
	mf_ug_radio_max_set(value+1);

	elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	UG_TRACE_END;
}

void mf_ug_genlist_first_item_insert(void *data, char *fullpath, Elm_Object_Item *insert_afer)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(fullpath == NULL, "fullpath is NULL");
	Evas_Object *genlist = ugd->ug_MainWindow.ug_pNaviGenlist;
	int value = mf_ug_radio_max_get();
	Elm_Object_Item *it = NULL;

	it = mf_ug_genlist_item_insert(genlist, fullpath, ugd, value+1, &ugd->ug_Status.ug_1text2icon4_itc, insert_afer);
	mf_ug_radio_max_set(value+1);

	elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	UG_TRACE_END;
}
/*To fix P131209-06058 wangyan*/
void mf_ug_genlist_item_bringin_top(void *data, const char *music_path)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");
	ug_mf_retm_if(music_path == NULL, "music_path is NULL");
	Elm_Object_Item *it = NULL;
	ugListItemData *itemData = NULL;
	it = elm_genlist_first_item_get(ugd->ug_MainWindow.ug_pNaviGenlist);
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->ug_pItemName == NULL || itemData->ug_pItemName->str == NULL) {
			continue;
		}
		if (g_strcmp0(music_path, itemData->ug_pItemName->str) == 0) {
			itemData->ug_bChecked = true;
			ugd->ug_Status.ug_iRadioOn = itemData->ug_iGroupValue;
			elm_radio_value_set(ugd->ug_MainWindow.ug_pRadioGroup, itemData->ug_iGroupValue);
			ug_debug("music file [%s] is brought to top", music_path);
			elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			break;
		}
		it = elm_genlist_item_next_get(it);
	}
	UG_TRACE_END;
}


Evas_Object *mf_ug_genlist_create_content_list_view(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugd is NULL");

	Evas_Object *genlist;
	int error_code = 0;
	Eina_List *dir_list = NULL;
	Eina_List *file_list = NULL;
	int groupValue = 1;
	int count = 0;
	unsigned int dir_list_len = 0;
	unsigned int filter_file_list_len = 0;
	ugd->ug_Status.ug_iRadioOn = 0;
	ugd->ug_Status.ug_bNoContentFlag = false;

	GString *fullpath = g_string_new(ugd->ug_Status.ug_pPath->str);

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root && ugd->ug_Status.ug_iMore != UG_MORE_SEARCH) {
		mf_ug_util_generate_root_view_file_list(&dir_list, ugd->ug_Status.ug_iMmcFlag);
		ugd->ug_UiGadget.ug_pDirList = dir_list;
		ugd->ug_UiGadget.ug_pFilterList = NULL;
		mf_ug_util_sort_the_file_list(ugd);
		dir_list_len = eina_list_count(dir_list);

	} else {
		error_code = mf_ug_fm_svc_wapper_get_file_list_by_filter(ugd, fullpath, &dir_list, &file_list);
		if (error_code == 0) {

			if (ugd->ug_UiGadget.ug_pDirList) {
				mf_ug_util_free_eina_list_data(&ugd->ug_UiGadget.ug_pDirList, NODE_TYPE_PNODE);
				ugd->ug_UiGadget.ug_pDirList = NULL;
			}
			if (ugd->ug_UiGadget.ug_pFilterList) {
				mf_ug_util_free_eina_list_data(&ugd->ug_UiGadget.ug_pFilterList, NODE_TYPE_PNODE);
				ugd->ug_UiGadget.ug_pFilterList = NULL;
			}

			ugd->ug_UiGadget.ug_pFilterList = file_list;
			ugd->ug_UiGadget.ug_pDirList = dir_list;
			mf_ug_util_sort_the_file_list(ugd);
			dir_list_len = eina_list_count(dir_list);
			filter_file_list_len = eina_list_count(ugd->ug_UiGadget.ug_pFilterList);
		}

	}

	/*      list option set */
	ug_debug("error_code is [%d]dir_list_len is [%d]file_list_len is [%d]\n", error_code, dir_list_len, filter_file_list_len);


	if ((dir_list_len == 0 && filter_file_list_len == 0)) {
		Evas_Object *nocontent = mf_ug_widget_nocontent_create(ugd->ug_MainWindow.ug_pMainLayout, MF_UG_LABEL_NO_RESULT, UG_ICON_MULTI_NO_CONTENTS);
		ugd->ug_Status.ug_bNoContentFlag = true;
		mf_ug_util_add_dir_watch(fullpath->str, ugd);
		g_string_free(fullpath, true);
		fullpath = NULL;
		return nocontent;
	}

	genlist = __mf_ug_genlist_create_gl(ugd);
	elm_genlist_block_count_set(genlist, MF_UG_GENLIST_REALIZE_ITEM_COUNT);

	ugd->ug_MainWindow.ug_pNaviGenlist = genlist;
	ugFsNodeInfo *pNode = NULL;
	char *real_name = NULL;
	Eina_List *l = NULL;

	EINA_LIST_FOREACH(ugd->ug_UiGadget.ug_pDirList, l, pNode) {
		if (pNode) {
			if (pNode->path && pNode->name) {
				real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
			}
		} else {
			continue;
		}
		count++;


		if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE ||
		    ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE ||
		    ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE ||
		    ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE ||
		    ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE ||
		    ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE ||
		    ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE ||
		    ugd->ug_UiGadget.ug_iSelectMode == DOCUMENT_SHARE) {
			mf_ug_genlist_item_append(genlist, real_name, ugd, 0, &ugd->ug_Status.ug_1text1icon_itc);
		} else {
			mf_ug_genlist_item_append(genlist, real_name, ugd, groupValue, &ugd->ug_Status.ug_1text3icon_itc);
			groupValue++;
		}
		UG_SAFE_FREE_CHAR(real_name);
	}

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root && ugd->ug_Status.ug_iMore != UG_MORE_SEARCH) {
		mf_ug_genlist_shortcuts_append(ugd, genlist, &ugd->ug_Status.ug_1text1icon_itc);
	}

	EINA_LIST_FOREACH(ugd->ug_UiGadget.ug_pFilterList, l, pNode) {
		if (pNode) {
			if (pNode->path && pNode->name) {
				real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
			}
		} else {
			continue;
		}
		count++;
		if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE) {
			mf_ug_genlist_item_append(genlist, real_name, ugd, groupValue, &ugd->ug_Status.ug_1text3icon_itc);
			groupValue++;
		} else if (ugd->ug_UiGadget.ug_iSelectMode == EXPORT_MODE || ugd->ug_UiGadget.ug_iSelectMode == SAVE_MODE) {
			mf_ug_genlist_item_append(genlist, real_name, ugd, 0, &ugd->ug_Status.ug_1text1icon_itc);
		} else {
			mf_ug_genlist_item_append(genlist, real_name, ugd, 0, &ugd->ug_Status.ug_1text3icon_itc);
		}
		UG_SAFE_FREE_CHAR(real_name);
	}

	if (fullpath != NULL) {
		if (ugd->ug_Status.ug_iViewType != mf_ug_view_root) {
			mf_ug_util_add_dir_watch(fullpath->str, ugd);
		}
		g_string_free(fullpath, TRUE);
	}
	mf_ug_radio_max_set(groupValue);
	UG_TRACE_END;
	return genlist;
}

static char *__get_title_gl_text(void *data, Evas_Object *obj, const char *part)
{
	if (data)
		return g_strdup((char *)data);
	else
		return NULL;
}
static void __del_title_gl(void *data, Evas_Object * obj)
{
	UG_SAFE_FREE_CHAR(data);
}

Evas_Object *mf_ug_genlist_create_path_info(Evas_Object *parent, Elm_Genlist_Item_Class *itc, char *info)
{
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *genlist = NULL;
	Elm_Object_Item *git = NULL;

	genlist = elm_genlist_add(parent);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	itc->item_style = "groupindex";
	itc->func.text_get = __get_title_gl_text;
	itc->func.del = __del_title_gl;

	git = elm_genlist_item_append(genlist, itc, info, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	return genlist;
}

void mf_ug_genlist_item_remove(Evas_Object *parent, int storage)
{
	ug_mf_retm_if(parent == NULL, "parent is NULL");

	ugListItemData *itemData = NULL;
	Elm_Object_Item *it = NULL;

	it = elm_genlist_first_item_get(parent);
	while (it) {
		itemData = elm_object_item_data_get(it);
		ug_debug("itemData->m_ItemName->str is [%s]", itemData->ug_pItemName->str);
		if (storage == itemData->storage_type) {
			Elm_Object_Item *temp_item = it;
			it = elm_genlist_item_next_get(it);
			elm_object_item_del(temp_item);
			continue;
		}

		it = elm_genlist_item_next_get(it);
	}
}


Elm_Object_Item *mf_ug_genlist_first_item_get(Evas_Object *genlist)
{
	int x = 300;
	int y = 220;
	int posret = 0;
	Elm_Object_Item *it = NULL;
	it = elm_genlist_at_xy_item_get(genlist, x, y, &posret);
	return it;
}
