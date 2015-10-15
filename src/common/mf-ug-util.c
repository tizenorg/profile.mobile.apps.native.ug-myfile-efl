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


#include <pthread.h>
#include "mf-ug-util.h"
#include "mf-ug-cb.h"
#include "mf-ug-winset.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-inotify-handle.h"
#include "mf-ug-resource.h"
#include "mf-ug-widget.h"
#include <system_settings.h>
#include "mf-ug-file-util.h"

#define MF_UG_TIMER_INTERVAL_VIBRATION 0.5
#define MF_UG_VIBRATION_DEVICE 0
#define MF_UG_VIBRATION_DURATION 500
#ifdef UG_OPERATION_SELECT_MODE
#define OPERATION_SEPERATOR		";"
#endif
#define NORMAL_SEPERATOR		"?"

static int __externalStorageId = 0;

bool getSupportedStorages_cb(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	UG_TRACE_BEGIN;

	if (type == STORAGE_TYPE_EXTERNAL) {
		__externalStorageId = storageId;
		UG_TRACE_END;
		return false;
	}

	return true;
}

/******************************
** Prototype    : ug_genlist_selected_state_get
** Description  :
** Input        : void *data
** Output       : bool
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
bool mf_ug_util_is_genlist_selected(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");

	Elm_Object_Item *gli = NULL;
	Elm_Object_Item *nli = NULL;
	Evas_Object *genlist = NULL;

	if (ugd->ug_MainWindow.ug_pNaviGenlist) {
		genlist = ugd->ug_MainWindow.ug_pNaviGenlist;
		gli = elm_genlist_first_item_get(genlist);

		while (gli) {
			ugListItemData *itemData = (ugListItemData *)elm_object_item_data_get(gli);
			ug_mf_retvm_if(itemData == NULL, false, "itemData is NULL");
			if (ugd->ug_Status.ug_iViewType == mf_ug_view_ringtone_del) {
				if (itemData->ug_bChecked == true) {
					UG_TRACE_END;
					return true;
				}
			} else if (ugd->ug_MainWindow.ug_pRadioGroup) {
				if (elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup) == 0) {
					UG_TRACE_END;
					return false;
				} else {
					if (elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup) == itemData->ug_iGroupValue) {
						UG_TRACE_END;
						return true;
					}
				}

			} else {
				if (itemData->ug_bChecked == true) {
					UG_TRACE_END;
					return true;
				}
			}
			nli = elm_genlist_item_next_get(gli);
			gli = nli;
		}
	}
	UG_TRACE_END;
	return false;
}

/******************************
** Prototype    : ug_parase_path_get
** Description  :
** Input        : GList *dest_list
**                char *path_list
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
void mf_ug_util_get_params_path(Eina_List **dest_list, const char *path_list)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(dest_list == NULL, "dest_list is NULL");

	gchar **result = NULL;
	gchar **params = NULL;
	result = g_strsplit(path_list, "?", 0);

	for (params = result; *params; params++) {
		*dest_list = eina_list_append(*dest_list, strdup(*params));
	}

	g_strfreev(result);
	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_util_free_eina_list_data
** Description  : Samsung
** Input        : Eina_List **list
**                mf_ug_eina_list_node_type node_type
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
void mf_ug_util_free_eina_list_data(Eina_List **list, mf_ug_eina_list_node_type node_type)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(list == NULL, "list is NULL");
	ug_mf_retm_if(*list == NULL, "*list is NULL");

	Eina_List *l = NULL;
	void *pNode = NULL;
	switch (node_type) {
	case NODE_TYPE_CHAR:
		EINA_LIST_FOREACH(*list, l, pNode) {
			free(pNode);
			pNode = NULL;
		}
		break;

	case NODE_TYPE_PNODE:
		EINA_LIST_FOREACH(*list, l, pNode) {
			ugFsNodeInfo *node = (ugFsNodeInfo *)pNode;
			if (node) {
				UG_SAFE_FREE_CHAR(node->path);
				UG_SAFE_FREE_CHAR(node->name);
				UG_SAFE_FREE_CHAR(node->ext);
				free(node);
				node = NULL;
			}
		}
		break;
	case NODE_TYPE_GSTRING:
		EINA_LIST_FOREACH(*list, l, pNode) {
			if (pNode != NULL) {
				g_string_free(pNode, TRUE);
				pNode = NULL;
			}
		}
		break;
	default:
		break;
	}

	eina_list_free(*list);
	*list = NULL;
	UG_TRACE_END;
}


/******************************
** Prototype    : __mf_ug_util_get_marked_selected_items
** Description  :
** Input        : GList *list
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
static void __mf_ug_util_get_marked_selected_items(Eina_List *list, Eina_List **select_list)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(select_list == NULL, "select_list is NULL");
	ug_mf_retm_if(list == NULL, "list is NULL");

	Eina_List *l = NULL;
	void *pNode = NULL;

	EINA_LIST_FOREACH(list, l, pNode) {
		if (pNode != NULL) {
			*select_list = eina_list_append(*select_list, strdup(pNode));
		}
	}
	UG_TRACE_END;

}

/******************************
** Prototype    : __mf_ug_util_get_marked_off_selected_items
** Description  :
** Input        : void *data
**                Evas_Object* content
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
static void __mf_ug_util_get_marked_off_selected_items(void *data, Eina_List **select_list)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugData is NULL");
	ug_mf_retm_if(select_list == NULL, "selected_list is NULL");
	ug_mf_retm_if(ugd->ug_MainWindow.ug_pNaviGenlist == NULL, "ugd->ug_MainWindow.ug_pNaviGenlist is NULL");


	Evas_Object *content = ugd->ug_MainWindow.ug_pNaviGenlist;
	Elm_Object_Item *gli = elm_genlist_first_item_get(content);
	Elm_Object_Item *nli = NULL;
	Eina_List *l = NULL;
	char *name = NULL;

	if (ugd->ug_Status.ug_bSelectAllChecked == true
		&& (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE
		|| ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE
		|| ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE)) {

		EINA_LIST_FOREACH(ugd->ug_UiGadget.ug_pSearchFileList, l, name) {
			if (name) {
				*select_list = eina_list_append(*select_list, strdup(name));
			}
		}
	} else {
		while (gli) {
			ugListItemData *params = (ugListItemData *)elm_object_item_data_get(gli);
			ug_mf_retm_if(params == NULL, "params is NULL");
			if (ugd->ug_UiGadget.ug_iSelectMode == SINGLE_ALL_MODE
			    || ugd->ug_UiGadget.ug_iSelectMode == SINGLE_FILE_MODE
			    || ugd->ug_UiGadget.ug_iSelectMode == IMPORT_SINGLE) {
				if (params->ug_pRadioBox) {
					if (elm_radio_value_get(ugd->ug_MainWindow.ug_pRadioGroup) == params->ug_iGroupValue) {
						*select_list = eina_list_append(*select_list, strdup(params->ug_pItemName->str));
						break;
					}
				}
			} else if (ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE ||
				   ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE ||
				   ugd->ug_UiGadget.ug_iSelectMode == IMPORT_MODE ||
				   ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE ||
				   ugd->ug_UiGadget.ug_iSelectMode == SSM_DOCUMENT_SHARE) {
				if (params->ug_pCheckBox) {
					if (params->ug_bChecked == true) {
						*select_list = eina_list_append(*select_list, strdup(params->ug_pItemName->str));
					}
				}
			}
			nli = elm_genlist_item_next_get(gli);
			gli = nli;
		}
	}
	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_util_get_send_result
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
char *mf_ug_util_get_send_result(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugData is NULL");

	char *file_selected = NULL;
	Eina_List *selected_list = NULL;
	Eina_List *l = NULL;
	char *pNode = NULL;

	if (ugd->ug_UiGadget.ug_iMarkedMode == MARKED_ON
	    && (ugd->ug_UiGadget.ug_iSelectMode == MULTI_FILE_MODE || ugd->ug_UiGadget.ug_iSelectMode == MULTI_ALL_MODE)) {
		__mf_ug_util_get_marked_selected_items(ugd->ug_UiGadget.ug_pMultiSelectFileList, &selected_list);
	} else {
		__mf_ug_util_get_marked_off_selected_items(ugd, &selected_list);
	}

	ug_error(" select_list len is [%d]", eina_list_count(selected_list));
	int a_count = 0;
	EINA_LIST_FOREACH(selected_list, l, pNode) {
		if (pNode != NULL) {
			if (file_selected == NULL) {
				file_selected = g_strconcat(pNode, NULL);
			} else {
				char *temp = file_selected;
				file_selected = g_strconcat(file_selected, NORMAL_SEPERATOR, pNode, NULL);
				free(temp);
			}
			SECURE_ERROR("file_selected[%d] is [%s]", a_count, file_selected);
			a_count++;
		}
	}
	mf_ug_util_free_eina_list_data(&selected_list, NODE_TYPE_CHAR);
	if (file_selected != NULL)
	printf("a_count is [%d] file_list is [%d][%s]\n", a_count, strlen(file_selected), file_selected);
	UG_TRACE_END;
	return file_selected;
}

#ifdef UG_OPERATION_SELECT_MODE
char **mf_ug_util_get_send_result_array(void *data, int *item_count)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, NULL, "ugData is NULL");

	char **array = NULL;
	Eina_List *selected_list = NULL;
	Eina_List *l = NULL;
	char *pNode = NULL;
	int count = 0;

	__mf_ug_util_get_marked_off_selected_items(ugd, &selected_list);

	EINA_LIST_FOREACH(selected_list, l, pNode) {
		if (pNode != NULL) {
			count++;
		}
	}

	if (count > 0) {
		array = calloc(count, sizeof(char *));
		count = 0;
		EINA_LIST_FOREACH(selected_list, l, pNode) {
			if (pNode != NULL && array != NULL) {
				array[count] = g_strdup(pNode);
				count++;
			}
		}
		*item_count = count;
	} else {
		ug_error("no selection!!");
	}

	mf_ug_util_free_eina_list_data(&selected_list, NODE_TYPE_CHAR);
	UG_TRACE_END;
	return array;
}
#endif

/******************************
** Prototype    : _ug_mf_set_state_as
** Description  :
** Input        : struct ugmyfiledata* data
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
void mf_ug_util_set_current_state(void *data, int state)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugd->ug_Status.ug_iState = state;
	UG_TRACE_END;
}


/******************************
** Prototype    : mf_ug_util_get_mmc_state
** Description  :
** Input        : int* mmc_card
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
int mf_ug_util_get_mmc_state(int *mmc_card)
{
	UG_TRACE_BEGIN;
	int error_code = STORAGE_ERROR_NONE;

	ug_mf_retvm_if(mmc_card == NULL, MYFILE_ERR_SRC_ARG_INVALID, "mmc_card is NULL");

	*mmc_card = 0;

	error_code = storage_foreach_device_supported(getSupportedStorages_cb, NULL);
	if (error_code != STORAGE_ERROR_NONE) {
		UG_TRACE_END;
		return MYFILE_ERR_GET_CONF_FAIL;
	}

	storage_state_e mmc_state;
	error_code = storage_get_state(__externalStorageId, &mmc_state);
	if (error_code != STORAGE_ERROR_NONE) {
		UG_TRACE_END;
		return MYFILE_ERR_GET_CONF_FAIL;
	}

	if (mmc_state == STORAGE_STATE_MOUNTED) {
		*mmc_card = MMC_ON;
	} else {
		*mmc_card = MMC_OFF;
	}

	UG_TRACE_END;
	return error_code;
}

/******************************
** Prototype    : mf_ug_util_create_dir_monitor
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
int mf_ug_util_create_dir_monitor(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, UG_ERROR_RETURN, "ugData is NULL");

	if (ugd->ug_UiGadget.ug_pInotifyPipe) {
		ecore_pipe_del(ugd->ug_UiGadget.ug_pInotifyPipe);
		ugd->ug_UiGadget.ug_pInotifyPipe = NULL;
	}
	ugd->ug_UiGadget.ug_pInotifyPipe = ecore_pipe_add(mf_ug_cb_dir_pipe_cb, (const void *)ugd);
	UG_TRACE_END;

	return mf_ug_inotify_handle_init_inotify();
}


/******************************
** Prototype    : mf_ug_util_add_dir_watch
** Description  : Samsung
** Input        : const char *path
**                void *data
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
int mf_ug_util_add_dir_watch(const char *path, void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, UG_ERROR_RETURN, "ugd is NULL");
	UG_SAFE_FREE_CHAR(ugd->ug_Status.monitor_path);
	ugd->ug_Status.monitor_path = g_strdup(path);
	return mf_ug_inotify_handle_add_inotify_watch(path, mf_ug_cb_dir_update_cb, data);
}

int mf_ug_util_remove_dir_watch(void)
{
	return mf_ug_inotify_handle_rm_inotify_watch();
}

/******************************
** Prototype    : mf_ug_util_set_mmc_state_cb
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
int mf_ug_util_set_mmc_state_cb(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, UG_ERROR_RETURN, "ugd is NULL");

	int mmc_state = MMC_OFF;
	mf_ug_util_get_mmc_state(&mmc_state);
	ugd->ug_Status.ug_iMmcFlag = mmc_state;

	UG_TRACE_END;
	return storage_set_state_changed_cb(__externalStorageId, mf_ug_cb_mmc_changed_cb, ugd);
}

/******************************
** Prototype    : mf_ug_util_destory_mmc_state_cb
** Description  : Samsung
** Input        : None
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
void mf_ug_util_destory_mmc_state_cb()
{
	UG_TRACE_BEGIN;
	int error_code = storage_unset_state_changed_cb(__externalStorageId, mf_ug_cb_mmc_changed_cb);
	if (error_code != STORAGE_ERROR_NONE) {
		ug_debug("storage_unset_state_changed_cb() failed!!");
	}
	UG_TRACE_END;
}

/******************************
** Prototype    : mf_ug_util_destory_dir_monitor
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
void mf_ug_util_destory_dir_monitor(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	if (ugd->ug_UiGadget.ug_pInotifyPipe) {
		ecore_pipe_del(ugd->ug_UiGadget.ug_pInotifyPipe);
		ugd->ug_UiGadget.ug_pInotifyPipe = NULL;
	}

	mf_ug_inotify_handle_finalize_inotify();
	UG_TRACE_END;

	return;
}


/******************************
** Prototype    : mf_ug_util_storage_insert_action
** Description  : Samsung
** Input        : void *data
**                char* pItemLabel
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
void mf_ug_util_storage_insert_action(void *data, char *pItemLabel)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	ugFsNodeInfo *pNode = NULL;

	if (ugd->ug_Status.ug_iViewType != mf_ug_view_root) {
		return;
	}

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		if (ugd->ug_Status.ug_iMmcFlag) {
			pNode = (ugFsNodeInfo *) malloc(sizeof(ugFsNodeInfo));
			if (pNode == NULL)
				return;
			memset(pNode, 0, sizeof(ugFsNodeInfo));
			/*set path */
			pNode->path = g_strdup(STORAGE_PARENT);
			/*set name */
			pNode->name = g_strdup(MMC_NAME);
			pNode->type = UG_FILE_TYPE_DIR;
			ugd->ug_UiGadget.ug_pDirList = eina_list_append(ugd->ug_UiGadget.ug_pDirList, pNode);
			mf_ug_genlist_item_append(ugd->ug_MainWindow.ug_pNaviGenlist, MEMORY_FOLDER, ugd, 0, &ugd->ug_Status.ug_1text1icon_itc);
		}
	}

	UG_TRACE_END;
}

