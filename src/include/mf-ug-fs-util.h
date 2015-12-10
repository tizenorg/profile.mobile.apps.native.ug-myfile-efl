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

#ifndef __DEF_MF_UG_FS_UTIL_H_
#define __DEF_MF_UG_FS_UTIL_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>

#include "Eina.h"
#include "Elementary.h"
#include "media_content.h"
#include "mf-ug-dlog.h"

/*	File system related value definition	*/
#define	FILE_EXT_LEN_MAX			8
#define MYFILE_DIR_PATH_LEN_MAX		4096
#define MYFILE_FILE_NAME_LEN_MAX	255
#define MYFILE_FILE_PATH_LEN_MAX	MYFILE_DIR_PATH_LEN_MAX + MYFILE_FILE_NAME_LEN_MAX


/*	File system related String definition	*/
#define PHONE_FOLDER	"/opt/usr/media"
#define MEMORY_FOLDER	"/opt/storage/sdcard"
#define PHONE_PARENT	"/opt/usr"
#define PHONE_NAME		"media"
#define STORAGE_PARENT	"/opt/storage"
#define MMC_NAME		"sdcard"

#define SOUNDS_FOLDER		"/opt/usr/media/Sounds"
#define DEBUG_FOLDER		"SLP_debug"

#define MYFILE_NAME_PATTERN	"[\\:;*\"<>|?/]"

#define IMAGE_AND_VIDEO		"Images and videos"
#define SOUND_AND_MUSIC		"Sounds and music"
#define DOWNLOADS			"Downloads"
#define CAMERA_SHOTS		"Camera shots"


#define WALLPAPER			"Wallpapers"
#define MY_PHOTO_CLIPS		"My photo clips"
#define MY_ALBUM			"My album"          /*/_("IDS_MF_BODY_ACCESS_MORE_MY_ALBUM") */
#define MY_VIDEO_CLIPS		"My video clips"	/*/_("IDS_MF_BODY_MY_VIDEO_CLIPS") */

#define FM_RADIO			"FM Radio"	/*/dgettext("sys_string", "IDS_COM_HEADER_FMRADIO") */
#define MUSIC				"Music"     /*/_("IDS_MF_BODY_MUSIC") */
#define RINGTONES			"Ringtones"
#define VOICE_RECORDER		"Voice recorder"	/*/dgettext("sys_string", "IDS_COM_BODY_VOICE_RECORDER") */
#define ALERTS				"Alerts"
#define OTHERS				"Others"	/*/dgettext("sys_string","IDS_COM_BODY_OTHERS") */

/*	File system define default folder	*/

/*	compile option	*/
#define UG_DEBUG_FOLDER_OPTION

#ifndef UG_DEFAULT_ICON
#define UG_DEFAULT_ICON "myfile_icon_etc.png"
#endif

#define UG_ICON_FOLDER	"myfile_icon_folder.png"
#define UG_ICON_ITEM_SHORTCUT   "my_files_folder_favorite.png"

