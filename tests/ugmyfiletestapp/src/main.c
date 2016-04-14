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

#include <app.h>
#include <Elementary.h>
#include <assert.h>
//#include <ui-gadget.h>

#ifdef UNIT_TESTS
#include "run-tests.h"
#endif

#define EDJ_PATH "/opt/apps/org.tizen.ugmyfiletestapp/res/edje"

#define EDJ_FILE EDJ_PATH"/layoutedj.edj"

struct _appdata {
	const char *name;
	Evas_Object *win;
	ui_gadget_h ug;
	Evas_Object *layout;
	Ecore_Event_Handler *key_event_handler;
};

static void ug_layout_callback(ui_gadget_h ug, enum ug_mode mode, void *priv)
{

	printf("%s\n", __FUNCTION__);

	struct _appdata *ad = priv;
	Evas_Object *base = (Evas_Object *)ug_get_layout(ug);
	Evas_Object *win = ug_get_window();
	if (!base) {
		printf("!base -> Call ug_destroy\n");
		ug_destroy(ug);
		return;
	}

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(win, base);
		evas_object_show(base);
		break;
	case UG_MODE_FRAMEVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(ad->layout, "custom", base);
		break;
	default:
		break;
	}
}

void ug_destroy_callback(ui_gadget_h ug, void *priv)
{
	struct _appdata *ad = priv;
	ug_destroy(ad->ug);
	ad->ug = NULL;
}


static void on_click(void *data, Evas_Object *obj, void *event_info)
{

	struct _appdata *ad = data;
	struct ug_cbs cbs;

	memset(&cbs, 0, sizeof(cbs));
	cbs.layout_cb = ug_layout_callback;
	cbs.destroy_cb = ug_destroy_callback;
	cbs.priv = ad;
	app_control_h app_control;
	int ret = app_control_create(&app_control);

	if (ret != SERVICE_ERROR_NONE) {
		printf("app_control create failed\n");
	}

	app_control_add_extra_data(app_control, "path", "/");
	app_control_add_extra_data(app_control, "select_type",  elm_object_text_get(obj));

	printf("-----%s\n", __FUNCTION__);
	ad->ug = ug_create(NULL, "myfile-efl", UG_MODE_FULLVIEW, app_control, &cbs);
	printf("-----ug addr is %p\n", ad->ug);

}



static Evas_Object *_add_win(const char *name)
{
	Evas_Object *win;

	win = elm_win_util_standard_add(name, "ugmyfiletestapp");
	if (!win) {
		return NULL;
	}

	evas_object_show(win);

	return win;
}

static void _add_button(const char *name, Evas_Object *box, struct _appdata *ad)
{
	Evas_Object *btn;
	btn = elm_button_add(box);
	elm_object_text_set(btn, name);
	evas_object_smart_callback_add(btn, "clicked", on_click, ad);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(box, btn);
	evas_object_show(btn);
}


static Eina_Bool callback_hw_key_down(void *data, int type __attribute__((unused)), void *event)
{
	assert(data != NULL);
	assert(event != NULL);

	if (data == NULL) {
		return ECORE_CALLBACK_DONE;
	}
	if (event == NULL) {
		return ECORE_CALLBACK_DONE;
	}

	struct _appdata *ad = data;
	Ecore_Event_Key *key_event = event;

	if (strcmp(key_event->keyname, "XF86Back") == 0) {
		if (ad->ug) {
			return ECORE_CALLBACK_PASS_ON;
		} else {
			elm_exit();
		}
	}

	return ECORE_CALLBACK_DONE;
}

static bool _create(void *user_data)
{
	char *btn_name[] = {    "MULTI_ALL",
	                        "SINGLE_ALL",
	                        "MULTI_FILE",
	                        "SINGLE_FILE",
	                        "IMPORT",
	                        "EXPORT",
	                        "SHORTCUT",
	                        "SAVE"
	                   };

	struct _appdata *ad;
	Evas_Object *win;
	Evas_Object *bx;
	Evas_Object *datetime;
	Evas_Object *layout;

	if (!user_data) {
		return false;
	}

	ad = user_data;

	win = _add_win(ad->name);
	if (!win) {
		return false;
	}
	layout = elm_layout_add(win);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, layout);
	elm_layout_file_set(layout, EDJ_FILE, "mylayout");
	evas_object_show(layout);

	const char *title = elm_layout_data_get(layout, "title");
	if (title) {
		elm_win_title_set(win, title);
		elm_object_part_text_set(layout, "title", title);
	}

	bx = elm_box_add(win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/*elm_win_resize_object_add(win, bx);*/
	elm_box_horizontal_set(bx, EINA_FALSE);
	elm_object_part_content_set(layout, "custom", bx);
	evas_object_show(bx);

	_add_button("full view", bx, ad);
	_add_button("frame view", bx, ad);

	int i = 0;
	int len = sizeof(btn_name) / sizeof(btn_name[0]);
	for (; i < len; ++i) {
		/* code */
		_add_button(btn_name[i], bx, ad);
	}

	elm_object_part_content_set(layout, "custom", bx);

	ad->key_event_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, callback_hw_key_down, ad);

	UG_INIT_EFL(win, UG_OPT_INDICATOR_ENABLE);
	ad->win = win;
	ad->layout = layout;
	return true;
}

static void _terminate(void *user_data)
{
	struct _appdata *ad;

	if (!user_data) {
		return;
	}

	ad = user_data;

	if (ad->win) {
		evas_object_del(ad->win);
	}
	if (ad->key_event_handler) {
		ecore_event_handler_del(ad->key_event_handler);
	}
}

static void _pause(void *user_data)
{
	if (!user_data) {
		return;
	}
}

static void _resume(void *user_data)
{
}

static void _app_control(app_control_h app_control, void *user_data)
{
}

static void _low_memory(void *user_data)
{
}

static void _low_battery(void *user_data)
{
}

static void _dev_orientation_changed(app_device_orientation_e orientation,
                                     void *user_data)
{
}

static void _lang_changed(void *user_data)
{
}

static void _region_fmt_changed(void *user_data)
{
}

void _init_and_run(int argc, char **argv)
{
	struct _appdata ad;
	app_event_callback_s cbs = {
		.create = _create,
		.terminate = _terminate,
		.pause = _pause,
		.resume = _resume,
		.app_control = _app_control,
		.low_memory = _low_memory,
		.low_battery = _low_battery,
		.device_orientation = _dev_orientation_changed,
		.language_changed = _lang_changed,
		.region_format_changed = _region_fmt_changed,
	};

	memset(&ad, 0x00, sizeof(ad));
	ad.name = "testug";

	app_efl_main(&argc, &argv, &cbs, &ad);
}

int main(int argc, char **argv)
{
#ifdef UNIT_TESTS
	return run_tests(argc, argv);
#endif /* UNIT_TESTS */

	_init_and_run(argc, argv);
	return 0;
}