void mf_ug_util_mmc_remove_items_by_type(Evas_Object *genlist, int storage_type)
{
	ug_mf_retm_if(genlist == NULL, "parent is NULL");

	ugListItemData *itemData = NULL;
	Elm_Object_Item *it = NULL;

	it = elm_genlist_first_item_get(genlist);
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->storage_type == storage_type) {
			Elm_Object_Item *temp_item = it;
			it = elm_genlist_item_next_get(it);
			elm_object_item_del(temp_item);
			continue;
		}
		it = elm_genlist_item_next_get(it);
	}

}

void mf_ug_util_mmc_remove_action(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retm_if(ugd == NULL, "ugd is NULL");

	int optStorage = MF_UG_MMC;

	if (ugd->ug_Status.ug_iViewType == mf_ug_view_root) {
		mf_ug_genlist_item_remove(ugd->ug_MainWindow.ug_pNaviGenlist, MF_UG_MMC);
	} else {
		if (mf_ug_fm_svc_wapper_get_location(ugd->ug_Status.ug_pPath->str) == optStorage) {
			if (0 != ugd->ug_ListPlay.ug_Player) {
				mf_ug_list_play_destory_playing_file(ugd);
				ugd->ug_ListPlay.play_data = NULL;
				UG_SAFE_FREE_CHAR(ugd->ug_ListPlay.ug_pPlayFilePath);
			}
			if (ugd->ug_UiGadget.ug_iSelectMode == IMPORT_PATH_SELECT_MODE) {
				mf_ug_navi_bar_create_default_view(ugd);
			} else {
				ugd->ug_Status.ug_iViewType = mf_ug_view_root;
				UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
				ugd->ug_Status.ug_pPath = g_string_new(PHONE_FOLDER);
				mf_ug_navi_bar_create_default_view(ugd);
			}
			mf_ug_navi_bar_set_ctrl_item_disable(ugd);
		}
	}
}