#define UG_ICON_IMAGE		"myfile_icon_images.png"
#define UG_ICON_VIDEO		"myfile_icon_video.png"
#define UG_ICON_MUSIC		"myfile_icon_music.png"
//#define UG_ICON_SOUND		"myfile_icon_music.png"
#define UG_ICON_PDF			"myfile_icon_pdf.png"
#define UG_ICON_DOC			"myfile_icon_word.png"
#define UG_ICON_PPT			"myfile_icon_ppt.png"
#define UG_ICON_EXCEL		"myfile_icon_excel.png"
//#define UG_ICON_VOICE		"myfile_icon_music.png"
#define UG_ICON_HTML		"myfile_icon_html.png"
#define UG_ICON_FLASH		"myfile_icon_swf.png"
#define UG_ICON_TXT			"myfile_icon_text.png"
#define UG_ICON_VCONTACT	"myfile_icon_vcard.png"
#define UG_ICON_VCALENDAR	"myfile_icon_vcalender.png"
//#define UG_ICON_VNOTE		"myfile_icon_text.png"
#define UG_ICON_RSS			"myfile_icon_rss.png"
#define UG_ICON_JAVA		"myfile_icon_java.png"
#define UG_ICON_TPK			"myfile_icon_tpk.png"
#define UG_ICON_SNB			"myfile_icon_snb.png"
#define UG_ICON_HWP			"myfile_icon_hwp.png"
#define UG_ICON_GUL			"myfile_icon_etc.png"
#define UG_ICON_MUSIC_PLAY_WHITE		"myfile_icon_control_play.png"
#define UG_ICON_MUSIC_PAUSE_WHITE		"myfile_icon_control_pause.png"
#define UG_ICON_MUSIC_PLAY_WHITE_PRESS	"myfile_icon_control_play_press.png"
#define UG_ICON_MUSIC_PAUSE_WHITE_PRESS	"myfile_icon_control_pause_press.png"
#define UG_ICON_ENTRY_FOLDER			"myfile_icon_entry_folder.png"
#define UG_ICON_ENTRY_FOLDER_PRESS		"myfile_icon_entry_folder_press.png"
#define UG_ICON_VIDEO_PLAY				"myfile_icon_video_play.png"

//#define UG_ICON_ITEM_PHONE	"myfile_icon_folder.png"
#define UG_ICON_ITEM_MMC	"myfile_icon_folder_sdcard.png"
#define UG_ICON_MMC			"myfile_icon_grid_folder_card.png"

#define UG_ICON_ADD				"myfile_icon_add.png"
#define UG_ICON_DELETE			"myfile_icon_delete.png"
#define UG_ICON_CREATE_FOLDER	"myfile_icon_create_folder.png"
#define UG_ICON_MULTI_NO_CONTENTS	"00_nocontents_multimedia.png"

#define UG_ICON_ITEM_ROOT_PHONE		"myfile_icon_root_folder_device_memory.png"
#define UG_ICON_ITEM_ROOT_MMC		"my_files_sd_card.png"

typedef enum _mf_ug_fs_file_type mf_ug_fs_file_type;

enum _mf_ug_fs_file_type {
	UG_FILE_TYPE_NONE = 0,

	UG_FILE_TYPE_DIR,       /**< Folder category */
	UG_FILE_TYPE_FILE,      /**< File category */
	UG_FILE_TYPE_IMAGE,     /**< Image category */
	UG_FILE_TYPE_VIDEO,     /**< Video category */
	UG_FILE_TYPE_MUSIC,     /**< Music category */

	UG_FILE_TYPE_SOUND,     /**< Sound category */
	UG_FILE_TYPE_PDF,       /**< Pdf category */
	UG_FILE_TYPE_DOC,       /**< Word category */
	UG_FILE_TYPE_PPT,       /**< Powerpoint category */
	UG_FILE_TYPE_EXCEL,     /**< Excel category */

	UG_FILE_TYPE_VOICE,     /**< Voice category */
	UG_FILE_TYPE_HTML,      /**< Html category */
	UG_FILE_TYPE_FLASH,	    /**< Flash category */
	UG_FILE_TYPE_GAME,	    /**< Game category */
	UG_FILE_TYPE_APP,	    /**< Application category */

	UG_FILE_TYPE_THEME,     /**< Theme category */
	UG_FILE_TYPE_TXT,       /**< Txt category */
	UG_FILE_TYPE_VCONTACT,  /**< Vcontact category */
	UG_FILE_TYPE_VCALENDAR, /**< Vcalendar category */
	UG_FILE_TYPE_VNOTE,     /**< Vnote category */

	UG_FILE_TYPE_VBOOKMARK,         /**< Vbookmark category */
	UG_FILE_TYPE_VIDEO_PROJECT,     /**< Video editor project category */
	UG_FILE_TYPE_RADIO_RECORDED,    /**< radio recorded clips category */
	UG_FILE_TYPE_MOVIE_MAKER,       /**< Movie maker project category */
	UG_FILE_TYPE_SVG,               /**< Svg category */

