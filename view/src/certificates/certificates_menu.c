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

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

Eina_Bool _quit_cb(void *data, Elm_Object_Item *it)
{
    struct ug_data *ad = (struct ug_data*) data;

	if (ad->ug) {
		ug_destroy_me(ad->ug);
		ad->ug = NULL;
	}

	return EINA_TRUE;   
}
	  

void certificates_menu_cb(void *data, Evas_Object *obj, void *event_info) {
    struct ug_data *ad = (struct ug_data*) data;

    Evas_Object *list = NULL;
    Evas_Object *back = NULL;

    if (certsvc_instance_new(&(ad->instance)) == CERTSVC_FAIL) {
        LOGD("certsvc_instance_new returned CERTSVC_FAIL");
        return;
    }
    ad->list_element = NULL;
    ad->type_of_screen = NONE_SCREEN;

    list = elm_list_add(ad->navi_bar);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);

    evas_object_smart_callback_add(list, "selected", list_clicked_cb, NULL);

    elm_list_item_append(list, dgettext(PACKAGE, "IDS_ST_BODY_TRUSTED_ROOT_CERTIFICATES"), NULL, NULL, trusted_root_cert_cb, ad);
    elm_list_item_append(list, dgettext(PACKAGE, "IDS_ST_BODY_USER_CERTIFICATES"), NULL, NULL, pfx_cert_cb, ad);

    elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "IDS_ST_BODY_CERTIFICATES"), NULL, NULL, list, NULL);
	
	back = elm_button_add(ad->navi_bar);

	elm_object_style_set(back, "naviframe/back_btn/default");
	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "IDS_ST_BODY_CERTIFICATES"), back, NULL, list, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _quit_cb, data); 
}