/******************************
** Prototype    : mf_ug_util_get_file_launch_type
** Description  :
** Input        : char * path
** Output       : UG_MYFILE_LAUNCH_TYPE
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
mf_ug_launch_type mf_ug_util_get_file_launch_type(char *path)
{
	if (path == NULL) {
		UG_TRACE_END;
		return LAUNCH_TYPE_UNSUPPORT;
	}
	if (mf_file_get(path) == NULL) {
		UG_TRACE_END;
		return LAUNCH_TYPE_UNSUPPORT;
	}
	if (mf_is_dir(path) == 1) {
		UG_TRACE_END;
		return LAUNCH_TYPE_UNSUPPORT;
	}
	mf_ug_fs_file_type category_t = 0;
	{
		mf_ug_file_attr_get_file_category(path, &category_t);
		/*P131206-01154 by wanygan,sound can not play a music file without extension
		do not get the type,retry to get the type using the next api*/
		if (UG_FILE_TYPE_NONE == category_t) {
			category_t = mf_ug_file_attr_get_file_type_by_mime(path);
		}
		if (category_t == UG_FILE_TYPE_MUSIC || category_t == UG_FILE_TYPE_SOUND || category_t == UG_FILE_TYPE_VOICE || category_t == UG_FILE_TYPE_MP4_AUDIO) {
			UG_TRACE_END;
			return LAUNCH_TYPE_MUSIC;
		} else if (category_t == UG_FILE_TYPE_IMAGE) {
			UG_TRACE_END;
			return LAUNCH_TYPE_IMAGE;
		} else {
			UG_TRACE_END;
			return LAUNCH_TYPE_UNSUPPORT;
		}
	}
	UG_TRACE_END;
}

