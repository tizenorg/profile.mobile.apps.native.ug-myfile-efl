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





#ifndef __DEF_MF_UG_INOTIFY_HANDLE_H
#define __DEF_MF_UG_INOTIFY_HANDLE_H

typedef enum _mf_ug_inotify_event mf_ug_inotify_event;
enum _mf_ug_inotify_event {
	UG_MF_INOTI_NONE = 0,
	UG_MF_INOTI_CREATE,
	UG_MF_INOTI_DELETE,
	UG_MF_INOTI_MODIFY,
	UG_MF_INOTI_MOVE_OUT,
	UG_MF_INOTI_MOVE_IN,
	UG_MF_INOTI_DELETE_SELF,
	UG_MF_INOTI_MOVE_SELF,
	UG_MF_INOTI_MAX,
};

typedef void (*mf_ug_inotify_cb) (mf_ug_inotify_event event, char *name, void *data);

int mf_ug_inotify_handle_init_inotify(void);
int mf_ug_inotify_handle_add_inotify_watch(const char *path, mf_ug_inotify_cb callback, void *user_data);
int mf_ug_inotify_handle_rm_inotify_watch(void);
void mf_ug_inotify_handle_finalize_inotify(void);

#endif