	UG_FILE_TYPE_RSS,	            /**< Rss reader file, *.opml */
	UG_FILE_TYPE_CERTIFICATION,     /**< certification file, *.pem */
	UG_FILE_TYPE_JAVA,	            /**< java file, *.jad, *.jar */
	UG_FILE_TYPE_WGT,	            /**< wrt , *.wgt, *.wgt */
	UG_FILE_TYPE_MP4_AUDIO,
	UG_FILE_TYPE_MP4_VIDEO,
	UG_FILE_TYPE_TPK,       /**< *.tpk>*/
	UG_FILE_TYPE_SNB,       /**<*.snb> */
	UG_FILE_TYPE_GUL,       /**<*.gul> */

	UG_FILE_TYPE_HWP,       /**<*.hwp> */
	UG_FILE_TYPE_ETC,       /**< Other files category */
	UG_FILE_TYPE_MAX
};

typedef enum _mf_ug_iter_category_filter_t mf_ug_iter_category_filter_t;
enum _mf_ug_iter_category_filter_t {
	UG_FILTER_CATEGORY_NONE = 0x00000000,		/**< Default */
	UG_FILTER_CATEGORY_IMAGE = 0x00000001,		/**< Image category */
	UG_FILTER_CATEGORY_VIDEO = 0x00000002,		/**< Video category */
	UG_FILTER_CATEGORY_SOUND = 0x00000004,		/**< Sound category */
	UG_FILTER_CATEGORY_VOICE = 0x00000008,		/**< Voice category */
	UG_FILTER_CATEGORY_MUSIC = 0x00000010,		/**< Music category */
	UG_FILTER_CATEGORY_HTML = 0x00000020,		/**< Html category */
	UG_FILTER_CATEGORY_FLASH = 0x00000040,		/**< Flash category */
	UG_FILTER_CATEGORY_GAME = 0x00000080,		/**< Game category */
	UG_FILTER_CATEGORY_APP = 0x00000100,		/**< Application category */
	UG_FILTER_CATEGORY_THEME = 0x00000200,		/**< Theme category */
	UG_FILTER_CATEGORY_DOC = 0x00000400,		/**< Word category */
	UG_FILTER_CATEGORY_EXCEL = 0x00000800,		/**< Excel category */
	UG_FILTER_CATEGORY_PPT = 0x00001000,		/**< Powerpoint category */
	UG_FILTER_CATEGORY_PDF = 0x00002000,		/**< Pdf category */
	UG_FILTER_CATEGORY_TXT = 0x00004000,		/**< Txt category */
	UG_FILTER_CATEGORY_VCONTACT = 0x00008000,	/**< Vcontact category */
	UG_FILTER_CATEGORY_VCALENDAR = 0x00010000,	/**< Vcalendar category */
	UG_FILTER_CATEGORY_VNOTE = 0x00020000,		/**< Vnote category */
	UG_FILTER_CATEGORY_VBOOKMARK = 0x00040000,	/**< Vbookmark category */
	UG_FILTER_CATEGORY_VIDEO_PROJECT = 0x00080000,	/**< Video editor project category */
	UG_FILTER_CATEGORY_SVG = 0x00100000,			/**< SVG category */
	UG_FILTER_CATEGORY_RSS = 0x00200000,			/**< RSS category */
	UG_FILTER_CATEGORY_ETC = 0x00400000,			/**< Other files category */
	UG_FILTER_CATEGORY_MP4_VIDEO = 0x00800000,
	UG_FILTER_CATEGORY_MP4_AUDIO = 0x0100000,
	UG_FILTER_CATEGORY_SNB = 0x0200000,
	UG_FILTER_CATEGORY_GUL = 0x0400000,
	UG_FILTER_CATEGORY_HWP = 0x0800000,
	UG_FILTER_CATEGORY_ALL = 0x8000000,
};