char *mf_ug_util_upper_folder_name_get(void *data, GString *fullpath)
{
	ug_mf_retvm_if(data == NULL, NULL, "data is NULL");
	ug_mf_retvm_if(fullpath == NULL, NULL, "fullpath is NULL");
	ug_mf_retvm_if(fullpath->str == NULL, NULL, "fullpath is NULL");

	GString *parent_path = NULL;
	GString *gName = NULL;
	char *upper_name = NULL;

	parent_path = mf_ug_fm_svc_wrapper_get_file_parent_path(fullpath);

	ug_mf_retvm_if(parent_path == NULL, NULL, "fullpath is NULL");
	ug_mf_retvm_if(parent_path->str == NULL, NULL, "fullpath is NULL");


	if (!g_strcmp0(parent_path->str, PHONE_FOLDER)) {
		upper_name = g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_PHONE));
	} else if (!g_strcmp0(parent_path->str, MEMORY_FOLDER)) {
		upper_name = g_strdup(mf_ug_widget_get_text(MF_UG_LABEL_MMC));
	} else {
		gName = mf_ug_fm_svc_wapper_get_file_name(parent_path);
		UG_GSTRING_CHECK_NULL_GOTO(gName, FAILED_EXIT);
		upper_name = g_strdup(gName->str);
	}

	UG_CHAR_CHECK_NULL_GOTO(upper_name, FAILED_EXIT);
	UG_SAFE_FREE_GSTRING(parent_path);
	UG_SAFE_FREE_GSTRING(gName);
	return upper_name;

