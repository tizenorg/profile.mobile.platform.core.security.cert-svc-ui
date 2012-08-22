/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved 
 * 
 * This file is part of the Manage Applications
 * Written by Eunmi Son <eunmi.son@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of 
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not 
 * disclose such Confidential Information and shall use it only in 
 * accordance with the terms of the license agreement you entered 
 * into with SAMSUNG ELECTRONICS. 
 *
 * SAMSUNG make no representations or warranties about the suitability 
 * of the software, either express or implied, including but not limited 
 * to the implied warranties of merchantability, fitness for a particular 
 * purpose, or non-infringement. SAMSUNG shall not be liable for any 
 * damages suffered by licensee as a result of using, modifying or 
 * distributing this software or its derivatives.
 *
 */


#ifndef __MGR_APP_VIEW_MANAGER_H__
#define __MGR_APP_VIEW_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <Evas.h>
#include <Elementary.h>
#include <glib.h>
#include <glib-object.h>
#include "mgr-app-common-error.h"

typedef struct _mgr_app_view_common_data_t {
	Evas_Object 	*common_win_main;
	Evas_Object		*common_bg;
	Evas_Object 	*common_layout_main;
	Evas_Object 	*common_navibar;

	Evas_Object 	*view_cbar;
	Evas_Object 	*view_layout;

	char			*title;
	char			*btn1_label;
	Evas_Smart_Cb 	btn1_func;

	void 			*user_view_data;
} mgr_app_view_common_data_t;

typedef struct _mgr_app_view_t {
	Eina_Bool 			is_created;

	Evas_Object		*(*create)(void *data);
	Evas_Object		*(*setcbar)(void *data);
	int 			(*setnavibar)(void *data);
	int 			(*destroy)(void *data);
	int 			(*update)(void *data);

	mgr_app_view_common_data_t *view_common_data;
} mgr_app_view_t;

void mgr_app_view_set_win_main(Evas_Object *winmain);
void mgr_app_view_set_bg(Evas_Object *bg);
void mgr_app_view_set_layout_main(Evas_Object *layoutmain);
void mgr_app_view_set_navibar(Evas_Object *navibar);
void mgr_app_view_set_popup(Evas_Object *popup);
void mgr_app_view_set_viewlist(GList *viewlist);

Evas_Object *mgr_app_view_get_win_main(void);
Evas_Object *mgr_app_view_get_bg(void);
Evas_Object *mgr_app_view_get_layout_main(void);
Evas_Object *mgr_app_view_get_navibar(void);
Evas_Object *mgr_app_view_get_popup(void);
GList *mgr_app_view_get_viewlist(void *data);

mgr_app_result_e mgr_app_view_create(mgr_app_view_t *view, void *data);
mgr_app_result_e mgr_app_view_destroy(mgr_app_view_t *view, void *data);
mgr_app_result_e mgr_app_view_update(mgr_app_view_t *view, void *data);
mgr_app_result_e mgr_app_view_cleanup(mgr_app_view_t *view, void *data);
mgr_app_result_e mgr_app_view_manager_navibar_title_update(const char *title, mgr_app_view_t *view, void *data);
mgr_app_result_e mgr_app_view_manager_set_navibar(mgr_app_view_common_data_t *view_data, char *title, char *btn1_label, Evas_Smart_Cb btn1_func, const char *btn2_label, Evas_Smart_Cb btn2_func);
void mgr_app_view_common_back_cb(void *data, Eina_Bool is_update);
void mgr_app_view_go_special_view(mgr_app_view_t *target_view, void *data);


#ifdef __cplusplus
}
#endif

#endif /*__MGR_APP_VIEW_MANAGER_H__*/

