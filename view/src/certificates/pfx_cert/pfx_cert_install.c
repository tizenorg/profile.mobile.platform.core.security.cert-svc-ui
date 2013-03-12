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
 * @file        pfx_cert_install.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include <dirent.h>
#include <dlog.h>

#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static void install_pfx_button_cb (void *data, Evas_Object *obj, void *event_info);

static void _clear_list (struct ListElement *listElement){
    deleteList(listElement);
}

static void _back_cb (void *data, Evas_Object *obj, void *event_info){

    if(NULL == data)
        return;
    _clear_list((struct ListElement *) data);
}

static struct ListElement* scan_dir(const char * dir_path, Evas_Object *list, struct ListElement *lastListElement){

    DIR                *dir;
    struct dirent      *dp;
    Elm_Object_Item    *it;
    struct ListElement *current;

    dir = opendir(dir_path);
    if (dir == NULL) {
        LOGE("There's no such directory: %s", dir_path);
        return lastListElement;
    }
    LOGD("Scanning dir (%s) - looking for certs", dir_path);
    while ((dp = readdir(dir)) != NULL) {
        char *tmp = NULL;
        tmp = path_cat(dir_path, dp->d_name);
        char *dot = strrchr(dp->d_name, '.');

        if (dot != NULL && strlen(dot) > 3
                && (strncmp(dot, ".pfx", 4) == 0 || strncmp(dot, ".PFX", 4) == 0 || strncmp(dot, ".p12", 4) == 0
                        || strncmp(dot, ".P12", 4) == 0)) {
            if (!(dp->d_type == DT_DIR)) {
                current = addListElementWithPath(lastListElement, dp->d_name, dir_path);
                lastListElement = current;
                it = elm_list_item_append(list, dp->d_name, NULL, NULL, install_pfx_button_cb, current);
                if (!it){
                    LOGE("Error in elm_list_item_append");
                }

                LOGD("elm list append     = %s", current->name);
                LOGD("elm list append dir = %s", current->path);
            }

        }
        if (tmp) {
            free(tmp);
            tmp = NULL;
        }
    }
    closedir(dir);
    dir = NULL;

    return lastListElement;
}

void pfx_cert_install_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_cb");

    if(NULL == data){
        return;
    }
    struct ug_data *ad   = (struct ug_data *) data;
    Evas_Object    *list = NULL;

    struct ListElement *firstListElement = initList();
    struct ListElement *lastListElement  = firstListElement;

    list = elm_list_add(ad->win_main);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);

    lastListElement = scan_dir(PATH_SDCARD, list, lastListElement);
    lastListElement = scan_dir(PATH_MEDIA, list, lastListElement);
    scan_dir(PATH_MEDIA_DOWNLOADS, list, lastListElement);

    Elm_Object_Item *navi_it = NULL;
    if(firstListElement->next) {
        navi_it = elm_naviframe_item_push(
                ad->navi_bar,
                dgettext(PACKAGE, "IDS_ST_BODY_SELECT_CERTIFICATE_TO_INSTALL"),
                NULL,
                NULL,
                list,
                NULL);
    }
    else { //No content
        Evas_Object *no_content = create_no_content_layout(ad);

        if(!no_content){
            LOGD("Cannot create no_content layout (NULL); return");
            return;
        }
        navi_it = elm_naviframe_item_push(
                ad->navi_bar,
                dgettext(PACKAGE, "IDS_ST_BODY_SELECT_CERTIFICATE_TO_INSTALL"),
                NULL,
                NULL,
                no_content,
                NULL);
    }

    Evas_Object *back = elm_object_item_part_content_get(navi_it, "prev_btn");
    evas_object_smart_callback_add(back, "clicked", _back_cb, firstListElement);
}

static void install_pfx_button_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("install_pfx_button_cb()");

    struct ListElement *current = (struct ListElement *) data;
    struct ug_data *ad = get_ug_data();
    Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
    if (it){
        elm_list_item_selected_set(it, EINA_FALSE);
    }

    char *path = NULL;
    CertSvcString certSvcString_path;

    path = path_cat(current->path, current->name);
    if(!path){
        LOGD("path_cat returned NULL; return");
        return;
    }

    certsvc_string_new(ad->instance, path, strlen(path), &certSvcString_path);

    int returned_value;
    if(certsvc_pkcs12_has_password(ad->instance, certSvcString_path, &returned_value)
            != CERTSVC_SUCCESS){
        LOGD("Wrong PKCS12 or PFX file.");
        elm_naviframe_item_pop(ad->navi_bar);
        _clear_list(current);
        free(path);
        return;
    }

    switch (returned_value){
    case CERTSVC_TRUE:
        LOGD("%s/%s is passwod protected", current->path, current->name);
        put_pkcs12_name_and_pass_cb(current, NULL, NULL);
        free(path);
        return;

    case CERTSVC_FALSE:
        LOGD("%s/%s is NOT passwod protected", current->path, current->name);
        put_pkcs12_name_cb(current, NULL, NULL);
        free(path);
        return;
    }
    free(path);
}