FAILED_EXIT:
	UG_SAFE_FREE_GSTRING(parent_path);
	UG_SAFE_FREE_GSTRING(gName);
	return NULL;

}

void mf_ug_util_sort_the_file_list(void *data)
{

	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is null");
	ugData *ugd = (ugData *)data;

	mf_ug_fs_oper_sort_list(&ugd->ug_UiGadget.ug_pFilterList, MF_UG_SORT_BY_NAME_A2Z);
	mf_ug_fs_oper_sort_list(&ugd->ug_UiGadget.ug_pDirList, MF_UG_SORT_BY_NAME_A2Z);

	/*mf_ug_fs_oper_sort_list(&ugd->ug_UiGadget.ug_pFilterList, iSortTypeValue);*/
	/*need to sort folder items only By Name and Date*/
	/*mf_ug_fs_oper_sort_list(&ugd->ug_UiGadget.ug_pDirList, iSortTypeValue);*/
}

int mf_ug_util_check_disk_space(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(data == NULL, MYFILE_ERR_SRC_ARG_INVALID, "data is null");
	ugData *ugd = (ugData *)data;

	int free_space = 0;

	if (ugd->ug_Status.ug_iMore == UG_MORE_CREATE_FOLDER) {
		mf_ug_fm_svc_wapper_get_location(ugd->ug_Status.ug_pPath->str);
		free_space = mf_ug_fm_svc_wrapper_get_free_space();
		/*
		 **     in vfat fs type, sector size is 16K.
		 **     it is to say that the limited size of the free space should be 16K
		 **     or it will report space used up.
		 **     check free_space == 0 can make sure at least 16K is free on the disk
		 **     while every dir takes 4K
		 */
		if (free_space == 0) {
			ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL,
					   MF_UG_LABEL_NOT_ENOUGH_SPACE, MF_UG_LABEL_OK,
					   NULL, NULL, mf_ug_cb_warning_popup_cb, ugd);
			return MYFILE_ERR_NO_FREE_SPACE;
		}
	}

	return MYFILE_ERR_NONE;
}

