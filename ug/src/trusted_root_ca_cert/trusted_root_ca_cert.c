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
 * @file        trusted_root_ca_cert.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include "common-utils.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static Eina_Bool trusted_root_cert_create_genlist(struct ug_data *ad, Evas_Object *parent)
{
	if (!ad)
		return EINA_TRUE;

	ad->list_to_refresh = elm_genlist_add(parent);
	evas_object_size_hint_weight_set(ad->list_to_refresh, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->list_to_refresh, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Eina_Bool no_content_bool = EINA_TRUE;

	if (!make_list(ad, ad->list_to_refresh, NULL, ad->list_element))
		no_content_bool = EINA_FALSE;

	return no_content_bool;
}

void trusted_root_cert_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("Start layout - Trusted root CA certificates.");
	Eina_Bool no_content_bool;
	Elm_Object_Item *nf_it;
	Evas_Object *box = NULL;
	struct ListElement *firstListElement;
	struct ug_data *ad = (struct ug_data *)data;

	if (ad == NULL)
		return;

	firstListElement = initList();

	if (!firstListElement) {
		LOGE("Fail to initList for firstListElement");
		return;
	}

	ad->list_element = firstListElement;

	box = elm_box_add(ad->navi_bar);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	no_content_bool = trusted_root_cert_create_genlist(ad, box);
	evas_object_show(ad->list_to_refresh);
	elm_box_pack_end(box, ad->list_to_refresh);
	evas_object_show(box);

	if (!no_content_bool) {
		nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES_ABB", common_back_btn(ad), NULL, box, NULL);
	} else {
		Evas_Object *no_content = create_no_content_layout(ad);

		if (!no_content) {
			LOGD("Cannot create no_content layout (NULL); return");
			return;
		}
		nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES_ABB", common_back_btn(ad), NULL, no_content, NULL);
	}
	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, back_cb, (struct Evas_Object *)ad);
}
