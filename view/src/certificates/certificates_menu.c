/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of the Manage Applications
 * Written by Eunmi Son <eunmi.son@samsung.com>
 *
 * Author of this file:
 * Janusz Kozerski <j.kozerski@samsung.com>
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

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static void _quit_cb(void *data, Evas_Object* obj, void* event_info) {
	//---------------------------
	struct ug_data *ad = (struct ug_data*) data;

	/* bg must delete before starting on_destroy */
	evas_object_del(ad->bg);
	ad->bg = NULL;
	ug_destroy_me(ad->ug);
	//---------------------------
}

void certificates_menu_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data*)data;

	Evas_Object *list = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *back = NULL;

	layout = elm_layout_add(ad->win_main);
	if (!layout)
		return;

	list = elm_list_add(layout);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(list, "selected", list_clicked_cb, NULL);

	elm_list_item_append(list, dgettext(PACKAGE, "TRUSTED_ROOT_CERTIFICATE"), NULL, NULL, trusted_root_cert_cb, ad);
	elm_list_item_append(list, dgettext(PACKAGE, "USER_CERTIFICATE"), NULL, NULL, user_cert_cb, ad);
	elm_list_item_append(list, dgettext(PACKAGE, "PFX_CERTIFICATES"), NULL, NULL, pfx_cert_cb, ad);

	elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "CERTIFICATE_MENU_TITLE"), NULL, NULL, list, NULL);

	Elm_Object_Item *navi_it = NULL;
	navi_it = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "CERTIFICATE_MENU_TITLE"), NULL, NULL, list, NULL);

	back = elm_object_item_part_content_get(navi_it, "prev_btn");
	evas_object_smart_callback_add(back, "clicked", _quit_cb, ad);
}
