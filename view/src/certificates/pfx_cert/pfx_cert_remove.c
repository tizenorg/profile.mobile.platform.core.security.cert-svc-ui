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
 * @file        pfx_cert_remove.c
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
static Eina_Bool *state_pointer; //check states
static int list_length = 0;

static void _select_all_check_changed_cb(void *data, Evas_Object *obj, void *event_info){

    int i = 0;
    Eina_List * realized_list;
    Elm_Object_Item *it;
    Eina_List *l;
    Eina_Bool checked;
    checked = elm_check_state_get(obj);
    for (i = 1; i <= list_length; i++) {
        state_pointer[i] = checked;
    }
    // Update realized items only
    realized_list = elm_genlist_realized_items_get(data);
    EINA_LIST_FOREACH(realized_list, l, it){
        elm_genlist_item_update(it);
    }
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info){

    int index;
    Elm_Object_Item *item = (Elm_Object_Item *) event_info;
    if (item != NULL) {
        index = (int) elm_object_item_data_get(item);
        elm_genlist_item_selected_set(item, EINA_FALSE);
        state_pointer[index] = !state_pointer[index];
        elm_genlist_item_update(item);
        if ((state_pointer[index] == EINA_FALSE) && (state_pointer[0] == EINA_TRUE)) {
            state_pointer[0] = EINA_FALSE;
        }
    }
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part) {

    return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj) {

    return;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part) {
    int index = (int) data;
    Evas_Object *check;

    if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
        check = elm_check_add(obj);
        //set the State pointer to keep the current UI state of Checkbox.
        elm_check_state_pointer_set(check, &(state_pointer[index]));

        evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);

        //Select All Checkbox, Callback handler
        if (0 == index) {
            evas_object_smart_callback_add(check, "changed", _select_all_check_changed_cb, obj);
        }
        return check;
    }

    return NULL;
}

static char* _gl_get_text_sa(void *data, Evas_Object *obj, const char *part) {

    return strdup(dgettext(PACKAGE, "SELECT_ALL"));
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;

    CertSvcString buffer;
    if (certsvc_string_list_get_one(stringList, index-1, &buffer) != CERTSVC_SUCCESS) {
        return strdup("ERROR WHILE LOADING STRING");
    }
    const char *char_buffer;
    certsvc_string_to_cstring(buffer, &char_buffer, NULL);

    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        return strdup(char_buffer);
    } else if (!strcmp(part, "elm.text.2")) {
        return strdup("example@address.net");
    }

    return NULL;
}

static Elm_Genlist_Item_Class itc_sa = {
        .item_style = "1text.1icon",
        .func.text_get = _gl_get_text_sa,
        .func.content_get = _gl_content_get,
        .func.state_get = NULL,
        .func.del = NULL
};

static Elm_Genlist_Item_Class itc_2text = {
        .item_style = "2text.1icon.3",
        .func.text_get = _gl_text_get,
        .func.content_get = _gl_content_get,
        .func.state_get = _gl_state_get,
        .func.del = _gl_del
};

static void genlist_pfx_delete_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("genlist_pfx_remove_cb");
    (void)obj;
    (void)event_info;

    struct ug_data *ad = (struct ug_data *) data;
    CertSvcString buffer;

    int i;
    if (EINA_TRUE == state_pointer[0]) {
        for (i = 0; i < list_length; i++) {
            if (certsvc_string_list_get_one(stringList, i, &buffer) == CERTSVC_SUCCESS) {
                certsvc_pkcs12_delete(ad->instance, buffer);
                LOGD("%s --- removed", buffer);
            }
            certsvc_string_free(buffer);
        }
    } else {;
        for (i = 0; i < list_length; i++) {
            if (EINA_TRUE == state_pointer[i+1] &&
                    certsvc_string_list_get_one(stringList, i, &buffer) == CERTSVC_SUCCESS) {
                certsvc_pkcs12_delete(ad->instance, buffer);
                LOGD("%s --- removed", buffer);
                certsvc_string_free(buffer);
            }
        }
    }
    pfx_cert_create_list(ad);
    elm_naviframe_item_pop(ad->navi_bar);
}

void pfx_cert_remove_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_remove_cb");

    struct ug_data *ad = (struct ug_data *) data;
    Evas_Object *genlist = NULL;
    Evas_Object *toolbar = NULL;
    Elm_Object_Item *git = NULL;


    certsvc_pkcs12_get_id_list(ad->instance, &stringList);

    toolbar = elm_toolbar_add(ad->navi_bar);
    if (!toolbar)
        return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);

    elm_toolbar_item_append(toolbar, NULL, dgettext(PACKAGE, "UNINSTALL"), genlist_pfx_delete_cb, ad);

    genlist = elm_genlist_add(ad->navi_bar);

    git = elm_genlist_item_append(genlist, &itc_sa, (void *) 0, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    int i;
    certsvc_string_list_get_length(stringList, &list_length);
    state_pointer = malloc((list_length+1) * sizeof(Eina_Bool));
    state_pointer[0] = EINA_FALSE;
    for (i = 1; i < list_length + 1; i++) {
        state_pointer[i] = EINA_FALSE;
        elm_genlist_item_append( genlist, &itc_2text, (void *) i, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

    }
    Elm_Object_Item *itm = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "PFX_CERTIFICATE"), NULL, NULL,
            genlist, NULL);
    elm_object_item_part_content_set(itm, "controlbar", toolbar);
}
