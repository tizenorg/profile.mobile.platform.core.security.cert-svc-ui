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
#define _GNU_SOURCE

#include <stdlib.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static void copy(char *source, char *dest);

static char *sd_dir_path = PATH_SDCARD;
//static char *sd_dir_path = PATH_CERT_WAC; // This path can be use for testing instead PATH_SDCARD
static char *user_dir_path = PATH_CERT_USER;

static struct ug_data *ad;
static struct ListElement *firstListElement;
static struct ListElement *lastListElement;

void user_search_cert_cb(void *data, Evas_Object *obj, void *event_info) {

	ad = (struct ug_data *) data;
	Evas_Object *list = NULL;

	firstListElement = initList();
	lastListElement = firstListElement;

	list = elm_list_add(ad->win_main);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", list_clicked_cb, NULL);

	if(make_list(ad, list, sd_dir_path, lastListElement, TO_INSTALL)){
		Evas_Object *no_content = elm_layout_add(ad->win_main);
		elm_layout_theme_set(no_content, "layout", "nocontents", "text");
		elm_object_part_text_set(no_content, "elm.text", dgettext(PACKAGE, "IDS_COM_BODY_NO_CONTENTS"));
		evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(no_content);

		elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "SEARCHED_CERTIFICATE"), NULL, NULL, no_content, NULL);
	} else {
		Elm_Object_Item *itm = NULL;
		itm = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "SEARCHED_CERTIFICATE"), NULL, NULL, list, NULL);

		Evas_Object *back = NULL;
		back = elm_object_item_part_content_get(itm, "prev_btn");
		evas_object_smart_callback_add(back, "clicked", back_cb, ad);
	}

}


void install_cb(void *data, Evas_Object *obj, void *event_info) {
	LOGD("install_cb");
	struct ListElement *listElement = (struct ListElement *) data;
	char *file_name = listElement->name;
	LOGD("%s", file_name);
	char buf_src[256];
	char buf_dst[256];
	sprintf(buf_src, "%s/%s", sd_dir_path, file_name);
	sprintf(buf_dst, "%s/%s", user_dir_path, file_name);
	LOGD("Start copying");
	copy(buf_src, buf_dst);
	LOGD("End copying");

	deleteList(firstListElement);

	refresh_list(ad);
	elm_naviframe_item_pop(ad->navi_bar);
}

static void copy(char *source, char *dest) {
	LOGD("copy()");
	if (!source || !dest) {
		LOGD("Null pointer to files");
		return;
	}
	char *command;
	int result;
	result = asprintf(&command, "cp %s %s", source, dest);
	if(command == NULL && result != -1){
		LOGD("Error while allocating memory");
		return; //Error while allocating memory
	}

	LOGD("%s", command);
	result = system(command);
	LOGD("%s --- return %d", command, result);

	free(command);
	LOGD("copy() - done");
}
