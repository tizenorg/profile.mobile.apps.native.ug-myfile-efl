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

#ifndef __MF_UG_MEDIA_TYPES_H_DEF__
#define __MF_UG_MEDIA_TYPES_H_DEF__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef void MFDHandle;		/**< Handle */

/**
 *@enum GM_CONTENT_TYPE
 * Enumerations of  GM_CONTENT_TYPE
 */

#if 1
//1 Shortcut
typedef struct _MFSitem
{
	char *path;
	char *name;
}MFSitem;

typedef struct _MFRitem
{
	char *path;
	char *name;
	int storyage_type;
	char *thumbnail;
}MFRitem;

typedef struct _mfRingtone
{
	char *path;
	char *name;
}mfRingtone;


typedef bool (*mf_shortcut_item_cb)(MFSitem *Sitem, void *user_data);
typedef bool (*mf_recent_files_item_cb)(MFRitem *Ritem, void *user_data);
typedef bool (*mf_ringtone_item_cb)(mfRingtone *Ritem, void *user_data);

#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__MF_UG_MEDIA_TYPES_H_DEF__*/