typedef enum _mf_ug_storage_type mf_ug_storage_type;
enum _mf_ug_storage_type {
	MF_UG_NONE,
	MF_UG_PHONE,
	MF_UG_MMC,
	MF_UG_MAX
};

typedef enum _mf_ug_sort_option mf_ug_sort_option;

enum _mf_ug_sort_option {
	MF_UG_SORT_BY_NONE = 0,		 /**< Sort by default */
	MF_UG_SORT_BY_NAME_A2Z,		 /**< Sort by file name ascending */
	MF_UG_SORT_BY_SIZE_S2L,		 /**< Sort by file size ascending */
	MF_UG_SORT_BY_DATE_O2R,		 /**< Sort by file date ascending */
	MF_UG_SORT_BY_TYPE_A2Z,		 /**< Sort by file type ascending */
	MF_UG_SORT_BY_NAME_Z2A,		 /**< Sort by file name descending */
	MF_UG_SORT_BY_SIZE_L2S,		 /**< Sort by file size descending */
	MF_UG_SORT_BY_DATE_R2O,		 /**< Sort by file date descending */
	MF_UG_SORT_BY_TYPE_Z2A,		 /**< Sort by file type descending */
	MF_UG_SORT_BY_MAX
} ;

typedef enum __MF_UG_SORT_BY_PRIORITY_SEQUENCE MF_UG_SORT_BY_PRIORITY_SEQUENCE;
enum __MF_UG_SORT_BY_PRIORITY_SEQUENCE {
	MF_UG_SORT_BY_PRIORITY_TYPE_A2Z,
	MF_UG_SORT_BY_PRIORITY_TYPE_Z2A,
	MF_UG_SORT_BY_PRIORITY_DATE_O2R,
	MF_UG_SORT_BY_PRIORITY_DATE_R2O,
	MF_UG_SORT_BY_PRIORITY_SIZE_S2L,
	MF_UG_SORT_BY_PRIORITY_SIZE_L2S,
};

/*  File operation error check options definition   */
#define	MF_ERROR_CHECK_SRC_ARG_VALID	0x0001
#define	MF_ERROR_CHECK_SRC_EXIST		0x0002
#define MF_ERROR_CHECK_SRC_PATH_VALID	0x0004
#define	MF_ERROR_CHECK_DUPLICATED		0x0008

/*	File system error definition	*/
#define MF_ERROR_MASKL16       0xFFFF
#define MF_ERROR_SET(X)        (X & MF_ERROR_MASKL16)
#define MID_CONTENTS_MGR_ERROR  0
#define MYFILE_ERR_NONE   (MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x00))	   /**< No error */

