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

#include <Elementary.h>
#include "mf-ug-dlog.h"
#include "mf-ug-conf.h"
#include "mf-ug-main.h"
#include "mf-ug-fs-util.h"
#include "mf-ug-util.h"
#include "mf-ug-resource.h"
#include "mf-ug-fm-svc-wrapper.h"
#include "mf-ug-winset.h"
#include "mf-ug-file-util.h"

char *mf_ug_widget_get_text(const char *ID)
{
	ug_mf_retvm_if(ID == NULL, NULL, "ID is NULL");

	char *str;

	str = dgettext(UGPACKAGE, ID);

	/** Fix P140626-02426
	 * if got text from sys failed, try to
	 * get it from UGPACKAGE.
	 */
	if (strncmp(ID, str, strlen(ID)) == 0) {
		str = dgettext(UGPACKAGE, ID);
	} else {
		goto END;
	}

	/** Fix P140710-04855
	 * if got text from package failed, try to
	 * get it from myfile.
	 */
	if (strncmp(ID, str, strlen(ID)) == 0) {
		str = dgettext("myfile", ID);
	} else {
		goto END;
	}

	/** at worst case, try gettext() */
	if (strncmp(ID, str, strlen(ID)) == 0) {
		str = gettext(ID);
	}
END:
	return str;
}

void mf_ug_widget_object_text_set(Evas_Object *obj, const char *ID, const char* part)
{
	ug_mf_retm_if(ID == NULL, "ID is NULL");
	ug_mf_retm_if(obj == NULL, "obj is NULL");

	const char *domain;

	domain = UGPACKAGE;    /*PKGNAME_MYFILE;*/

	elm_object_domain_translatable_part_text_set(obj, part, domain, ID);
}


void mf_ug_widget_object_item_text_set(Elm_Object_Item *item, const char *ID, const char* part)
{
	ug_mf_retm_if(ID == NULL, "ID is NULL");
	ug_mf_retm_if(item == NULL, "item is NULL");
	const char *domain;

	domain = UGPACKAGE;    /*PKGNAME_MYFILE;*/

	elm_object_item_domain_translatable_part_text_set(item, part, domain, ID);
}

void mf_ug_widget_object_item_translate_set(Elm_Object_Item *item, const char *ID)
{
	ug_mf_retm_if(ID == NULL, "ID is NULL");
	ug_mf_retm_if(item == NULL, "item is NULL");
	const char *domain;

	domain = UGPACKAGE;

	SECURE_DEBUG(">>>>>>>>>>>>>>> ID is [%s] domain is [%s]", ID, domain);
	//elm_object_item_domain_text_translatable_set(item, domain, EINA_TRUE);
	elm_object_domain_translatable_text_set(item, domain, ID);
}

Evas_Object *mf_ug_widget_create_progressbar(Evas_Object *parent, char *style)
{
	UG_TRACE_BEGIN;
	if (!parent) {
		return NULL;
	}

	Evas_Object *progressbar = elm_progressbar_add(parent);
	if (style) {
		elm_object_style_set(progressbar, style);
	}
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);
	return progressbar;
}

Evas_Object *mf_ug_widget_create_button(Evas_Object *parent, const char *style,
                                        const char *caption, Evas_Object *icon,
                                        void (*func)(void *, Evas_Object *, void *),
                                        void *data,
                                        Eina_Bool flag_propagate)
{
	UG_TRACE_BEGIN;
	if (!parent) {
		return NULL;
	}

	Evas_Object *btn;

	btn = elm_button_add(parent);

	if (style) {
		elm_object_style_set(btn, style);
	}
	if (caption) {
		mf_ug_widget_object_text_set(btn, caption, NULL);
		elm_access_info_set(btn, ELM_ACCESS_INFO, mf_ug_widget_get_text(caption));
	}

	if (icon) {
		elm_object_content_set(btn, icon);
	}

	evas_object_propagate_events_set(btn, flag_propagate);

	evas_object_smart_callback_add(btn, "clicked", func, (void *)data);
	evas_object_show(btn);
	UG_TRACE_END;

	return btn;
}

Evas_Object *mf_ug_widget_toolbar_create(Evas_Object *parent)
{
	Evas_Object *toolbar = NULL;
	toolbar = elm_toolbar_add(parent);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	return toolbar;
}

Elm_Object_Item *mf_ug_widget_item_tabbar_item_append(Evas_Object *obj,
        const char *icon,
        const char *label,
        Evas_Smart_Cb func,
        const void *data)
{
	Elm_Object_Item *item = elm_toolbar_item_append(obj, icon, label, func, data);

	mf_ug_widget_object_item_translate_set(item, label);
	return item;
}

Evas_Object *mf_ug_widget_nocontent_create(Evas_Object *parent, const char *text, const char *icon_path)
{
	Evas_Object *nocontent = elm_layout_add(parent);
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);

	Evas_Object *icon = elm_image_add(nocontent);
	elm_image_file_set(icon, UG_EDJ_IMAGE, icon_path);
	elm_object_part_content_set(nocontent, "nocontents.image", icon);
	mf_ug_widget_object_text_set(nocontent, text, "elm.text");
	return nocontent;
}

