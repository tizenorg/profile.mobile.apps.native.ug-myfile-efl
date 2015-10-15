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

#ifndef __DEF_MF_UG_MAIN_H_
#define __DEF_MF_UG_MAIN_H_

#define __ARM__

#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>

#include <app.h>
#include <Ecore.h>
#include <Elementary.h>
#include <Ethumb.h>
#include <player.h>
#include <ui-gadget-module.h>
#include <ui-gadget.h>
#include <device/power.h>
#include <device/callback.h>
#include <media_content.h>

#include "mf-ug-dlog.h"
#include "mf-ug-conf.h"
#include "mf-ug-search.h"
#include "mf-ug-media-types.h"
/***********	Global Definitions		***********/
#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#define SILENT      "silent"
#define SILENT_SHOW "silent show"
#define DEFAULT_RINGTONE_MARK   "default ringtone"

typedef void (*ugCallBack) (void *, Evas_Object *, void *);

typedef struct _ugMainWindow ugMainWindow;
struct _ugMainWindow {
	Evas_Object *ug_pWindow;
	Evas_Object *ug_pBackGround;
	Evas_Object *ug_pTabBackGround;
	Evas_Object *ug_pMainLayout;
	Evas_Object *ug_pConformant;
	Evas_Object *ug_pNormalPopup;
	Evas_Object *ug_pSearchLabel;
	Evas_Object *ug_pSelectInfoLayout;
	Evas_Object *ug_pRadioGroup;

	Evas_Object *ug_pNaviBar;
	Evas_Object *ug_pNaviLayout;
	Evas_Object *ug_pNaviGenlist;
	Evas_Object *ug_pNaviCtrlBar;
	Evas_Object *ug_pNaviBox;

	Evas_Object *ug_pEditField;
	Evas_Object *ug_pEntry;
	Evas_Object *ug_pSelectAllLayout;
	Evas_Object *ug_pSelectAllCheckBox;
	Evas_Object *ug_pSearchPopup;
	Evas_Object *ug_pContextPopup;
	Evas_Object *ug_pNewFolderPopup;
	Evas_Object *pPathinfo;
	char *ug_pNaviTitle;

	Elm_Object_Item *ug_pPreNaviItem;
	Elm_Object_Item *ug_pNaviItem;
	Eina_List *ug_pNaviBarList;
};


typedef struct _ugStatus ugStatus;
struct _ugStatus {
	GString *ug_pPath;
	char *ug_launch_path; /*myfile launch view path*/
	int ug_launch_view;
	int ug_iState;
	int ug_iSortType;
	int ug_iRadioOn;
	int ug_iMmcFlag;
	int ug_iCtrlBarType;
	int ug_iRadioValue;	/** current the radio box selected item value **/
	int ug_iSelectedSortType;
	int ug_iMore;
	int ug_iCheckedCount;
	int ug_iTotalCount;
	int ug_iViewType;
	Eina_Bool flagSearchStart;

	bool ug_bInstallFlag;
	bool ug_bNoContentFlag;
	bool ug_bCancelDisableFlag;
	Eina_Bool ug_bSelectAllChecked;
	int  ug_iThemeType;
	char *ug_pUpper_folder;
	char *ug_pEntryPath; /*the current ringtone file path*/
	char *monitor_path;
	char *mark_mode;
	Eina_Bool ug_bDisableSelectAll;

	Elm_Genlist_Item_Class ug_1text3icon_itc;
	Elm_Genlist_Item_Class ug_1text2icon4_itc;
	Elm_Genlist_Item_Class ug_1text2icon_itc;
	Elm_Genlist_Item_Class ug_1text1icon_itc;
 	Evas_Object *ug_pRadioGroup;
	mf_search_handle search_handler;

	Eina_List *search_result_list;
	Ecore_Idler *search_idler;
	Ecore_Idler *popup_del_idler;
	Ecore_Idler *popup_create_idler;
	Ecore_Idler *msg_finish_idler;
	Ecore_Timer *pSearchTimer;
	Ecore_Timer *play_timer;

};

typedef enum __mf_ug_sound_mode_e mf_ug_sound_mode_e;
enum __mf_ug_sound_mode_e {
	mf_ug_sound_mode_none,
	mf_ug_sound_mode_ringtone,
	mf_ug_sound_mode_alert
};

typedef struct _ugUiGadget ugUiGadget;
struct _ugUiGadget {
	int ug_iFilterMode;
	int ug_iSelectMode;
	int ug_iMarkedMode;
	int ug_iImportMode;
	int ug_iSoundMode;
#ifdef UG_OPERATION_SELECT_MODE
	int ug_bOperationSelectFlag;
#endif
	unsigned long ug_iFileFilter;
	int ug_iMaxLength;
	Eina_Bool ug_MaxSetFlag;
	char *ug_pExtension;
	char *default_ringtone;
	char *title;
	char *domain;
	char *position;

	Eina_Bool silent;
	Eina_List *ug_pSearchFileList;
	Eina_List *ug_pDirList;
	Eina_List *ug_pFilterList;
	Eina_List *ug_pMultiSelectFileList;
	Ecore_Pipe *ug_pInotifyPipe;
	Ecore_Pipe *ug_pSyncPipe;
};

typedef struct _ugListPlay ugListPlay;
struct _ugListPlay {
	char *ug_pPlayFilePath;
	player_h ug_Player;
	int ug_iPlayState;
	Elm_Object_Item *play_data;
	bool hiden_flag;
	Ecore_Idler *playing_err_idler;
};


typedef struct _ugData ugData;
struct _ugData {
	ugMainWindow ug_MainWindow;
	ugStatus ug_Status;
	ugUiGadget ug_UiGadget;
	ugListPlay ug_ListPlay;
	ui_gadget_h ug;

	long long int limitsize;
	long long int selsize;

	Evas_Object *genlist;
	Ecore_Idler *show;
};

Evas_Object *mf_ug_main_tab_layout_create(Evas_Object *parent);
Evas_Object *mf_ug_main_create_bg(Evas_Object *win);
ugData * mf_ug_ugdata();
void mf_ug_main_update_ctrl_in_idle(void *data);
bool mf_ug_main_is_background();

#endif /* __DEF_MYFILE_H_ */