/*/1-10*/
#define MYFILE_ERR_SRC_ARG_INVALID		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x01))   /**< invalid src argument */
#define MYFILE_ERR_DST_ARG_INVALID		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x02))   /**< invalid dst argument */
#define MYFILE_ERR_DIR_OPEN_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x03))   /**< exception of dir open*/
#define MYFILE_ERR_INVALID_DIR_PATH		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x04))   /**< exception of invalid dir path */
#define MYFILE_ERR_INVALID_FILE_NAME	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x05))   /**< exception of invalid file name */
#define MYFILE_ERR_INVALID_FILE_PATH	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x06))   /**< exception of invalid file path */
#define MYFILE_ERR_SRC_NOT_EXIST		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x08))   /**< source not found */
#define MYFILE_ERR_STORAGE_TYPE_ERROR	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x09))   /**< storage type error */
#define MYFILE_ERR_EXT_GET_ERROR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0a))   /**< get ext type failed */
#define MYFILE_ERR_GET_STAT_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0b))   /**< get stat failed */
#define MYFILE_ERR_GET_CATEGORY_FAIL	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0c))   /**< get file category failed */
#define MYFILE_ERR_GET_CONF_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0d))   /**< get conf value failed */
#define MYFILE_ERR_INVALID_ARG			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0e))   /**< argument of function is not valid */
#define MYFILE_ERR_ALLOCATE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0f))
#define MYFILE_ERR_LIST_PLAY_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x10))
#define MYFILE_ERR_INVALID_PATH			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x11))   /**< invalid path string */
#define MYFILE_ERR_GET_THUMBNAIL_FAILED	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x12))   /**<get thumbnail failed */
#define MYFILE_ERR_UNKNOW_ERROR			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x13))   /**<unknow error */
#define MYFILE_ERR_NO_FREE_SPACE		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x14))   /**< get free space failed */
#define MYFILE_ERR_DUPLICATED_NAME		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x15))    /**< exception of duplicated dir name*/
#define MYFILE_ERR_ALLOCATE_MEMORY_FAIL	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x16))	/**< exception of memory allocation */
#define MYFILE_ERR_GET_LOGIC_PATH_FAIL	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x17))	/**< get logical path failed */
#define MYFILE_ERR_GENERATE_NAME_FAIL	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x18))	/**< generate name failed */
#define MYFILE_ERR_DIR_CREATE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x19))	/**< exception of create dir */
#define MYFILE_ERR_GET_PARENT_PATH_FAIL	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1a))	/**< get parent path failed */
#define MYFILE_ERR_EXCEED_MAX_LENGTH	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1b))   /**< length of file/dir path exceeds maximum length*/

/*	File system related callback definition	*/
typedef struct _ugFsNodeInfo ugFsNodeInfo;
struct _ugFsNodeInfo {
	char *path;
	char *name;
	time_t date;
	mf_ug_fs_file_type type;
	char *ext;
	unsigned int size;
	int storage_type;
};


/**********			File Attribute Related			**********/
int mf_ug_file_attr_get_file_stat(const char *filename, ugFsNodeInfo ** node);
int mf_ug_file_attr_get_file_category(char *filepath, mf_ug_fs_file_type * category);
int mf_ug_file_attr_is_dir(const char *filepath);
int mf_ug_file_attr_get_store_type_by_full(const char *filepath, mf_ug_storage_type * store_type);
int mf_ug_file_attr_get_file_ext(const char *filepath, char **file_ext);
int mf_ug_file_attr_is_right_dir_path(const char *dir_path);
int mf_ug_file_attr_is_right_file_path(const char *file_path);
int mf_ug_file_attr_is_duplicated_name(const char *dir, const char *name);
int mf_ug_file_attr_get_logical_path_by_full(const char *full_path, char **path);
int mf_ug_file_attr_is_valid_name(const char *filename);
char *mf_ug_file_attr_default_icon_get_by_type(mf_ug_fs_file_type ftype);
int mf_ug_file_attr_is_system_dir(char *fullpath, bool * result);
char *mf_ug_file_attr_sound_title_get(const char *fullpath);
int mf_ug_file_attr_get_file_size(const char *filename, off_t *size);
int mf_ug_file_attr_get_file_icon(char *file_path, int *error_code, char **thumbnail,
                                  media_info_h *media_info);
mf_ug_fs_file_type mf_ug_file_attr_get_file_type(const char *mime);
mf_ug_fs_file_type mf_ug_file_attr_get_file_type_by_mime(const char *file_path);

/**********			File Operation Related			**********/
int mf_ug_fs_oper_read_dir(char *path, Eina_List **dir_list, Eina_List **file_list);
int mf_ug_fs_oper_list_filter(Eina_List *in_list, Eina_List **out_list, int option);
int mf_ug_fs_oper_list_filter_by_extension(Eina_List *in_list, Eina_List **out_list, char *ext);
int mf_ug_fs_oper_create_dir(const char *dir);
void mf_ug_fs_oper_sort_list(Eina_List **list, int sort_opt);


#endif