void mf_ug_util_operation_alloc_failed(void *data)
{
	UG_TRACE_BEGIN;
	ug_mf_retm_if(data == NULL, "data is null");
	ugData *ugd = (ugData *)data;

	ugd->ug_MainWindow.ug_pNormalPopup = mf_ug_popup_create(ugd, UG_POPMODE_TEXT, NULL, MF_UG_LABEL_NOT_ENOUGH_SPACE,
			   NULL, NULL, NULL, (Evas_Smart_Cb) elm_exit, NULL);
}

long mf_ug_util_character_count_get(const char *original)
{
	ug_mf_retvm_if(original == NULL, 0, "input string is NULL");
	long count = 0;
	char *utf8_form = g_locale_to_utf8(original, -1, NULL, NULL, NULL);
	if (utf8_form == NULL)
		return count;
	else {
		  count = g_utf8_strlen(utf8_form, -1);
		  free(utf8_form);
		  ug_debug("utf8 count is %ld", count);
		  return count;
	}
}

void mf_util_remove_item_from_list_by_location(Eina_List **list, int location)
{
	ug_mf_retm_if(list == NULL, "list is NULL");

	Eina_List *l = NULL;
	ugFsNodeInfo *node = NULL;

	EINA_LIST_FOREACH(*list, l, node) {
		if ((ugFsNodeInfo *)node != NULL && strlen(((ugFsNodeInfo *)node)->path) != 0) {
			if (mf_ug_fm_svc_wapper_get_location(node->path) == location) {
				UG_SAFE_FREE_CHAR(node->ext);
				UG_SAFE_FREE_CHAR(node->path);
				UG_SAFE_FREE_CHAR(node->name);
				UG_SAFE_FREE_CHAR(node);
				*list = eina_list_remove_list(*list, l);
			}
		}
	}
}

