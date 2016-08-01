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




#include <sys/statvfs.h>
#include <storage.h>
#include "mf-ug-util.h"
#include "mf-ug-cb.h"
#include "mf-ug-main.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-resource.h"
#include "mf-ug-widget.h"
#include "mf-ug-file-util.h"

#define MF_UG_PATH_INFO_RETRENCH			128
#define MF_UG_PATH_INFO_HEAD_LEN(x)			strlen(x)
#define MF_UG_PATH_INFO_TRANS_OMIT			elm_entry_utf8_to_markup("..")
#define	MF_UG_PATH_INFO_LEVEL_BOUNDARY			3
#define MF_UG_PATH_INFO_LEN_THRESHOLD			4
#define MF_UG_PATH_INFO_SEP				elm_entry_utf8_to_markup("/")
typedef struct {
	int len_orig;
	int len_trans;
	char *original;
	char *transfer;
	bool flag_trans;
} ug_pNode;

/*********************
**Function name:	__mf_ug_fm_svc_wapper_COMESFROM
**Parameter:
**	GString* fullpath:	fullpath to check the location
**
**Return value:
**	location of the path
**
**Action:
**	get storage type by fullpath
*********************/
static int __mf_ug_fm_svc_wapper_COMESFROM(char *fullpath)
{
	if (!fullpath) {
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}

	if (PHONE_FOLDER == NULL) {
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
	if (MEMORY_FOLDER == NULL) {
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
	int len_phone = strlen(PHONE_FOLDER);
	int len_memory = strlen(MEMORY_FOLDER);

	if (strncmp(fullpath, PHONE_FOLDER, len_phone) == 0) {
		return MF_UG_PHONE;
	} else if (strncmp(fullpath, MEMORY_FOLDER, len_memory) == 0) {
		return MF_UG_MMC;
	} else {
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
}

/******************************
** Prototype    : _ug_mf_get_file_list
** Description  :
** Input        : struct ugmyfiledata *data
**                GString* folder_name
**                Eina_List** dir_list
**                Eina_List** file_list
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
static int __mf_ug_fm_svc_wapper_get_file_list(GString *fullpath, Eina_List **dir_list, Eina_List **file_list)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(fullpath == NULL, MYFILE_ERR_INVALID_ARG, "fullpath is NULL");
	ug_mf_retvm_if(fullpath->str == NULL, MYFILE_ERR_INVALID_ARG, "fullpath->str is NULL");
	ug_mf_retvm_if(fullpath->len == 0, MYFILE_ERR_INVALID_ARG, "fullpath->len is 0");

	int error_code = 0;

	error_code = mf_ug_fs_oper_read_dir(fullpath->str, dir_list, file_list);
	if (error_code != 0) {
		ug_debug("error_code is [%d]\n", error_code);
	} else {
		ug_debug("success get the file list\n");
	}
	UG_TRACE_END;
	return error_code;
}


/******************************
** Prototype    : mfUgGetFileListWithFormat
** Description  :
** Input        : struct ugmyfiledata *data
**                GString* folder_name
**                Eina_List** dir_list
**                Eina_List** file_list
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
unsigned long mf_ug_fm_svc_wapper_get_file_filter(int file_filter_mode)
{
	unsigned long filter = 0;
	switch (file_filter_mode) {
	case SHOW_ALL_LIST:
		filter = UG_FILTER_CATEGORY_ALL;
		ug_debug("show_all_list:::::::::::::::::::::::::::::::::: filter is [%d]", filter);
		break;
	case SHOW_IMAGE_LIST:
		filter |= UG_FILTER_CATEGORY_IMAGE;
		break;
	case SHOW_SOUND_LIST:
		filter |= UG_FILTER_CATEGORY_MUSIC | UG_FILTER_CATEGORY_SOUND | UG_FILTER_CATEGORY_VOICE | UG_FILTER_CATEGORY_MP4_AUDIO;
		break;
	case SHOW_VIDEO_LIST:
		filter |= UG_FILTER_CATEGORY_VIDEO | UG_FILTER_CATEGORY_MP4_VIDEO;
		break;
	case SHOW_FLASH_LIST:
		filter |= UG_FILTER_CATEGORY_FLASH;
		break;
	case SHOW_FOLDER_LIST:
		break;
	case SHOW_IMAGE_VIDEO_LIST:
		filter |= UG_FILTER_CATEGORY_IMAGE | UG_FILTER_CATEGORY_VIDEO | UG_FILTER_CATEGORY_MP4_VIDEO;
		break;
	case SHOW_IMAGE_SOUND_LIST:
		filter |= UG_FILTER_CATEGORY_IMAGE | UG_FILTER_CATEGORY_SOUND | UG_FILTER_CATEGORY_MUSIC | UG_FILTER_CATEGORY_VOICE | UG_FILTER_CATEGORY_MP4_AUDIO;
		break;
	case SHOW_VIDEO_SOUND_LIST:
		filter |= UG_FILTER_CATEGORY_SOUND | UG_FILTER_CATEGORY_VIDEO | UG_FILTER_CATEGORY_MUSIC | UG_FILTER_CATEGORY_VOICE | UG_FILTER_CATEGORY_MP4_AUDIO;
		break;
	case SHOW_DOCUMENT_LIST:
		filter |= UG_FILTER_CATEGORY_DOC | UG_FILTER_CATEGORY_EXCEL | UG_FILTER_CATEGORY_PPT | UG_FILTER_CATEGORY_PDF | UG_FILTER_CATEGORY_TXT | UG_FILTER_CATEGORY_SNB | UG_FILTER_CATEGORY_GUL | UG_FILTER_CATEGORY_HWP;
		break;
	default:
		break;
	}
	return filter;
}

int mf_ug_fm_svc_wapper_get_file_list_by_filter(ugData *data, GString *fullpath, Eina_List **dir_list, Eina_List **filter_list)
{
	ugData *ugd = data;
	ug_mf_retvm_if(ugd == NULL, MYFILE_ERR_INVALID_ARG, "ugd is NULL");
	ug_mf_retvm_if(fullpath == NULL, MYFILE_ERR_INVALID_ARG, "fullpath is NULL");
	ug_mf_retvm_if(fullpath->str == NULL, MYFILE_ERR_INVALID_ARG, "fullpath->str is NULL");
	ug_mf_retvm_if(fullpath->len == 0, MYFILE_ERR_INVALID_ARG, "fullpath->len is 0");

	int error_code = 0;
	int filter_mode = 0;
	int file_filter = 0;
	int file_list_len = 0;
	Eina_List *file_list = NULL;
	char *extension = NULL;

	filter_mode = ugd->ug_UiGadget.ug_iFilterMode;
	file_filter = ugd->ug_UiGadget.ug_iFileFilter;

	error_code = __mf_ug_fm_svc_wapper_get_file_list(fullpath, dir_list, &file_list);

	if (error_code == 0) {
		file_list_len = eina_list_count(file_list);

		if (file_list_len > 0) {
			ug_debug("file_filter is [%d]\n", filter_mode);

			if (filter_mode != SHOW_BY_EXTENSION) {
				ug_debug("file_filter is [%d] ", file_filter);
				error_code = mf_ug_fs_oper_list_filter(file_list, filter_list, file_filter);
			} else if (ugd->ug_UiGadget.ug_pExtension != NULL) {
				extension = strdup(ugd->ug_UiGadget.ug_pExtension);
				error_code = mf_ug_fs_oper_list_filter_by_extension(file_list, filter_list, extension);
				free(extension);
			}
			return error_code;
		}
	}
	return error_code;
}

bool mf_ug_fm_svc_wapper_is_default_ringtone(void *data, char* selected_file)
{
	UG_TRACE_BEGIN;
	ugData *ugd = data;
	ugFsNodeInfo *pNode = NULL;
	char *real_name = NULL;
	Eina_List *l = NULL;
	ug_mf_retvm_if(ugd == NULL, false, "ugd is NULL");
	ug_mf_retvm_if(selected_file == NULL, false, "selected_file is NULL");
	ug_mf_retvm_if(ugd->ug_UiGadget.ug_pFilterList == NULL, false, "ugd->ug_UiGadget.ug_pFilterList is NULL");
	EINA_LIST_FOREACH(ugd->ug_UiGadget.ug_pFilterList, l, pNode) {
		if (pNode) {
			if (pNode->path && pNode->name) {
				real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
			}
		} else {
			continue;
		}
		if (real_name != NULL && strcmp(selected_file, real_name) == 0) {
			SECURE_DEBUG("real_name=%s", real_name);
			UG_SAFE_FREE_CHAR(real_name);
			return true;
		}
		UG_SAFE_FREE_CHAR(real_name);
	}

	return false;
}

/******************************
** Prototype    : mfUgIsRootPath
** Description  :
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
bool mf_ug_fm_svc_wapper_is_root_path(void *data)
{
	char *g_path = (char *)data;
	ug_mf_retvm_if(g_path == NULL, false, "g_path is NULL");

	if (!strcmp(g_path, PHONE_FOLDER)) {
		return true;
	} else if (!strcmp(g_path, MEMORY_FOLDER)) {
		return true;
	} else {
		return false;
	}
}


/******************************
** Prototype    : mfUgGetFileName
** Description  :
** Input        : GString* path
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
GString *mf_ug_fm_svc_wapper_get_file_name(GString *path)
{
	GString *ret = NULL;
	if (mf_file_exists(path->str)) {
		ret = g_string_new(mf_file_get(path->str));
	} else {
		ret = NULL;
	}
	return ret;
}

char *mf_ug_fm_svc_wapper_get_root_path_by_tab_label(const char *label)
{
	if (g_strcmp0(label, MF_UG_LABEL_PHONE) == 0) {
		return g_strdup(PHONE_FOLDER);
	} else if (g_strcmp0(label, MF_UG_LABEL_MMC) == 0) {
		return g_strdup(MEMORY_FOLDER);
	} else {
		return NULL;
	}
}

/******************************
** Prototype    : mf_ug_fm_svc_wapper_get_location
** Description  : Samsung
** Input        : char* fullpath
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
int mf_ug_fm_svc_wapper_get_location(char *fullpath)
{
	return __mf_ug_fm_svc_wapper_COMESFROM(fullpath);
}

gint mf_ug_fm_svc_wapper_get_folder_foldersystem(GString *path, bool * result)
{

	int error_code = 0;
	error_code = mf_ug_file_attr_is_system_dir(path->str, result);
	return error_code;

}

GString *mf_ug_fm_svc_wrapper_get_file_parent_path(GString *fullpath)
{
	GString *ret = NULL;
	char *path = NULL;
	int error_code = 0;

	if (fullpath == NULL || fullpath->str == NULL) {
		return NULL;
	}
	error_code = mf_ug_file_attr_get_parent_path(fullpath->str, &path);
	if (error_code != 0) {
		return NULL;
	}

	ret = g_string_new(path);
	free(path);
	path = NULL;
	return ret;
}

char *mf_ug_fm_svc_path_info_retrench(const char *string)
{
	ug_mf_retvm_if(string == NULL, g_strdup(MF_UG_PATH_INFO_TRANS_OMIT), "input path is NULL");
	char *retrench = NULL;
	char *utf8_string = elm_entry_utf8_to_markup(string);
	if (utf8_string && strlen(string) > MF_UG_PATH_INFO_LEN_THRESHOLD) {
		if (g_utf8_strlen(utf8_string, -1) > 2) {
			retrench = calloc(1, MF_UG_PATH_INFO_RETRENCH);
			if (retrench) {
				char *omit = MF_UG_PATH_INFO_TRANS_OMIT;
				char *temp = g_utf8_strncpy(retrench, utf8_string, 2);
				retrench = g_strconcat(temp, omit, NULL);
				UG_SAFE_FREE_CHAR(omit);
				UG_SAFE_FREE_CHAR(temp);
			}
			UG_SAFE_FREE_CHAR(utf8_string);

		} else {
			retrench = utf8_string;
		}
		return retrench;
	} else {
		return utf8_string;
	}
}

static void __mf_ug_fm_svc_wrapper_path_info_node_free(Eina_List *list)
{
	ug_mf_retm_if(list == NULL, "list is NULL");
	const Eina_List *l = NULL;
	void *data = NULL;
	EINA_LIST_FOREACH(list, l, data) {
		ug_pNode *node = (ug_pNode *)data;
		if (node != NULL) {
			UG_SAFE_FREE_CHAR(node->original);
			UG_SAFE_FREE_CHAR(node->transfer);
			UG_SAFE_FREE_CHAR(node);
		}
	}
	eina_list_free(list);
}


char *mf_ug_fm_svc_path_info_translate(char *path_info, int path_info_max_len)
{

	ug_mf_retvm_if(path_info == NULL, g_strdup(dgettext("sys_string", "IDS_COM_BODY_UNKNOWN")), "input path is NULL");

	int top = 0;
	bool flag = TRUE;
	Eina_List *temp_list = NULL;
	const Eina_List *l = NULL;
	gchar **result = NULL;
	gchar **params = NULL;
	int count = 0;
	int max_len = 0;
	int total_len = 0;
	int i = 0;
	char *output = NULL;
	void *pnode = NULL;
	char *omit = MF_UG_PATH_INFO_TRANS_OMIT;
	if (!omit) {
		return NULL;
	}

	if (strlen(path_info) < path_info_max_len) {
		UG_SAFE_FREE_CHAR(omit);
		output = g_strdup(path_info);
		UG_SAFE_FREE_CHAR(path_info);
		return output;
	}

	result = g_strsplit(path_info, "/", 0);
	if (result == NULL) {
		free(path_info);
		path_info = NULL;
		UG_SAFE_FREE_CHAR(omit);
		return g_strdup(dgettext("sys_string", "IDS_COM_BODY_UNKNOWN"));
	}

	params = result;
	count = g_strv_length(result);

	if (count > MF_UG_PATH_INFO_LEVEL_BOUNDARY) {
		top = MF_UG_PATH_INFO_LEVEL_BOUNDARY;
		flag = FALSE;
		max_len = path_info_max_len - MF_UG_PATH_INFO_LEVEL_BOUNDARY - MF_UG_PATH_INFO_HEAD_LEN(omit);
		/*(2 is length of ..) ../aa../bb..*/
	} else {
		top = count;
		flag = TRUE;
		max_len = path_info_max_len - (count - 1);
	}

	for (i = top; i > 1; i--) {
		ug_pNode *nodeB = calloc(sizeof(ug_pNode), 1);

		if (nodeB != NULL) {
			nodeB->original = elm_entry_utf8_to_markup(params[count - i]);
			nodeB->len_orig = strlen(params[count - i]);
			nodeB->transfer = mf_ug_fm_svc_path_info_retrench(params[count - i]);

			if (nodeB->transfer != NULL) {
				nodeB->len_trans = strlen(nodeB->transfer);
			}

			nodeB->flag_trans = FALSE;
		}
		if (nodeB) {
			total_len += nodeB->len_orig;
		}

		temp_list = eina_list_append(temp_list, nodeB);
	}

	total_len += strlen(params[count - 1]);

	for (i = 0 ; i < eina_list_count(temp_list); i++) {
		if (total_len > max_len) {
			ug_pNode *data = NULL;
			data = eina_list_nth(temp_list, i);

			if (data != NULL) {
				total_len -= (data->len_orig - data->len_trans);
				data->flag_trans = TRUE;
			}
		}

		if (total_len <= max_len) {
			break;
		}
	}


	if (flag == FALSE) {
		output = elm_entry_utf8_to_markup("..");
	}
	char *temp = NULL;
	char *sep = MF_UG_PATH_INFO_SEP;
	EINA_LIST_FOREACH(temp_list, l, pnode) {
		if (!pnode) {
			continue;
		}
		ug_pNode *node = (ug_pNode *)pnode;
		temp = output;
		if (node->flag_trans == TRUE) {
			if (output != NULL) {
				output = g_strconcat(output, sep, node->transfer, NULL);
			} else {
				output = g_strdup(node->transfer);
			}
		} else {
			if (output != NULL) {
				output = g_strconcat(output, sep , node->original, NULL);
			} else {
				output = g_strdup(node->original);
			}
		}
		UG_SAFE_FREE_CHAR(temp);
	}
	temp = output;
	char *last_string = params[count - 1];
	char *utf8_last = elm_entry_utf8_to_markup(last_string);

	if (output != NULL && utf8_last != NULL) {
		int last_len = strlen(last_string);
		int output_len = strlen(output);
		int d_value = path_info_max_len - output_len;
		if ((last_len + output_len) > path_info_max_len) {
			const char *end = NULL;
			gboolean ret = FALSE;
			ret = g_utf8_validate(utf8_last, d_value, &end);
			if (ret == TRUE) {
				d_value = last_len - strlen(end);
				utf8_last[d_value] = '\0';
				output = g_strconcat(output, sep, utf8_last, omit, NULL);
				UG_SAFE_FREE_CHAR(temp);
			}
		} else {
			output = g_strconcat(output, sep, utf8_last, NULL);
			UG_SAFE_FREE_CHAR(temp);
		}
	} else {
		if (utf8_last != NULL) {
			output = g_strdup(utf8_last);
		} else {
			output = NULL;
		}

		UG_SAFE_FREE_CHAR(temp);
	}
	UG_SAFE_FREE_CHAR(utf8_last);
	UG_SAFE_FREE_CHAR(sep);
	UG_SAFE_FREE_CHAR(omit);
	UG_SAFE_FREE_CHAR(path_info);
	__mf_ug_fm_svc_wrapper_path_info_node_free(temp_list);
	temp_list = NULL;
	g_strfreev(result);
	result = NULL;
	return output;
}


char *mf_ug_fm_svc_wrapper_translate_path(char *original_path)
{
	ug_mf_retvm_if(original_path == NULL, g_strdup(dgettext("sys_string", "IDS_COM_BODY_UNKNOWN")), "input path is NULL");

	char *new_path = NULL;
	int root_len = 0;

	if (mf_ug_fm_svc_wapper_get_location(original_path) == MF_UG_PHONE) {
		root_len = strlen(PHONE_FOLDER);
		new_path = g_strconcat(mf_ug_widget_get_text(MF_UG_LABEL_PHONE), original_path + root_len, "/", NULL);
	} else if (mf_ug_fm_svc_wapper_get_location(original_path) == MF_UG_MMC) {
		root_len = strlen(MEMORY_FOLDER);
		new_path = g_strconcat(mf_ug_widget_get_text(MF_UG_LABEL_MMC), original_path + root_len, "/", NULL);
	} else {
		new_path = g_strdup(original_path);
	}

	ug_debug("new path is %s", new_path);
	return new_path;
}

char *mf_ug_fm_svc_wapper_path_info_get(char *original_path)
{
	ug_mf_retvm_if(original_path == NULL, g_strdup(dgettext("sys_string", "IDS_COM_BODY_UNKNOWN")), "input path is NULL");
	char *path_info = NULL;
	int len = 0;

	path_info = mf_ug_fm_svc_wrapper_translate_path(original_path);
	if (path_info) {
		len = strlen(path_info);
		if (len > 0 && path_info[len - 1] == '/') {
			path_info[len - 1] = '\0';
		}
	}
	return path_info;

}

/*
unsigned long mf_ug_fm_svc_wrapper_get_free_space(int state)
{
	struct statvfs info;
	char *path = NULL;

	if (state == MF_UG_PHONE) {
		path = PHONE_FOLDER;
	} else if (state == MF_UG_MMC) {
		path = MEMORY_FOLDER;
	} else {
		return -1;
	}

	if (-1 == statvfs(path, &info)) {
		return -2;
	}
	return (info.f_bsize) * info.f_bfree;
}
*/

int mf_ug_fm_svc_wrapper_get_free_space(int state)
{
	struct statvfs info;
	char *path = NULL;
	int ret = 0;

	if (state == MF_UG_PHONE) {
		/*path = PHONE_FOLDER;*/
		ret = storage_get_internal_memory_size(&info);
	} else if (state == MF_UG_MMC) {
		path = MEMORY_FOLDER;
		ret = statvfs(path, &info);
	} else {
		return -1;
	}

	if (-1 == ret) {
		return -2;
	}
	return (info.f_bsize) * info.f_bfree;
	/*struct statvfs s;
	return storage_get_internal_memory_size(&s);*/
}

bool mf_ug_fm_svc_wrapper_detect_duplication(GString *to)
{
	int existing = MYFILE_ERR_NONE;
	if (to == NULL) {
		return false;
	}
	GString *parent_path = mf_ug_fm_svc_wrapper_get_file_parent_path(to);
	GString *file_name = mf_ug_fm_svc_wapper_get_file_name(to);

	SECURE_DEBUG("full path and file name %s", to->str);
	if (file_name == NULL || parent_path == NULL || file_name->len == 0) {
		return false;
	}

	if (parent_path->str != NULL) {
		ug_debug("parent_path->str is %s", parent_path->str);
	}
	if (file_name->str != NULL) {
		SECURE_DEBUG("file_name->str is %s", file_name->str);
	}

	existing = mf_ug_file_attr_is_duplicated_name(parent_path->str, file_name->str);

	if (parent_path != NULL) {
		g_string_free(parent_path, TRUE);
	}
	parent_path = NULL;

	if (file_name != NULL) {
		g_string_free(file_name, TRUE);
	}
	file_name = NULL;

	if (existing == MYFILE_ERR_NONE) {
		return false;
	} else {
		return true;
	}
}

static int __mf_ug_fm_svc_wrapper_get_next_number(char *file_name_without_ext, int file_name_type)
{
	int nCount = 0;
	int nLength = 0;
	int nUnderline = 0;
	bool bAllDigits = true;
	int i;

	/* check _02d format */
	nLength = strlen(file_name_without_ext);

	if (file_name_type == FILE_NAME_WITH_UNDERLINE) {
		if (nLength < 3) {	/*4 means the # of minimum characters (*_n) */
			return 1;	/*doesn't match */
		} else {	/* input is more than 3 bytes */
			/* find '_' */
			for (nUnderline = nLength - 1; nUnderline >= 0; nUnderline--) {
				if (file_name_without_ext[nUnderline] == '_') {
					break;
				}
			}

			if (nUnderline == 0 && file_name_without_ext[0] != '_') {
				return 1;	/* doesn't match */
			}
			/* check the right characters are all digits */
			for (i = nUnderline + 1; i < nLength; i++) {
				if (file_name_without_ext[i] < '0' || file_name_without_ext[i] > '9') {
					bAllDigits = false;
					break;
				}
			}

			if (bAllDigits) {
				for (i = nUnderline + 1; i < nLength; i++) {
					nCount *= 10;
					nCount += file_name_without_ext[i] - '0';
				}

				file_name_without_ext[nUnderline] = '\0';	/* truncate the last  '_dd' */
			}
		}
	} else {

		if (nLength < 5) {	/* 5 means the # of minimum characters (*_(n)) */
			return 1;	/*doesn't match */
		} else {	/* input is more than 3 bytes */
			/* find '_' */
			for (nUnderline = nLength - 1; nUnderline >= 0; nUnderline--) {
				if (file_name_without_ext[nUnderline] == '(') {
					break;
				}
			}

			if (nUnderline == 0 && file_name_without_ext[0] != '(') {
				return 1;	/* doesn't match */
			}
			/* check the right characters are all digits */
			for (i = nUnderline + 1; i < nLength - 1; i++) {
				if (file_name_without_ext[i] < '0' || file_name_without_ext[i] > '9') {
					bAllDigits = false;
					break;
				}
			}

			/* and more than 2 columns. */
			if (bAllDigits) {
				for (i = nUnderline + 1; i < nLength - 1; i++) {
					nCount *= 10;
					nCount += file_name_without_ext[i] - '0';
				}

				file_name_without_ext[nUnderline] = '\0';	/* truncate the last  '_dd' */
			}
		}
	}

	/* increase nCount by 1 */
	nCount++;

	return nCount;
}

static int __mf_ug_fm_svc_wrapper_get_unique_name(const char *default_dir_full_path, char *original_file_name, char **unique_file_name,
        int file_name_type, void *data)
{
	/*mf_debug("%s %d\n", __func__, __LINE__);*/
	ug_mf_retvm_if(unique_file_name == NULL, MYFILE_ERR_SRC_ARG_INVALID, "unique_file_name is NULL");
	ug_mf_retvm_if(data == NULL, MYFILE_ERR_SRC_ARG_INVALID, "data is NULL");

	char *file_name_without_ext = NULL;
	char *file_ext = NULL;
	char *new_file_name = NULL;
	bool result = false;
	char *dir_rel_path = NULL;
	int nCount = 0;
	bool bExt = false;
	int error_code = 0;

	if (default_dir_full_path == NULL || original_file_name == NULL) {
		ug_debug("default_dir_full_path == NULL || \
						original_file_name == NULL ||   \
						unique_file_name == NULL || \
						error_code == NULL ");
		error_code =  MYFILE_ERR_SRC_ARG_INVALID;
		goto Exception;
	}
	result = mf_ug_file_attr_get_logical_path_by_full(default_dir_full_path, &dir_rel_path);

	if (result) {
		error_code = MYFILE_ERR_GET_LOGIC_PATH_FAIL;
		goto Exception;
	}

	error_code = mf_ug_file_attr_is_duplicated_name(default_dir_full_path, original_file_name);
	if (error_code == 0) {
		SECURE_DEBUG("unique_file_name [%s]", *unique_file_name);
		SECURE_DEBUG("original_file_name [%s]", new_file_name);
		*unique_file_name = g_strdup(original_file_name);
		SECURE_DEBUG("unique_file_name [%s]", *unique_file_name);
	}

	while (error_code < 0) {
		error_code = 0;
		bExt = mf_ug_file_attr_get_file_ext(original_file_name, &file_ext);
		file_name_without_ext = g_strdup(original_file_name);

		if (file_name_without_ext == NULL) {
			error_code = MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
			goto Exception;
		}

		/* add a condition, whether extention is or not. */
		if (bExt == 0) {
			file_name_without_ext[strlen(file_name_without_ext) - strlen(file_ext) - 1] = '\0';
		}

		nCount = __mf_ug_fm_svc_wrapper_get_next_number(file_name_without_ext, file_name_type);
		if (nCount == 1 && file_name_type == FILE_NAME_WITH_BRACKETS) {
			char *file_name_with_space = g_strconcat(file_name_without_ext, " ", NULL);
			if (file_name_with_space) {
				UG_SAFE_FREE_CHAR(file_name_without_ext);
				file_name_without_ext = file_name_with_space;
				file_name_with_space = NULL;
			}
		}

		if (bExt == 0) {
			if (file_name_type == FILE_NAME_WITH_BRACKETS) {
				new_file_name = g_strdup_printf("%s(%d).%s", file_name_without_ext, nCount, file_ext);
			} else {
				new_file_name = g_strdup_printf("%s_%d.%s", file_name_without_ext, nCount, file_ext);
			}
		} else {

			if (file_name_type == FILE_NAME_WITH_BRACKETS) {
				new_file_name = g_strdup_printf("%s(%d)", file_name_without_ext, nCount);
			} else {
				new_file_name = g_strdup_printf("%s_%d", file_name_without_ext, nCount);
			}
		}
		/*mf_debug("new_file_name [%s]", new_file_name);
		mf_debug("original_file_name [%s]", new_file_name);*/
		UG_SAFE_FREE_CHAR(file_name_without_ext);

		SECURE_DEBUG("new name is %s\n", new_file_name);

		error_code = mf_ug_file_attr_is_duplicated_name(default_dir_full_path, new_file_name);
		if (error_code == 0) {
			*unique_file_name = g_strdup(new_file_name);
			/*mf_debug("rename finished\n");*/
			error_code =  MYFILE_ERR_NONE;
			goto Exception;
		} else {
			/*mf_debug("rename continue\n");*/
			original_file_name = g_strdup(new_file_name);
			UG_SAFE_FREE_CHAR(new_file_name);
		}
		UG_SAFE_FREE_CHAR(file_ext);
	}

	return MYFILE_ERR_NONE;

Exception:
	UG_SAFE_FREE_CHAR(dir_rel_path);
	UG_SAFE_FREE_CHAR(file_ext);
	UG_SAFE_FREE_CHAR(new_file_name);
	return error_code;
}

int mf_ug_fm_svc_wrapper_file_auto_rename(void *data, GString *fullpath, int file_name_type, GString **filename)
{
	ug_mf_retvm_if(data == NULL, MYFILE_ERR_SRC_ARG_INVALID, "data is NULL");
	ugData *ugd = (ugData *)data;

	GString *parent_path = mf_ug_fm_svc_wrapper_get_file_parent_path(fullpath);
	GString *file_name = mf_ug_fm_svc_wapper_get_file_name(fullpath);

	if (parent_path == NULL || file_name == NULL) {
		return MYFILE_ERR_GENERATE_NAME_FAIL;
	}
	if (parent_path->str == NULL || file_name->str == NULL) {
		g_string_free(parent_path, TRUE);
		parent_path = NULL;
		g_string_free(file_name, TRUE);
		file_name = NULL;
		return MYFILE_ERR_GENERATE_NAME_FAIL;
	}

	char *name = NULL;
	int error_code = 0;

	if (parent_path->str != NULL) {
		/*mf_debug("parent_full_path is [%s]", parent_path->str);*/
	}

	if (file_name->str != NULL) {
		/*mf_debug("original_file_name is [%s]", file_name->str);*/
	}
	error_code = __mf_ug_fm_svc_wrapper_get_unique_name(parent_path->str, file_name->str, &name, file_name_type, ugd);
	if (error_code) {
		UG_SAFE_FREE_CHAR(name);
		return MYFILE_ERR_GENERATE_NAME_FAIL;
	}
	g_string_append_printf(parent_path, "/%s", name);
	ug_debug("After gstring append, PATH ::: [%s]", parent_path->str);

	if (file_name != NULL) {
		g_string_free(file_name, TRUE);
	}

	file_name = NULL;
	if (name != NULL) {
		free(name);
		name = NULL;
	}

	*filename = parent_path;
	return MYFILE_ERR_NONE;
}

int mf_ug_fm_svc_wrapper_create_service(void *data, GString *fullpath)
{
	int error_code;

	mf_ug_util_remove_dir_watch();
	error_code = mf_ug_fs_oper_create_dir(fullpath->str);

	if (error_code != 0) {
		ug_debug("Make DIR error\n");
	}

	return error_code;
}

int mf_ug_fm_svc_wrapper_create_p(const char *fullpath)
{
	UG_TRACE_BEGIN;

	int error_code = MYFILE_ERR_NONE;

	error_code = mf_ug_fs_oper_create_dir(fullpath);
	goto_if(error_code != MYFILE_ERR_NONE, EXIT);

EXIT:
	return error_code;
}

Eina_List *mf_ug_fm_svc_wrapper_level_path_get(const char *original_path)
{
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(original_path == NULL, NULL, "input path is NULL");

	char *current_path = g_strdup(original_path);
	char *temp_path = current_path;
	Eina_List *path_list = NULL;
	const char *root_path = NULL;

	ug_error("original_path is [%s]", original_path);
	int location = mf_ug_fm_svc_wapper_is_root_path(current_path);
	if (location == MF_UG_NONE) {
		location = mf_ug_fm_svc_wapper_get_location(current_path);
		switch (location) {
		case MF_UG_PHONE:
			root_path = PHONE_FOLDER;
			break;
		case MF_UG_MMC:
			root_path = MEMORY_FOLDER;
			break;
		default:
			UG_SAFE_FREE_CHAR(current_path);
			return NULL;
		}
		current_path = current_path + strlen(root_path) + 1;
		path_list = eina_list_append(path_list, g_strdup(root_path));
		gchar **result = NULL;
		gchar **params = NULL;
		result = g_strsplit(current_path, "/", 0);
		char *level_path = NULL;
		for (params = result; *params; params++) {
			if (level_path == NULL) {
				level_path = g_strconcat(root_path, "/", *params, NULL);
			} else {
				level_path = g_strconcat(level_path, "/", *params, NULL);
			}
			path_list = eina_list_append(path_list, level_path);
		}
		g_strfreev(result);
	} else {
		path_list = eina_list_append(path_list, g_strdup(original_path));
	}
	UG_TRACE_END;
	UG_SAFE_FREE_CHAR(temp_path);
	return path_list;
}

