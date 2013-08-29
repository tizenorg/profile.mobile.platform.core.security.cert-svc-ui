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

#include <dlog.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#include <dirent.h>

Eina_Bool trusted_root_cert_create_list(struct ug_data *ad){
    LOGD("trusted_root_cert_create_list()");

    if(NULL == ad) {
        return EINA_TRUE;
    }
    elm_list_clear(ad->list_to_refresh);

    Eina_Bool no_content_bool = EINA_TRUE;
    if(!make_list(ad, ad->list_to_refresh, PATH_CERT_TRUSTEDUSER, ad->list_element)){
        no_content_bool = EINA_FALSE;
    }
    ad->list_element = findLastElement(ad->list_element);
    if(!make_list(ad, ad->list_to_refresh, PATH_CERT_SSL_ETC, ad->list_element)){
        no_content_bool = EINA_FALSE;
    }

    return no_content_bool;
}

void trusted_root_cert_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("trusted_root_cert_cb()");

    if(NULL == data) {
        return;
    }
    struct ug_data *ad = (struct ug_data *) data;
    struct ListElement *firstListElement = NULL;
    struct ListElement *lastListElement  = NULL;
    firstListElement = initList();
    lastListElement  = firstListElement;
    ad->list_element = firstListElement;
    ad->list_to_refresh = NULL;
    ad->list_to_refresh = elm_list_add(ad->win_main);
    elm_list_mode_set(ad->list_to_refresh, ELM_LIST_COMPRESS);

    Elm_Object_Item *nf_it;
    if (!trusted_root_cert_create_list(ad)) { // There is some content

		nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES", NULL, NULL, ad->list_to_refresh, NULL);

    } else {
        // No content
        Evas_Object *no_content = create_no_content_layout(ad);

        if(!no_content){
            LOGD("Cannot create no_content layout (NULL); return");
            return;
        }
		nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_TRUSTED_ROOT_CA_CERTIFICATES", NULL, NULL, no_content, NULL);
    }

    elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, back_cb, (struct Evas_Object *)ad);  
	
}
