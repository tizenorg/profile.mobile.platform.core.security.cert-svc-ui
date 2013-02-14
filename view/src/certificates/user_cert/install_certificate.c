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
 * @file        install_certificate.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#define _GNU_SOURCE

#include <dlog.h>
#include <stdlib.h>
#include <time.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

// add current date on the end of destination filename
static void copy_add_date(char *source, char *dest);

void user_search_cert_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("user_search_cert_cb()");
    if(NULL == data){
        return;
    }
    struct ug_data *ad = (struct ug_data *) data;
    Evas_Object *list      = NULL;
    Eina_Bool   is_content = EINA_FALSE;

    struct ListElement *firstListElement = initList();
    struct ListElement *lastListElement = firstListElement;
    ad->list_element_install = lastListElement;

    list = elm_list_add(ad->win_main);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(list, "selected", list_clicked_cb, NULL);

    if(!make_list(ad, list, PATH_SDCARD, lastListElement, TO_INSTALL)){
        is_content = EINA_TRUE;
    }
    lastListElement = findLastElement(lastListElement);

    if(!make_list(ad, list, PATH_MEDIA, lastListElement, TO_INSTALL)){
        is_content = EINA_TRUE;
    }
    lastListElement = findLastElement(lastListElement);

    if(!make_list(ad, list, PATH_MEDIA_DOWNLOADS, lastListElement, TO_INSTALL)){
        is_content = EINA_TRUE;
    }

    Elm_Object_Item *itm = NULL;
    if(is_content) {
        itm = elm_naviframe_item_push(
                ad->navi_bar,
                dgettext(PACKAGE, "IDS_ST_BODY_SELECT_CERTIFICATE_TO_INSTALL"),
                NULL,
                NULL,
                list,
                NULL);
    }
    else { // No content
        Evas_Object *no_content = create_no_content_layout(ad);

        if(!no_content){
            LOGD("Cannot create no_content layout (NULL); return");
            return;
        }
        itm = elm_naviframe_item_push(
                ad->navi_bar,
                dgettext(PACKAGE, "IDS_ST_BODY_SELECT_CERTIFICATE_TO_INSTALL"),
                NULL,
                NULL,
                no_content,
                NULL);
    }

    Evas_Object *back = NULL;
    back = elm_object_item_part_content_get(itm, "prev_btn");
    evas_object_smart_callback_add(back, "clicked", back_install_cb, ad);
}

void install_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("install_cb");
    struct ug_data *ad = get_ug_data();
    if (NULL == data)
        return;
    struct ListElement *listElement = (struct ListElement *) data;
    char *file_name = listElement->name;
    LOGD("%s", file_name);
    char *src = NULL;
    char *dst = NULL;
    src = path_cat(listElement->path, listElement->name);
    if (!src)
        return;
    dst = path_cat(PATH_CERT_USER, listElement->name);
    if (!dst){
        free(src);
        return;
    }
    LOGD("Start copying");
    copy_add_date(src, dst);
    free(src);
    free(dst);
    LOGD("End copying");

    user_cert_create_list(ad);

    elm_naviframe_item_pop(ad->navi_bar);
    deleteList(ad->list_element_install);
    ad->list_element_install = NULL;

    elm_naviframe_item_pop(ad->navi_bar);
    user_cert_cb(ad, NULL, NULL); //TODO: This may not be the optimal solution
                                  // Refactoring may be needed
}

static void copy_add_date(char *source, char *dest) {
    LOGD("copy_add_date()");
    if (!source || !dest) {
        LOGD("Null pointer to files");
        return;
    }
    char *command;
    int result;
    char *dest_with_date;
    char *dot;

    dot = strrchr(dest, '.');
    if (NULL != dot) {
        dot[0] = 0;
        result  = asprintf(&dest_with_date, "%s_%ld.%s", dest, time(NULL), &dot[1]);
    }
    else {
        result  = asprintf(&dest_with_date, "%s_%ld", dest, time(NULL));
    }
    if (result < 0) {
        LOGE("Error while allocating memory");
        return;
    }

    result = asprintf(&command, "cp %s %s", source, dest_with_date);
    free(dest_with_date);
    if (command == NULL && result < 0) {
        LOGD("Error while allocating memory");
        return; //Error while allocating memory
    }

    LOGD("%s", command);
    result = system(command);
    LOGD("%s --- return %d", command, result);

    free(command);
    LOGD("copy() - done");
}
