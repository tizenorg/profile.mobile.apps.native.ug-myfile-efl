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






#ifndef __DEF_MF_UG_CONF_H_
#define __DEF_MF_UG_CONF_H_

#include <Elementary.h>
#include <app_common.h>

static inline char * mf_get_resource_path() {
	char * path = app_get_resource_path();
	return path;
}

static inline char* full_path(char *str1, char *str2) {
	char path[1024] = {};
	snprintf(path, 1024, "%s%s", str1, str2);
	char *full_path = strdup(path);
	return full_path;
}

#define PKGNAME_SYSTEM				"sys_string"
#define UGPACKAGE				"ug-myfile-efl"
#define UGPKGNAME_MYFILE			"org.tizen.myfile"
#define UGLOCALEDIR				"/usr/ug/res/locale"
#define MF_IMAGE_HEAD				"myfile_"

#define UG_EDJE_RES_PATH 		mf_get_resource_path()
#define UG_EDJ_PATH				full_path(UG_EDJE_RES_PATH, "edje")

#define UG_EDJ_NAVIGATIONBAR		full_path(UG_EDJ_PATH, "/ug_navibar_layout.edj")
#define UG_EDJ_IMAGE				 full_path(UG_EDJ_PATH, "/ug_edc_image_macro.edj")


#define UG_GRP_LIST				"thumbnail_only"
#define UG_GRP_NAVI_VIEW			"navigation_view"
#define UG_GRP_NO_CONTENT			"noContent"
#define UG_OPTION_COUNT				8
#define UG_ERROR_RETURN				(-1)

#define UG_SELECT_MODE_MULTI_ALL		"MULTI_ALL"
#define UG_SELECT_MODE_SINGLE_ALL		"SINGLE_ALL"
#define UG_SELECT_MODE_MULTI_FILE		"MULTI_FILE"
#define UG_SELECT_MODE_SINGLE_FILE		"SINGLE_FILE"
#define UG_SELECT_MODE_IMPORT			"IMPORT"
#define UG_SELECT_MODE_IMPORT_PATH_SELECT	"IMPORT_PATH_SELECT"
#define UG_SELECT_MODE_IMPORT_SINGLE			"IMPORT_SINGLE"
#define UG_SELECT_MODE_EXPORT			"EXPORT"
#define UG_SELECT_MODE_SHORTCUT			"SHORTCUT"
#define UG_SELECT_MODE_SAVE			"SAVE"
#define UG_SELECT_MODE_DOCUMENT_SHARE		"DOCUMENT_SHARE"
#define UG_SELECT_MODE_SSM_DOCUMENT_SHARE	"SSM_DOCUMENT_SHARE"

#define UG_FILE_FILTER_IMAGE			"IMAGE"
#define UG_FILE_FILTER_SOUND			"SOUND"
#define UG_FILE_FILTER_VIDEO			"VIDEO"
#define UG_FILE_FILTER_FLASH			"FLASH"
#define UG_FILE_FILTER_FOLDER			"FOLDER"
#define UG_FILE_FILTER_DOCUMENT			"DOCUMENT"
#define UG_FILE_FILTER_IV			"IV"
#define UG_FILE_FILTER_IS			"IS"
#define UG_FILE_FILTER_VS			"VS"
#define UG_FILE_FILTER_ALL			"ALL"
#define UG_FILE_MIME_TYPE_IMAGE			"image/*"
#define UG_FILE_MIME_TYPE_VIDEO			"video/*"
#define UG_FILE_MIME_TYPE_AUDIO			"audio/*"
#define UG_FILE_MIME_TYPE_DOCUMENT		"document/*"
#define UG_FILE_MIME_TYPE_ALL			"*/*"
#define UG_FILE_MIME_TYPE_DIR			"inode/directory"


#define UG_VIEW_MODE_DEFAULT_SOUND_ITEM			"DEFAULT_SOUND_ITEM"

#define MESSAGE 					"message"
#define UG_MUSIC_PATH				"/opt/usr/media/Sounds and music/Music"
#define UG_RINGTION_PATH			"/opt/usr/media/Sounds and music/Ringtones"
#define UG_SETTING_RINGTONE_PATH		"/opt/usr/share/settings/Ringtones"
#define UG_SETTING_MSG_ALERTS_PATH		"/opt/usr/share/settings/Alerts"
#define UG_SETTING_ALERTS_PATH			"/opt/usr/share/settings/Alarms"
#define UG_SETTING_SMART_ALRAMS			"/opt/usr/share/settings/Smartalarms"
#define UG_SETTING_DEFAULT_RINGTONE_PATH	"/opt/usr/share/settings/Ringtones/Over_the_horizon.ogg"
#define UG_SETTING_DEFAULT_ALERT_PATH		"/opt/usr/share/settings/Alerts/Beep_Once.ogg"


#define MAX_MESSAGE_LEN				1024
#define ACCUMULATED_DATE			86400	/* 24*60*60 */

#define UG_LABEL_STRING_LENGTH			128

#define UG_TIME_FORMAT "02u:%02u:%02u"
#define UG_TIME_ARGS(t) \
	(t) / (3600), \
	((t) / 60) % 60, \
	(t) % 60

#define _EDJ(o) elm_layout_edje_get(o)

#define INTERVAL_THUMB_UPDATE	0.5

#endif /* __DEF_MYFILE_CONF_H_ */
