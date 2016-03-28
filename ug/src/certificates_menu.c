/**
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        certificates_menu.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include <efl_extension.h>

#include "common-utils.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

void direct_pfx_install_screen_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;

	ad->list_element = NULL;
	ad->type_of_screen = PKCS12_SCREEN;
	ad->refresh_screen_cb = NULL;
	ad->user_cert_list_item = NULL;

	pfx_cert_install(ad);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (intptr_t)data;

	if (strcmp(part, "elm.text"))
		return NULL;

	if (index == 0)
		return strdup(dgettext(PACKAGE, "IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES_ABB"));
	else if (index == 1)
		return strdup(dgettext(PACKAGE, "IDS_ST_BODY_USER_CERTIFICATES"));
	else
		return NULL;
}

static Evas_Object *_create_genlist(struct ug_data *ad, Evas_Object *parent)
{
	LOGD("Start to create genlist in Manage Cerificates.");
	Evas_Object *genlist = elm_genlist_add(parent);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	if (itc == NULL)
		return genlist;

	itc->item_style = "default";
	itc->func.text_get = _gl_text_get;

	evas_object_smart_callback_add(genlist, "selected", genlist_clicked_cb, NULL);

	// Add "Trusted root CA certificates"
	elm_genlist_item_append(
			genlist,
			itc,
			(void *)0,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			trusted_root_cert_cb,
			ad);

	// Add "User certificates"
	elm_genlist_item_append(
			genlist,
			itc,
			(void *)1,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			pfx_cert_cb,
			ad);

	return genlist;
}

void create_certificates_menu(struct ug_data *ad)
{
	LOGD("Start to create certificates menu.");
	Evas_Object *genlist = _create_genlist(ad, ad->navi_bar);
	evas_object_show(genlist);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(
			ad->navi_bar,
			"IDS_ST_HEADER_MANAGE_CERTIFICATES_ABB",
			common_back_btn(ad),
			NULL,
			genlist,
			NULL);

	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, quit_cb, ad);
}