int mf_ug_util_generate_root_view_file_list(Eina_List **list, int storage_state)
{
	UG_TRACE_BEGIN;

	ugFsNodeInfo *pNode = NULL;
	pNode = (ugFsNodeInfo *) malloc(sizeof(ugFsNodeInfo));
	if (pNode == NULL)
		return -1;
	memset(pNode, 0, sizeof(ugFsNodeInfo));
	/*set path */
	pNode->path = g_strdup(PHONE_PARENT);
	/*set name */
	pNode->name = g_strdup(PHONE_NAME);
	pNode->type = UG_FILE_TYPE_DIR;
	*list = eina_list_append(*list, pNode);

	if (storage_state == MMC_ON) {
		pNode = (ugFsNodeInfo *) malloc(sizeof(ugFsNodeInfo));
		if (pNode == NULL)
			return -1;
		memset(pNode, 0, sizeof(ugFsNodeInfo));
		/*set path */
		pNode->path = g_strdup(STORAGE_PARENT);
		/*set name */
		pNode->name = g_strdup(MMC_NAME);
		pNode->type = UG_FILE_TYPE_DIR;
		*list = eina_list_append(*list, pNode);
	}

	return 0;
}

char *mf_ug_util_get_default_ringtone()
{
	char *default_ringtone = NULL;
	int retcode = -1;
	retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_INCOMING_CALL_RINGTONE, &default_ringtone);

	SECURE_INFO("default_ringtone is [%s]", default_ringtone);
	if ((retcode == SYSTEM_SETTINGS_ERROR_NONE) && default_ringtone && mf_file_exists(default_ringtone)) {
		return default_ringtone;
	} else {
		UG_SAFE_FREE_CHAR(default_ringtone);
		default_ringtone = g_strdup(UG_SETTING_DEFAULT_RINGTONE_PATH);
	}
	return default_ringtone;
}

char *mf_ug_util_get_default_alert()
{
	char *default_ringtone = NULL;
	int retcode = -1;
	retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_SOUND_NOTIFICATION, &default_ringtone);

	SECURE_INFO("default_ringtone is [%s]", default_ringtone);
	if ((retcode == SYSTEM_SETTINGS_ERROR_NONE) && default_ringtone && mf_file_exists(default_ringtone)) {
		return default_ringtone;
	} else {
		UG_SAFE_FREE_CHAR(default_ringtone);
		default_ringtone = g_strdup(UG_SETTING_DEFAULT_ALERT_PATH);
	}
	return default_ringtone;
}

int mf_ug_util_set_default_ringtone_cb(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, UG_ERROR_RETURN, "ugd is NULL");

	UG_TRACE_END;
	return system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_INCOMING_CALL_RINGTONE, mf_ug_cb_default_ringtone_changed_cb, ugd);
}

void mf_ug_util_destory_default_ringtone_cb()
{
	UG_TRACE_BEGIN;
	int retcode = system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_INCOMING_CALL_RINGTONE);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ug_mf_error("[ERR] failed to unset the default ringtone");
	}
	UG_TRACE_END;
}

int mf_ug_util_set_default_alert_cb(void *data)
{
	UG_TRACE_BEGIN;
	ugData *ugd = (ugData *)data;
	ug_mf_retvm_if(ugd == NULL, UG_ERROR_RETURN, "ugd is NULL");

	UG_TRACE_END;
	return system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_SOUND_NOTIFICATION, mf_ug_cb_default_ringtone_changed_cb, ugd);
}

