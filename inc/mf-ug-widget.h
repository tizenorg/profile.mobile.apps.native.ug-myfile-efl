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



#ifndef __MF_UG_WIDGET_H
#define __MF_UG_WIDGET_H

#include "mf-ug-main.h"

Evas_Object *mf_ug_widget_create_button(Evas_Object *parent, const char *style,
				  const char *caption, Evas_Object *icon,
				  void (*func) (void *, Evas_Object *, void *),
				  void *data,
				  Eina_Bool flag_propagate);
void mf_ug_widget_object_text_set(Evas_Object *obj, const char *ID, const char* part);
void mf_ug_widget_object_item_translate_set(Elm_Object_Item *item, const char *ID);

char *mf_ug_widget_get_text(const char *ID);
Evas_Object *mf_ug_widget_toolbar_create(Evas_Object *parent);
Elm_Object_Item *mf_ug_widget_item_tabbar_item_append(Evas_Object *obj,
		                        const char *icon,
		                        const char *label,
		                        Evas_Smart_Cb func,
		                        const void *data);
void mf_ug_widget_object_item_text_set(Elm_Object_Item *item, const char *ID, const char* part);
Evas_Object *mf_ug_widget_create_progressbar(Evas_Object *parent, char *style);
Evas_Object *mf_ug_widget_nocontent_create(Evas_Object *parent, const char *text, const char *icon_path);
Evas_Object *mf_ug_tabbar_create_path_tab(Evas_Object *parent, char *info);
void mf_ug_navi_bar_create_default_view(void *data);
void mf_ug_navi_bar_set_ctrl_button(void *data);
int mf_ug_genlist_ringtone_items_add(void *data, int value);
Elm_Object_Item *mf_ug_genlist_default_ringtone_item_append(Evas_Object *parent,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc);
Elm_Object_Item *mf_ug_genlist_silent_item_append(Evas_Object *parent,
				      void *data,
				      int groudValue,
				      Elm_Genlist_Item_Class *itc);

void mf_ug_object_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc,
	Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout);
#endif
