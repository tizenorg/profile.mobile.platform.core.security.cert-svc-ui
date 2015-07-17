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

#include <dlog.h>
#include <cert-svc/cinstance.h>
#include <efl_extension.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static Evas_Object *genlist = NULL;

void _ug_install_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;

	pfx_cert_install_cb(ad, NULL, NULL);
}

void direct_pfx_install_screen_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("certificates_pfx_install_cb : IN");
	struct ug_data *ad = (struct ug_data*) data;

	if (certsvc_instance_new(&(ad->instance)) == CERTSVC_FAIL) {
		LOGD("certsvc_instance_new returned CERTSVC_FAIL");
		return;
	}
	ad->list_element = NULL;
	ad->type_of_screen = PKCS12_SCREEN;
	ad->refresh_screen_cb = NULL;
	ad->user_cert_list_item = NULL;

	_ug_install_button_cb(ad, obj, event_info);

	elm_naviframe_prev_btn_auto_pushed_set(ad->navi_bar, EINA_FALSE);
	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	LOGD("certificates_pfx_install_cb : EXIT");
}


static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

	int index = (int) data;
	if (!strcmp(part, "elm.text.main.left"))  {
		if (index == 0 ){
			return strdup(dgettext(PACKAGE,"IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES_ABB"));
		}
		else if(index == 1) {
			return strdup(dgettext(PACKAGE,"IDS_ST_BODY_USER_CERTIFICATES"));
		}
	}
	return  NULL;
}

static Evas_Object * _create_genlist(struct ug_data *ad, Evas_Object *parent)
{
	genlist = elm_genlist_add(parent);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    if(itc == NULL) goto error;
	itc->item_style = "1line";
	itc->func.text_get = _gl_text_get;

	Elm_Genlist_Item_Class *itc1 = elm_genlist_item_class_new();
    if(itc1 == NULL) goto error;
	itc1->item_style = "1line";
	itc1->func.text_get = _gl_text_get;
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(genlist, "selected", genlist_clicked_cb, NULL);

	elm_genlist_item_append(
			genlist,			// genlist object
			itc,				// item class
			(void *) 0,		// data
			NULL,
			ELM_GENLIST_ITEM_NONE,
			trusted_root_cert_cb,
			ad
	);

	elm_genlist_item_append(
			genlist,			// genlist object
			itc1,				// item class
			(void *) 1,		// data
			NULL,
			ELM_GENLIST_ITEM_NONE,
			pfx_cert_cb,
			ad
	);

error:
    if(itc != NULL)
	    elm_genlist_item_class_free(itc);
	return genlist;
}

void certificates_menu_cb(void *data, Evas_Object *obj, void *event_info) {

	LOGD("certificates_menu_cb");
	struct ug_data *ad = (struct ug_data*) data;
	Evas_Object *box = NULL;

	if (certsvc_instance_new(&(ad->instance)) == CERTSVC_FAIL) {
		LOGD("certsvc_instance_new returned CERTSVC_FAIL");
		return;
	}

	box = elm_box_add(ad->navi_bar);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	genlist = _create_genlist(ad, box);
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	evas_object_show(box);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_HEADER_MANAGE_CERTIFICATES_ABB", NULL, NULL, box, NULL);
	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, quit_cb, data);

	elm_naviframe_prev_btn_auto_pushed_set(ad->navi_bar, EINA_FALSE);
	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
}
