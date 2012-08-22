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
 * @file        pfx_cert.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#include <dirent.h>

static CertSvcStringList stringList;

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);

static Elm_Genlist_Item_Class itc_2text = { .item_style = "2text.2", .func.text_get = _gl_text_get };

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;

    CertSvcString buffer;
    if (certsvc_string_list_get_one(stringList, index, &buffer) != CERTSVC_SUCCESS) {
        return strdup("ERROR WHILE LOADING STRING");
    }
    const char *char_buffer;
    certsvc_string_to_cstring(buffer, &char_buffer, NULL);

    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        return strdup(char_buffer);
    } else if (!strcmp(part, "elm.text.2")) {
        return strdup("example@address.net");
//        TODO: get real email addresses when
//        certsvc_pkcs12_email_dup function will be ready
    }
    certsvc_string_free(buffer);
    return NULL;
}

static void _certsvc_free_instance(void *data, Evas_Object *obj, void *event_info) {
    LOGD("_certsvc_free_instance");
    struct ug_data *ad = (struct ug_data *) data;
    certsvc_instance_free(ad->instance);
}

void pfx_cert_create_list(struct ug_data *ad){

    certsvc_pkcs12_get_id_list(ad->instance, &stringList);

    elm_genlist_clear(ad->genlist_pfx);

    int i;
    int list_length;
    certsvc_string_list_get_length(stringList, &list_length);
    for (i = 0; i < list_length; i++) {
        elm_genlist_item_append(ad->genlist_pfx, &itc_2text, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, genlist_clicked_cb, NULL);
    }
}

void pfx_cert_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_cb");

    struct ug_data *ad = (struct ug_data *) data;

    if (certsvc_instance_new(&(ad->instance)) == CERTSVC_FAIL) {
        LOGD("CERTSVC_FAIL");
        return;
    }

    Evas_Object *toolbar = elm_toolbar_add(ad->win_main);
    if (!toolbar)
        return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);

    elm_toolbar_item_append(toolbar, NULL, dgettext(PACKAGE, "INSTALL"), pfx_cert_install_cb, ad);
    elm_toolbar_item_append(toolbar, NULL, dgettext(PACKAGE, "UNINSTALL"), pfx_cert_remove_cb, ad);

    ad->genlist_pfx = elm_genlist_add(ad->win_main);

    pfx_cert_create_list(ad); // creating genlist

    Elm_Object_Item *itm = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "PFX_CERTIFICATE"), NULL, NULL,
            ad->genlist_pfx, NULL);
    elm_object_item_part_content_set(itm, "controlbar", toolbar);

    Evas_Object *back = NULL;
    back = elm_object_item_part_content_get(itm, "prev_btn");
    evas_object_smart_callback_add(back, "clicked", _certsvc_free_instance, ad);
}
