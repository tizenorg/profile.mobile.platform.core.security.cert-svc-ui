/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        cert-selection.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <Elementary.h>
#include "dlog.h"

#include <stdio.h>
#include <dirent.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "cert-selection-uigadget.h"

#define CERT_MAX_DATA_SIZE  256

static CertSvcInstance instance;
static CertSvcStringList stringList;
static Elm_Genlist_Item_Class itc;
static int state_index = 0; //selected radio index
static char *selected_name = NULL;
static Eina_Bool selected = EINA_FALSE;
static Evas_Object *radio_main = NULL;

static void _open(void *data, Evas_Object *obj, void *event_info) {

    LOGD("selected index = %d", state_index);
    (void)obj;
    (void)event_info;

    struct ug_data *ad = (struct ug_data*) data;

    if (selected) {
        service_h service;
        service_create(&service);
        service_add_extra_data(service, "selected-cert", selected_name);
        ug_send_result(ad->ug, service);
        LOGD("result send");
    }

    /* bg must delete before starting on_destroy */
    LOGD("Closing UG");
    free(selected_name);
    evas_object_del(ad->bg);
    ad->bg = NULL;
    ug_destroy_me(ad->ug);
}

static void _cancel(void *data, Evas_Object *obj, void *event_info) {

    (void)obj;
    (void)event_info;

    struct ug_data *ad = (struct ug_data*) data;

    /* bg must delete before starting on_destroy */
    LOGD("Closing UG");
    free(selected_name);
    evas_object_del(ad->bg);
    ad->bg = NULL;
    ug_destroy_me(ad->ug);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

    (void)obj;
    int index = (int) data;

    CertSvcString buffer;
    if (certsvc_string_list_get_one(stringList, index, &buffer) != CERTSVC_SUCCESS) {
        return strdup("ERROR WHILE LOADING STRING");
    }
    char *char_buffer;
    char_buffer = strndup(buffer.privateHandler, buffer.privateLength);
    certsvc_string_free(buffer);

    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        return char_buffer;
    } else if (!strcmp(part, "elm.text.2")) {
        return strdup("example@address.net");
    }

    return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;
    Evas_Object *radio;

    if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
        radio = elm_radio_add(obj);
        elm_radio_state_value_set(radio, index);
        elm_radio_group_add(radio, radio_main);
        if (index == state_index)
            elm_radio_value_set(radio, state_index);
        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

        return radio;
    }
    return NULL;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part) {

    (void)data;
    (void)obj;
    (void)part;

    return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj) {

    (void)data;
    (void)obj;

    return;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info) {

    (void)obj;

    int pkcs_index = (int) data;
    int index;
    Elm_Object_Item *item = (Elm_Object_Item *) event_info;

    if (item) {
        index = (int) elm_object_item_data_get(item);
        state_index = index;
        elm_genlist_item_update(item);
    }
    selected = EINA_TRUE;

    CertSvcString buffer;
    if (certsvc_string_list_get_one(stringList, pkcs_index, &buffer) != CERTSVC_SUCCESS) {
        selected = EINA_FALSE;
        return;
    }
    if(selected_name)
        free(selected_name);
    selected_name = malloc((buffer.privateLength+1) * sizeof(char));
    strncpy(selected_name, buffer.privateHandler, buffer.privateLength);
    selected_name[buffer.privateLength] = 0;
    LOGD("SELECTED NAME = %s", selected_name);
    certsvc_string_free(buffer);
}

void cert_selection_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("cert_selection");

    (void)data;
    (void)obj;
    (void)event_info;

    struct ug_data *ad = (struct ug_data *) data;
    Evas_Object *genlist = NULL;
    Evas_Object *layout = NULL;
    Evas_Object *toolbar = NULL;

    layout = elm_layout_add(ad->win_main);
    if (!layout)
        return;

    toolbar = elm_toolbar_add(layout);
    if (!toolbar)
        return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);

    elm_toolbar_item_append(toolbar, NULL, "Open", _open, ad);
    elm_toolbar_item_append(toolbar, NULL, "Cancel", _cancel, ad);

    // Create genlist;
    genlist = elm_genlist_add(layout);
    if (!radio_main) {
        radio_main = elm_radio_add(genlist);
        elm_radio_state_value_set(radio_main, 0);
        elm_radio_value_set(radio_main, 0);
    }

    // Set genlist item class
    itc.item_style = "2text.1icon.2";
    itc.func.text_get = _gl_text_get;
    itc.func.content_get = _gl_content_get;
    itc.func.state_get = _gl_state_get;
    itc.func.del = _gl_del;

    if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
        LOGD("CERTSVC_FAIL");
        return;
    }
    certsvc_pkcs12_get_id_list(instance, &stringList);

    int i;
    int list_length;
    certsvc_string_list_get_length(stringList, &list_length);
    for (i = 0; i < list_length; i++) {
        elm_genlist_item_append(genlist, &itc, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, (void*) i);

    }

    Elm_Object_Item *itm = elm_naviframe_item_push(ad->navi_bar, "Pick Certificate", NULL, NULL, genlist, NULL);
    elm_object_item_part_content_set(itm, "controlbar", toolbar);

    LOGD("end of cert_selection");
}