void mf_ug_util_destory_default_alert_cb()
{
	UG_TRACE_BEGIN;
	int retcode = system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_SOUND_NOTIFICATION);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ug_mf_error("[ERR] failed to unset the default alert");
	}
	UG_TRACE_END;
}

bool mf_ug_util_find_item_from_pnode_list(Eina_List *list, const char *fullpath)
{
	Eina_List *l = NULL;
	ugFsNodeInfo *pNode = NULL;
	EINA_LIST_FOREACH(list, l, pNode) {
		if (pNode) {
			if (pNode->path && pNode->name) {
				char *real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
				if (g_strcmp0(real_name, fullpath) == 0) {
					free(real_name);
					real_name = NULL;
					return true;
				}
			}
		} else {
			continue;
		}
	}
	return false;
}

bool mf_ug_util_is_unique_view(int mode)
{
	bool flag = false;
	if (mode == IMPORT_MODE
	    || mode == IMPORT_PATH_SELECT_MODE
	    || mode == IMPORT_SINGLE
	    || mode == DOCUMENT_SHARE
	    || mode ==  SSM_DOCUMENT_SHARE
	) {
		flag = true;
	}
	return flag;
}

bool mf_ug_util_is_multi_select_mode(int mode)
{
	bool flag = false;
	if (mode == MULTI_FILE_MODE
		|| mode == MULTI_ALL_MODE
		|| mode == IMPORT_PATH_SELECT_MODE
		|| mode == IMPORT_MODE
		|| mode ==  SSM_DOCUMENT_SHARE
	) {
		flag = true;
	}
	return flag;
}
bool mf_ug_util_is_single_select_mode(int mode)
{
	bool flag = false;
	if (mode == SINGLE_FILE_MODE
	   || mode == SINGLE_ALL_MODE
	   || mode == IMPORT_SINGLE
	) {
		flag = true;
	}
	return flag;
}

bool mf_ug_util_is_import_mode(int mode)
{
		bool flag = false;
		if (mode == IMPORT_MODE
		   || mode == IMPORT_PATH_SELECT_MODE
		   || mode == IMPORT_SINGLE
		) {
			flag = true;
		}
		return flag;

}

static Eina_List *path_stack = NULL;

void mf_ug_util_path_push(char *path, int view_type)
{
	mf_ug_view_node_s *view_node = calloc(1, sizeof(mf_ug_view_node_s));
	
	if (view_node != NULL) {
		view_node->view_type = view_type;
		view_node->path = g_strdup(path);
		ug_error("path is [%s] view_type is [%d]", path, view_type);
		path_stack = eina_list_prepend(path_stack, view_node);
	}
}

mf_ug_view_node_s *mf_ug_util_path_pop()
{
	
	mf_ug_view_node_s *view_node = eina_list_nth(path_stack, 0);
	path_stack = eina_list_remove(path_stack, view_node);
	return view_node;
}

mf_ug_view_node_s *mf_ug_util_path_top_get()
{
	mf_ug_view_node_s *view_node = eina_list_nth(path_stack, 0);
	if (view_node) {
		ug_error("================= top path is [%s] view_type is [%d] ", view_node->path, view_node->view_type);
	} else {
		ug_error("Failed to get the top");
	}
	return view_node;
}

void mf_ug_util_view_node_free(mf_ug_view_node_s **view_node)
{
	if (*view_node != NULL) {
		UG_SAFE_FREE_CHAR((*view_node)->path);
		free(*view_node);
		*view_node = NULL;
	}
}

void mf_ug_util_path_stack_free()
{
	if (path_stack) {
		mf_ug_view_node_s *view_node = NULL;
		Eina_List *l = NULL;
		
		EINA_LIST_FOREACH(path_stack, l, view_node) {
			mf_ug_util_view_node_free(&view_node);
		}
		eina_list_free(path_stack);
		path_stack = NULL;
	}
}