Evas_Object *mf_ug_tabbar_path_widget_create(Evas_Object *parent)
{
	Evas_Object *obj;
	obj = elm_toolbar_add(parent);
	elm_toolbar_homogeneous_set(obj, EINA_FALSE);
	elm_toolbar_align_set(obj, 0.0);
	if (obj == NULL) {
		return NULL;
	}
	elm_object_style_set(obj, "navigationbar");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_SCROLL);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	return obj;
}

Elm_Object_Item *mf_ug_tabbar_item_append(Evas_Object *obj,
        const char *icon,
        const char *label,
        Evas_Smart_Cb func,
        const void *data)
{
	Elm_Object_Item *item = elm_toolbar_item_append(obj, icon, label, func, data);

	mf_ug_widget_object_item_translate_set(item, label);
	return item;
}

void mf_ug_tabbar_path_item_cb(void *data, Evas_Object * obj, void *event_info)
{
	ugData *ugd = mf_ug_ugdata();
	char *fullpath = (char *)data;
	ug_error("~~~~~~~~~~~~~~~~~~ fullpath is [%s], current path is [%s]", fullpath, ugd->ug_Status.ug_pPath->str);
	if (fullpath == NULL || !mf_file_exists(fullpath)) {
		UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
		ugd->ug_Status.ug_pPath = g_string_new(PHONE_FOLDER);
		ugd->ug_Status.ug_iViewType = mf_ug_view_root;
		mf_ug_navi_bar_create_default_view(ugd);
	} else {
		if (g_strcmp0(ugd->ug_Status.ug_pPath->str, fullpath) == 0) {
			ug_error("The same folder selected");
			return;
		} else {
			UG_SAFE_FREE_GSTRING(ugd->ug_Status.ug_pPath);
			ugd->ug_Status.ug_pPath = g_string_new(fullpath);
			mf_ug_navi_bar_create_default_view(ugd);
		}
	}
}

Evas_Object *mf_ug_tabbar_create_path_tab(Evas_Object *parent, char *info)
{
	ug_error();
	UG_TRACE_BEGIN;
	ug_mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	Evas_Object *tab = NULL;
	int location = mf_ug_fm_svc_wapper_get_location(info);
	ug_error();
	Eina_List *path_list = (Eina_List *)mf_ug_fm_svc_wrapper_level_path_get(info);
	ug_error();
	if (path_list) {
		ug_error();
		tab = mf_ug_tabbar_path_widget_create(parent);
		ug_error();
		Eina_List *l = NULL;
		char *path = NULL;
		const char *label = NULL;
		int count = 1;
		EINA_LIST_FOREACH(path_list, l, path) {
			if (path) {
				ug_error("path is [%s]", path);
				if (count == 1) {
					switch (location) {
					case MF_UG_PHONE:
						label = MF_UG_LABEL_PHONE;
						break;
					case MF_UG_MMC:
						label = MF_UG_LABEL_MMC;
						break;
					default:
						return NULL;
					}
					mf_ug_tabbar_item_append(tab, NULL, mf_ug_widget_get_text(label), mf_ug_tabbar_path_item_cb, g_strdup(path));
					count++;
				} else {
					label = mf_file_get(path);
					mf_ug_tabbar_item_append(tab, NULL, mf_ug_widget_get_text(label), mf_ug_tabbar_path_item_cb, g_strdup(path));
					count++;
				}
			}
		}
		ug_error();
		Elm_Object_Item *last_item = elm_toolbar_last_item_get(tab);
		elm_object_item_disabled_set(last_item, EINA_FALSE);
		elm_toolbar_item_show(last_item, ELM_TOOLBAR_ITEM_SCROLLTO_LAST);
		ug_error();
	}
	ug_error();
	mf_ug_util_free_eina_list_data(&path_list, NODE_TYPE_CHAR);
	ug_error();
	return tab;
}


void mf_ug_object_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc,
        Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout)
{
	Evas_Object *pSelectAllLayout = elm_layout_add(pParent);
	elm_layout_file_set(pSelectAllLayout, UG_EDJ_NAVIGATIONBAR, "select.all.layout");
	evas_object_size_hint_weight_set(pSelectAllLayout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(pSelectAllLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(pSelectAllLayout, EVAS_CALLBACK_MOUSE_DOWN, pMouseDownFunc, pUserData);
	*pSelectLayout = pSelectAllLayout;
	elm_object_part_text_set(pSelectAllLayout, "elm.text", mf_ug_widget_get_text(MF_UG_LABEL_SELECT_ALL));
	Evas_Object *pSelectAllCheckbox = elm_check_add(pSelectAllLayout);
	evas_object_smart_callback_add(pSelectAllCheckbox, "changed", pChangeFunc, pUserData);
	evas_object_propagate_events_set(pSelectAllCheckbox, EINA_FALSE);
	elm_object_part_content_set(pSelectAllLayout, "elm.icon", pSelectAllCheckbox);
	evas_object_show(pSelectAllLayout);
	*pCheckBox = pSelectAllCheckbox;
}
