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

#include <dirent.h>

//static char *dir_path = PATH_CERT_ROOT_PATH; // This path is the original certificates path
static char *dir_path = PATH_CERT_WAC; // This can be used for tests

void trusted_root_cert_cb(void *data, Evas_Object *obj, void *event_info) {

	LOGD("trusted root ---------------------");

	struct ug_data *ad = (struct ug_data *) data;
	Evas_Object *list = NULL;

	struct ListElement *firstListElement = NULL;
	struct ListElement *lastListElement = NULL;
	firstListElement = initList();
	lastListElement = firstListElement;

	Evas_Object *toolbar = elm_toolbar_add(ad->navi_bar);
	if (!toolbar)
		return;
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);

	elm_toolbar_item_append(toolbar, NULL, dgettext(PACKAGE, "UNINSTALL"), delete_cert_cb, strdup(dir_path));

	list = elm_list_add(ad->win_main);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", NULL, NULL);

	if(make_list(ad, list, dir_path, lastListElement, TRUSTED_ROOT)){
		Evas_Object *no_content = elm_layout_add(ad->win_main);
		elm_layout_theme_set(no_content, "layout", "nocontents", "text");
		elm_object_part_text_set(no_content, "elm.text", dgettext(PACKAGE, "IDS_COM_BODY_NO_CONTENTS"));
		evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(no_content);

		elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "TRUSTED_ROOT_CERTIFICATE"), NULL, NULL, no_content, NULL);
	} else {

		Elm_Object_Item *itm = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "TRUSTED_ROOT_CERTIFICATE"), NULL, NULL, list,
				NULL);
		elm_object_item_part_content_set(itm, "controlbar", toolbar);

		Evas_Object *back = NULL;
		back = elm_object_item_part_content_get(itm, "prev_btn");
		evas_object_smart_callback_add(back, "clicked", back_cb, ad);
	}

}

