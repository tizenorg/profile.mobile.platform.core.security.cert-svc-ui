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

#include <cert-svc/cinstance.h>
#include <dlog.h>
#include <efl_assist.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

void direct_pfx_install_screen_cb(void *data, Evas_Object *obj, void *event_info) {

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

    install_button_cb(ad, obj, event_info);

    elm_naviframe_prev_btn_auto_pushed_set(ad->navi_bar, EINA_FALSE);
	ea_object_event_callback_add(ad->navi_bar, EA_CALLBACK_BACK, ea_naviframe_back_cb, NULL);
	ea_object_event_callback_add(ad->navi_bar, EA_CALLBACK_MORE, ea_naviframe_more_cb, NULL);

	LOGD("certificates_pfx_install_cb : EXIT");
}


void certificates_menu_cb(void *data, Evas_Object *obj, void *event_info) {

	struct ug_data *ad = (struct ug_data*) data;
    Evas_Object *list = NULL;

    if (certsvc_instance_new(&(ad->instance)) == CERTSVC_FAIL) {
        LOGD("certsvc_instance_new returned CERTSVC_FAIL");
        return;
    }
    ad->list_element = NULL;
    ad->type_of_screen = NONE_SCREEN;
    ad->refresh_screen_cb = NULL;
    ad->user_cert_list_item = NULL;

    list = elm_list_add(ad->navi_bar);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);

    evas_object_smart_callback_add(list, "selected", list_clicked_cb, NULL);

    Elm_Object_Item *nf_it = elm_list_item_append(list, "IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES", NULL, NULL, trusted_root_cert_cb, ad);
    elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);

    nf_it = elm_list_item_append(list, "IDS_ST_BODY_USER_CERTIFICATES", NULL, NULL, pfx_cert_cb, ad);
    elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);

    nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_CERTIFICATES", NULL, NULL, list, NULL);
	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);

    elm_naviframe_item_pop_cb_set(nf_it, quit_cb, data);

    elm_naviframe_prev_btn_auto_pushed_set(ad->navi_bar, EINA_FALSE);
	ea_object_event_callback_add(ad->navi_bar, EA_CALLBACK_BACK, ea_naviframe_back_cb, NULL);
	ea_object_event_callback_add(ad->navi_bar, EA_CALLBACK_MORE, ea_naviframe_more_cb, NULL);
}
