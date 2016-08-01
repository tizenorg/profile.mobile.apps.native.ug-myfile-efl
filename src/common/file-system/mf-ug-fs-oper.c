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




#include <libgen.h>
#include <glib.h>
#include "mf-ug-fs-util.h"
#include "mf-ug-util.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-file-util.h"

static int __mf_ug_fs_oper_sort_by_date_cb_O2R(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_name_cb_A2Z(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_type_cb_A2Z(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_size_cb_S2L(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_name_cb_Z2A(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_date_cb_R2O(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_type_cb_Z2A(const void *d1, const void *d2);
static int __mf_ug_fs_oper_sort_by_size_cb_L2S(const void *d1, const void *d2);

/*********************
**Function name:	__mf_ug_fs_oper_file_system_error
**Parameter:
**	const char* src:	source path
**	const char* dst:	destination path
**	int check_option:	check option
**
**Return value:
**	error code
**
**Action:
**	input parameter checking
**
*********************/
static const char *__mf_ug_fs_oper_get_file(const char *path)
{
	char *result = NULL;

	if (!path) {
		return NULL;
	}
	if ((result = strrchr(path, '/'))) {
		result++;
	} else {
		result = (char *)path;
	}
	return result;
}


static int __mf_ug_fs_oper_file_system_error(const char *src, const char *dst, int check_option)
{
	if ((check_option & MF_ERROR_CHECK_SRC_ARG_VALID) && (src == NULL)) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}
	if ((check_option & MF_ERROR_CHECK_SRC_EXIST) && (!mf_file_exists(src))) {
		return MYFILE_ERR_SRC_NOT_EXIST;
	}

	if (check_option & MF_ERROR_CHECK_SRC_PATH_VALID) {
		if (!mf_is_dir(src)) {
			if (mf_ug_file_attr_is_right_file_path(src)) {
				return MYFILE_ERR_INVALID_FILE_PATH;
			}
		} else {
			if (mf_ug_file_attr_is_right_dir_path(src)) {
				return MYFILE_ERR_INVALID_DIR_PATH;
			}
		}
	}

	if (check_option & MF_ERROR_CHECK_DUPLICATED) {
		char *parent_path = NULL;

		if (!mf_ug_file_attr_get_parent_path(dst, &parent_path)) {
			if (mf_ug_file_attr_is_duplicated_name(parent_path, __mf_ug_fs_oper_get_file(dst))) {
				UG_SAFE_FREE_CHAR(parent_path);
				return MYFILE_ERR_DUPLICATED_NAME;
			}
			UG_SAFE_FREE_CHAR(parent_path);
		} else {
			UG_SAFE_FREE_CHAR(parent_path);
			return MYFILE_ERR_GET_PARENT_PATH_FAIL;
		}
	}
	return MYFILE_ERR_NONE;
}

/*********************
**Function name:	mf_ug_fs_oper_read_dir
**Parameter:
**	char *path:				path which we need to read
**	Eina_List** dir_list:	output parameter of dir list under specified path
**	Eina_List** file_list:	output parameter of file list under specified path
**
**Return value:
**	error code
**
**Action:
**	read element under the specified path
**
*********************/
int mf_ug_fs_oper_read_dir(char *path, Eina_List **dir_list, Eina_List **file_list)
{
	UG_TRACE_BEGIN;
	DIR *pDir = NULL;
	struct dirent *ent;
	struct dirent ent_struct;

	ug_mf_retvm_if(path == NULL, MYFILE_ERR_INVALID_ARG, "path is null");
	ug_mf_retvm_if(dir_list == NULL, MYFILE_ERR_INVALID_ARG, "dir_list is null");
	ug_mf_retvm_if(file_list == NULL, MYFILE_ERR_INVALID_ARG, "file_list is null");

	int option = MF_ERROR_CHECK_SRC_ARG_VALID | MF_ERROR_CHECK_SRC_EXIST | MF_ERROR_CHECK_SRC_PATH_VALID;
	int ret = __mf_ug_fs_oper_file_system_error(path, NULL, option);
	int storage_type = mf_ug_fm_svc_wapper_get_location(path);

	if (ret != MYFILE_ERR_NONE) {
		return ret;
	}

	pDir = opendir(path);

	if (pDir == NULL) {
		ug_error("could not open %s", path);
		return MYFILE_ERR_DIR_OPEN_FAIL;
	}
	while ((readdir_r(pDir, &ent_struct, &ent) == 0) && ent) {
		GString *childpath = NULL;
		ugFsNodeInfo *pNode = NULL;

		if (strncmp(ent->d_name, ".", 1) == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}

		if ((ent->d_type & DT_DIR) == 0 && (ent->d_type & DT_REG) == 0) {
			continue;
		}
#ifdef	UG_DEBUG_FOLDER_OPTION
		if ((ent->d_type & DT_DIR) != 0) {
			if (PHONE_FOLDER != NULL) {
				if ((strlen(path) == strlen(PHONE_FOLDER)) && (strcmp(path, PHONE_FOLDER) == 0)
						&& (strlen(ent->d_name) == strlen(DEBUG_FOLDER)) && (strcmp(ent->d_name, DEBUG_FOLDER) == 0)) {
					continue;
				}
			}
		}
#endif
		pNode = (ugFsNodeInfo *) malloc(sizeof(ugFsNodeInfo));

		if (pNode == NULL) {
			continue;
		}
		memset(pNode, 0, sizeof(ugFsNodeInfo));
		/*set path */
		pNode->path = g_strdup(path);
		/*set name */
		pNode->name = g_strdup(ent->d_name);
		pNode->storage_type = storage_type;
		if (ent->d_type & DT_DIR) {
			pNode->type = UG_FILE_TYPE_DIR;
		} else if (ent->d_type & DT_REG) {
			char *real_name = g_strconcat(pNode->path, "/", ent->d_name, NULL);

			if (real_name != NULL) {
				mf_ug_file_attr_get_file_category(real_name, &(pNode->type));
				SECURE_DEBUG(" file is [%s] type is [%d]", real_name, (pNode->type));
				UG_SAFE_FREE_CHAR(real_name);
			}
		}
		childpath = g_string_new(path);
		if (childpath == NULL) {
			free(pNode);
			pNode = NULL;
			continue;
		}
		g_string_append_printf(childpath, "/%s", ent->d_name);
		mf_ug_file_attr_get_file_stat(childpath->str, &pNode);
		if (pNode->type == UG_FILE_TYPE_DIR) {
			ug_mf_debug("dir append\n");
			*dir_list = eina_list_append(*dir_list, pNode);
		} else {
			ug_mf_debug("file append\n");
			ret = mf_ug_file_attr_get_file_ext(childpath->str, &pNode->ext);
			if (ret != MYFILE_ERR_NONE) {
				pNode->ext = NULL;
			}
			*file_list = eina_list_append(*file_list, pNode);
		}

		g_string_free(childpath, TRUE);
	}
	closedir(pDir);
	UG_TRACE_END;

	return MYFILE_ERR_NONE;
}

static bool __mf_ug_fs_oper_exec_filter(ugFsNodeInfo *pnode_info, int option)
{
	if (option == UG_FILTER_CATEGORY_ALL) {
		return TRUE;
	}
	if (pnode_info == NULL) {
		return FALSE;
	}
	if (option & UG_FILTER_CATEGORY_IMAGE) {
		if (pnode_info->type == UG_FILE_TYPE_IMAGE) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VIDEO) {
		if (pnode_info->type == UG_FILE_TYPE_VIDEO) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_SOUND) {
		if (pnode_info->type == UG_FILE_TYPE_SOUND) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VOICE) {
		if (pnode_info->type == UG_FILE_TYPE_VOICE) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_MUSIC) {
		if (pnode_info->type == UG_FILE_TYPE_MUSIC) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_HTML) {
		if (pnode_info->type == UG_FILE_TYPE_HTML) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_FLASH) {
		if (pnode_info->type == UG_FILE_TYPE_FLASH) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_GAME) {
		if (pnode_info->type == UG_FILE_TYPE_GAME) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_APP) {
		if (pnode_info->type == UG_FILE_TYPE_APP) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_THEME) {
		if (pnode_info->type == UG_FILE_TYPE_THEME) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_DOC) {
		if (pnode_info->type == UG_FILE_TYPE_DOC) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_EXCEL) {
		if (pnode_info->type == UG_FILE_TYPE_EXCEL) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_PPT) {
		if (pnode_info->type == UG_FILE_TYPE_PPT) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_PDF) {
		if (pnode_info->type == UG_FILE_TYPE_PDF) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_TXT) {
		if (pnode_info->type == UG_FILE_TYPE_TXT) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VCONTACT) {
		if (pnode_info->type == UG_FILE_TYPE_VCONTACT) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VCALENDAR) {
		if (pnode_info->type == UG_FILE_TYPE_VCALENDAR) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VNOTE) {
		if (pnode_info->type == UG_FILE_TYPE_VNOTE) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VBOOKMARK) {
		if (pnode_info->type == UG_FILE_TYPE_VBOOKMARK) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_VIDEO_PROJECT) {
		if (pnode_info->type == UG_FILE_TYPE_VIDEO_PROJECT) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_SVG) {
		if (pnode_info->type == UG_FILE_TYPE_SVG) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_RSS) {
		if (pnode_info->type == UG_FILE_TYPE_RSS) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_ETC) {
		if (pnode_info->type == UG_FILE_TYPE_ETC) {
			return TRUE;
		}
	}

	if (option & UG_FILTER_CATEGORY_MP4_AUDIO) {
		if (pnode_info->type == UG_FILE_TYPE_MP4_AUDIO) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_MP4_VIDEO) {
		if (pnode_info->type == UG_FILE_TYPE_MP4_VIDEO) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_HWP) {
		if (pnode_info->type == UG_FILE_TYPE_HWP) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_SNB) {
		if (pnode_info->type == UG_FILE_TYPE_SNB) {
			return TRUE;
		}
	}
	if (option & UG_FILTER_CATEGORY_GUL) {
		if (pnode_info->type == UG_FILE_TYPE_GUL) {
			return TRUE;
		}
	}
	return FALSE;
}

int mf_ug_fs_oper_list_filter(Eina_List *in_list, Eina_List **out_list, int option)
{
	ug_mf_debug();
	if (in_list == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (out_list == NULL) {
		return MYFILE_ERR_DST_ARG_INVALID;
	}

	if (option == 0) {
		*out_list = in_list;
		return MYFILE_ERR_NONE;
	}

	Eina_List *l = NULL;
	ugFsNodeInfo *data = NULL;
	EINA_LIST_FOREACH(in_list, l, data) {
		if (__mf_ug_fs_oper_exec_filter(data, option)) {
			*out_list = eina_list_append(*out_list, data);
		}
	}
	return MYFILE_ERR_NONE;
}

/******************************
** Prototype    : ug_mf_list_filter_by_extention
** Description  : filter from list by extension
** Input        : Eina_List *in_list
**                Eina_List **out_list
**                char* ext
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
int mf_ug_fs_oper_list_filter_by_extension(Eina_List *in_list, Eina_List **out_list, char *ext)
{
	if (in_list == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (out_list == NULL) {
		return MYFILE_ERR_DST_ARG_INVALID;
	}

	if (ext == NULL) {
		*out_list = in_list;
		return MYFILE_ERR_NONE;
	}

	Eina_List *l = NULL;
	ugFsNodeInfo *data = NULL;

	char *seps = ";";
	char *temp_ext = malloc(strlen(ext) + 1);
	if (temp_ext == NULL) {
		return MYFILE_ERR_ALLOCATE_FAIL;
	}

	gchar **result = NULL;
	gchar **params = NULL;

	EINA_LIST_FOREACH(in_list, l, data) {
		if (data) {
			memset(temp_ext, 0, strlen(ext) + 1);
			strncpy(temp_ext, ext, strlen(ext));
			result = g_strsplit(temp_ext, seps, 0);
			if (result == NULL) {
				continue;
			}
			for (params = result; *params; params++) {
				if (data->ext == NULL) {
					break;
				}
				if (strcasecmp(data->ext, *params) == 0) {
					*out_list = eina_list_append(*out_list, data);
					break;
				}
			}

			g_strfreev(result);
			result = NULL;
		}
	}
	free(temp_ext);
	return MYFILE_ERR_NONE;
}

static int __mf_ug_fs_oper_sort_by_priority(const void *d1, const void *d2, int sequence_type)
{
	int ret = 0;
	switch (sequence_type) {
	case MF_UG_SORT_BY_PRIORITY_TYPE_A2Z:
		ret = __mf_ug_fs_oper_sort_by_date_cb_O2R(d1, d2);
		if (ret == 0) {
			ret = __mf_ug_fs_oper_sort_by_size_cb_S2L(d1, d2);
			if (ret == 0) {
				ret = __mf_ug_fs_oper_sort_by_name_cb_A2Z(d1, d2);
			}
		}
		break;
	case MF_UG_SORT_BY_PRIORITY_TYPE_Z2A:
		ret = __mf_ug_fs_oper_sort_by_date_cb_R2O(d1, d2);
		if (ret == 0) {
			ret = __mf_ug_fs_oper_sort_by_size_cb_L2S(d1, d2);
			if (ret == 0) {
				ret = __mf_ug_fs_oper_sort_by_name_cb_Z2A(d1, d2);
			}
		}
		break;
	case MF_UG_SORT_BY_PRIORITY_DATE_O2R:
		ret = __mf_ug_fs_oper_sort_by_size_cb_S2L(d1, d2);
		if (ret == 0) {
			ret = __mf_ug_fs_oper_sort_by_name_cb_A2Z(d1, d2);
		}
		break;
	case MF_UG_SORT_BY_PRIORITY_DATE_R2O:
		ret = __mf_ug_fs_oper_sort_by_size_cb_L2S(d1, d2);
		if (ret == 0) {
			ret = __mf_ug_fs_oper_sort_by_name_cb_Z2A(d1, d2);
		}
		break;
	case MF_UG_SORT_BY_PRIORITY_SIZE_S2L:
		ret = __mf_ug_fs_oper_sort_by_name_cb_A2Z(d1, d2);
		break;
	case MF_UG_SORT_BY_PRIORITY_SIZE_L2S:
		ret = __mf_ug_fs_oper_sort_by_name_cb_Z2A(d1, d2);
		break;
	default:
		break;
	}
	return ret;
}
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the Assic table

**
*********************/
static int __mf_ug_fs_oper_sort_by_name_cb_A2Z(const void *d1, const void *d2)
{
	ugFsNodeInfo *txt1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *txt2 = (ugFsNodeInfo *) d2;
	gchar *name1 = NULL;
	gchar *name2 = NULL;
	int result = 0;

	if (!txt1) {
		return (1);
	}
	if (!txt2) {
		return (-1);
	}

	name1 = g_ascii_strdown(txt1->name, strlen(txt1->name));
	if (name1 == NULL) {
		return (-1);
	}
	name2 = g_ascii_strdown(txt2->name, strlen(txt2->name));
	if (name2 == NULL) {
		g_free(name1);
		name1 = NULL;
		return (-1);
	}
	result = g_strcmp0(name1, name2);

	g_free(name1);
	name1 = NULL;
	g_free(name2);
	name2 = NULL;
	return result;

}

/*********************
**Function name:	__sort_by_date_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the later created the later shown
*********************/
static int __mf_ug_fs_oper_sort_by_date_cb_O2R(const void *d1, const void *d2)
{
	int ret = 0;
	ugFsNodeInfo *time1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *time2 = (ugFsNodeInfo *) d2;

	if (!d1) {
		return 1;
	}
	if (!d2) {
		return -1;
	}

	if (time1->date > time2->date) {
		ret = 1;
	} else if (time1->date < time2->date) {
		ret = -1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_ug_fs_oper_sort_by_priority(d1, d2, MF_UG_SORT_BY_PRIORITY_DATE_O2R);
	}
	return ret;
}

/*********************
**Function name:	__sort_by_type_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 < d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the category type value
*********************/
static int __mf_ug_fs_oper_sort_by_type_cb_A2Z(const void *d1, const void *d2)
{
	ugFsNodeInfo *type1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *type2 = (ugFsNodeInfo *) d2;
	gchar *ext1 = NULL;
	gchar *ext2 = NULL;
	int result = 0;

	if (type1 == NULL || type1->ext == NULL) {
		return 1;
	}

	if (type2 == NULL || type2->ext == NULL) {
		return -1;
	}
	ext1 = g_ascii_strdown(type1->ext, strlen(type1->ext));
	if (ext1 == NULL) {
		return (-1);
	}
	ext2 = g_ascii_strdown(type2->ext, strlen(type2->ext));
	if (ext2 == NULL) {
		g_free(ext1);
		ext1 = NULL;
		return (-1);
	}
	result = g_strcmp0(ext1, ext2);

	g_free(ext1);
	ext1 = NULL;
	g_free(ext2);
	ext2 = NULL;

	if (result == 0) {
		result = __mf_ug_fs_oper_sort_by_priority(d1, d2, MF_UG_SORT_BY_PRIORITY_TYPE_A2Z);
	}

	return result;
}

/*order:	the one with smaller size will be shown earlier*/
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by size, rule is the smaller the later shown
*********************/
static int __mf_ug_fs_oper_sort_by_size_cb_S2L(const void *d1, const void *d2)
{
	int ret = 0;
	ugFsNodeInfo *size1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *size2 = (ugFsNodeInfo *) d2;

	if (!d1) {
		return 1;
	}

	if (!d2) {
		return -1;
	}

	if (size1->size > size2->size) {
		ret = 1;
	} else if (size1->size < size2->size) {
		ret = -1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_ug_fs_oper_sort_by_priority(d1, d2, MF_UG_SORT_BY_PRIORITY_SIZE_S2L);
	}
	return ret;
}

/*********************
**Function name:	__mf_fs_oper_sort_by_name_cb_Z2A
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	1	if d1 > d2
**	-1	if d1 <= d2
**
**Action:
**	sort the list order by the Assic table

**
*********************/
static int __mf_ug_fs_oper_sort_by_name_cb_Z2A(const void *d1, const void *d2)
{
	ugFsNodeInfo *txt1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *txt2 = (ugFsNodeInfo *) d2;

	int result = 0;

	if (!txt1) {
		return (1);
	}
	if (!txt2) {
		return (-1);
	}
	result = strcasecmp(txt1->name, txt2->name);

	if (result < 0) {
		return (1);
	} else {
		return (-1);
	}
}

/*********************
**Function name:	__sort_by_date_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by the later created the later shown
*********************/
static int __mf_ug_fs_oper_sort_by_date_cb_R2O(const void *d1, const void *d2)
{
	int ret = 0;
	ugFsNodeInfo *time1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *time2 = (ugFsNodeInfo *) d2;

	if (!d1) {
		return -1;
	}
	if (!d2) {
		return 1;
	}
	if (time1->date > time2->date) {
		ret = -1;
	} else if (time1->date < time2->date) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_ug_fs_oper_sort_by_priority(d1, d2, MF_UG_SORT_BY_PRIORITY_DATE_R2O);
	}
	return ret;
}

/*********************
**Function name:	__sort_by_type_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by the category type value
*********************/
static int __mf_ug_fs_oper_sort_by_type_cb_Z2A(const void *d1, const void *d2)
{
	ugFsNodeInfo *type1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *type2 = (ugFsNodeInfo *) d2;
	gchar *ext1 = NULL;
	gchar *ext2 = NULL;
	int result = 0;

	if (type1 == NULL || type1->ext == NULL) {
		return -1;
	}

	if (type2 == NULL || type2->ext == NULL) {
		return 1;
	}

	ext1 = g_ascii_strdown(type1->ext, strlen(type1->ext));
	if (ext1 == NULL) {
		return (1);
	}
	ext2 = g_ascii_strdown(type2->ext, strlen(type2->ext));
	if (ext2 == NULL) {
		g_free(ext1);
		ext1 = NULL;
		return (-1);
	}
	result = g_strcmp0(ext1, ext2);
	g_free(ext1);
	ext1 = NULL;
	g_free(ext2);
	ext2 = NULL;
	if (result == 0) {
		result = __mf_ug_fs_oper_sort_by_priority(d1, d2, MF_UG_SORT_BY_PRIORITY_TYPE_Z2A);
	}

	return -result;
}

/*order:	the one with smaller size will be shown earlier*/
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by size, rule is the smaller the later shown
*********************/
static int __mf_ug_fs_oper_sort_by_size_cb_L2S(const void *d1, const void *d2)
{
	int ret = 0;
	ugFsNodeInfo *size1 = (ugFsNodeInfo *) d1;
	ugFsNodeInfo *size2 = (ugFsNodeInfo *) d2;

	if (!d1) {
		return -1;
	}

	if (!d2) {
		return 1;
	}

	if (size1->size > size2->size) {
		ret = -1;
	} else if (size1->size < size2->size) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_ug_fs_oper_sort_by_priority(d1, d2, MF_UG_SORT_BY_PRIORITY_SIZE_L2S);
	}
	return ret;
}

/*********************
**Function name:	mf_fs_oper_sort_list
**Parameter:
**	Eina_List **list:	the list we need to sort
**	int sort_opt:		sort option
**
**Return value:
**	void
**
**Action:
**	sort the list order by sort option with the call back
*********************/
void mf_ug_fs_oper_sort_list(Eina_List **list, int sort_opt)
{
	Eina_Compare_Cb sort_func = NULL;
	if (!(*list)) {
		return;
	}
	switch (sort_opt) {
	case MF_UG_SORT_BY_NAME_A2Z:
		sort_func = __mf_ug_fs_oper_sort_by_name_cb_A2Z;
		break;
	case MF_UG_SORT_BY_TYPE_A2Z:
		sort_func = __mf_ug_fs_oper_sort_by_type_cb_A2Z;
		break;
	case MF_UG_SORT_BY_SIZE_S2L:
		sort_func = __mf_ug_fs_oper_sort_by_size_cb_S2L;
		break;
	case MF_UG_SORT_BY_DATE_O2R:
		sort_func = __mf_ug_fs_oper_sort_by_date_cb_O2R;
		break;
	case MF_UG_SORT_BY_NAME_Z2A:
		sort_func = __mf_ug_fs_oper_sort_by_name_cb_Z2A;
		break;
	case MF_UG_SORT_BY_TYPE_Z2A:
		sort_func = __mf_ug_fs_oper_sort_by_type_cb_Z2A;
		break;
	case MF_UG_SORT_BY_SIZE_L2S:
		sort_func = __mf_ug_fs_oper_sort_by_size_cb_L2S;
		break;
	case MF_UG_SORT_BY_DATE_R2O:
		sort_func = __mf_ug_fs_oper_sort_by_date_cb_R2O;
		break;
	default:
		sort_func = __mf_ug_fs_oper_sort_by_type_cb_A2Z;
		break;
	}
	*list = eina_list_sort(*list, eina_list_count(*list), sort_func);
}

int mf_ug_fs_oper_create_dir(const char *dir)
{
	int option = MF_ERROR_CHECK_SRC_ARG_VALID | MF_ERROR_CHECK_DUPLICATED;
	int ret = __mf_ug_fs_oper_file_system_error(dir, dir, option);

	if (ret != 0) {
		return ret;
	}

	ret = mf_ug_file_attr_is_right_dir_path(dir);

	if (ret != 0) {
		return ret;
	}
	if (!mf_mkpath(dir)) {
		return MYFILE_ERR_DIR_CREATE_FAIL;
	}

	return MYFILE_ERR_NONE;
}

