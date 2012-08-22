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

#ifndef __MGR_APP_WIDGET_H__
#define __MGR_APP_WIDGET_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <Evas.h>
#include <Elementary.h>

Evas_Object *mgr_app_view_create_base_navigation(Evas_Object *parent);
Evas_Object *mgr_app_widget_create_bg(Evas_Object *parent);
Evas_Object *mgr_app_widget_create_main_layout(Evas_Object *parent);
Evas_Object *mgr_app_create_layout(Evas_Object *parent, const char *clas, const char *group, const char *style);
Evas_Object *mgr_app_create_layout_file(Evas_Object *parent, const char *filename, const char *group);
Evas_Object *mgr_app_create_scroller(Evas_Object *parent);
Evas_Object *mgr_app_create_box(Evas_Object *parent, Eina_Bool is_hori);
Evas_Object *mgr_app_create_button(Evas_Object *parent, void (*func) (void *data, Evas_Object *obj, void *event_info), const char *label, const char *swallow, const char *style, void *data);
Evas_Object *mgr_app_create_label(Evas_Object *parent, const char *text, const char *swallow, void *data);
void mgr_app_box_pack_end_separator(Evas_Object *box, const char *style);

void mgr_app_set_transit_effect(Evas_Object *parent, int effect_type);

#ifdef __cplusplus
}
#endif


#endif /*__MGR_APP_WIDGET_H__*/
