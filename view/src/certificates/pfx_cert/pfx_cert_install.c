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
#define _GNU_SOURCE

#include <dirent.h>

#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static const char * const dir_path = PATH_SDCARD;

static void install_pfx_button_cb(void *data, Evas_Object *obj, void *event_info);

static struct ListElement *firstListElement = NULL;
static struct ListElement *lastListElement = NULL;

void pfx_cert_install_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_cb");

    struct ug_data *ad = (struct ug_data *) data;
    Evas_Object *list = NULL;

    DIR *dir;
    struct dirent *dp;

    firstListElement = initList();
    lastListElement = firstListElement;

    list = elm_list_add(ad->win_main);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);



    dir = opendir(dir_path);
    if (dir == NULL) {
        LOGE("There's no such directory: %s", dir_path);
        return; //TODO What if there's no SD card?
    }

    LOGD("Scanning dir (%s) - looking for certs", dir_path);
    while ((dp = readdir(dir)) != NULL) {
        char *tmp;
        tmp = path_cat(dir_path, dp->d_name);
        char *dot = strrchr(dp->d_name, '.');

        if(dot != NULL && strlen(dot)>3 && (strncmp(dot, ".pfx", 4) == 0 || strncmp(dot, ".PFX", 4) == 0 ||
                             strncmp(dot, ".p12", 4) == 0 || strncmp(dot, ".P12", 4) == 0)) {
            if (!(dp->d_type == DT_DIR)) {
                Elm_Object_Item * it;
                struct ListElement *current;
                current = addListElement(lastListElement, dp->d_name);
                lastListElement = current;
                it = elm_list_item_append(list, dp->d_name, NULL, NULL, install_pfx_button_cb, current);
                LOGD("elm list append = %s", current->name);
            }
            if (tmp) {
                free(tmp);
                tmp = NULL;
            }
        }
    }
    closedir(dir);

    elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "CHOOSE_PFX_TO_INSTALL"), NULL, NULL, list, NULL);
}

static void install_pfx_button_cb(void *data, Evas_Object *obj, void *event_info) {

    struct ListElement *current = (struct ListElement *) data;
    struct ug_data *ad = get_ug_data();
    Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
    if (it){
        elm_list_item_selected_set(it, EINA_FALSE);
    }

    char *path;
    CertSvcString certSvcString_path;
    int result;
    result = asprintf(&path, "%s/%s", dir_path, current->name);
    if(result == -1){
        LOGD("aspfintf failed (-1)");
        return;
    }
    certsvc_string_new(ad->instance, path, strlen(path), &certSvcString_path);

    int returned_value;
    if(certsvc_pkcs12_has_password(ad->instance, certSvcString_path, &returned_value)
            != CERTSVC_SUCCESS){
        LOGD("Wrong PKCS12 or PFX file.");
        elm_naviframe_item_pop(ad->navi_bar);
        return;
    }

    switch (returned_value){
    case CERTSVC_TRUE:
        LOGD("%s is passwod protected", current->name);
        put_pkcs12_name_and_pass_cb(current, NULL, NULL);
        return;

    case CERTSVC_FALSE:
        LOGD("%s is NOT passwod protected", current->name);
        put_pkcs12_name_cb(current, NULL, NULL);
        return;
    }
}
