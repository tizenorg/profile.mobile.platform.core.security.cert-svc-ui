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
 * @file        user_cert.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */
#include <dlog.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#include <dirent.h>

Eina_Bool user_cert_create_list(struct ug_data *ad) {
    LOGD("user_cert_create_list()");
    elm_list_clear(ad->list_to_refresh);
    return make_list(ad, ad->list_to_refresh, PATH_CERT_USER, ad->list_element, USER);
}

void user_cert_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("user_cert_cb()");

    if(NULL == data){
        return;
    }
    struct ug_data *ad = (struct ug_data *) data;
    ad->type_of_screen = USER_SCREEN;

    Evas_Object *install_button;
    Evas_Object *uninstall_button;

    struct ListElement *firstListElement = NULL;
    struct ListElement *lastListElement  = NULL;
    firstListElement = initList();
    lastListElement  = firstListElement;
    ad->list_element = firstListElement;

    install_button = elm_button_add(ad->navi_bar);
    if (!install_button)
        return;
    elm_object_text_set(install_button, dgettext(PACKAGE, "IDS_ST_BUTTON_INSTALL"));
    elm_object_style_set(install_button, "naviframe/toolbar/left");
    evas_object_smart_callback_add(install_button, "clicked", install_button_cb, USER);

    uninstall_button = elm_button_add(ad->navi_bar);
    if (!uninstall_button)
        return;
    elm_object_text_set(uninstall_button, dgettext(PACKAGE, "IDS_ST_BUTTON_UNINSTALL"));
    elm_object_style_set(uninstall_button, "naviframe/toolbar/right");
    evas_object_smart_callback_add(uninstall_button, "clicked", delete_cert_cb, PATH_CERT_USER);

    ad->list_to_refresh = NULL;
    ad->list_to_refresh = elm_list_add(ad->win_main);
    elm_list_mode_set(ad->list_to_refresh, ELM_LIST_COMPRESS);

    Elm_Object_Item *itm = NULL;
    if (!user_cert_create_list(ad)) { // There is some content


        itm = elm_naviframe_item_push(
                ad->navi_bar,
                dgettext(PACKAGE, "IDS_ST_BODY_EXTERNAL_CERTIFICATES"),
                NULL,
                NULL,
                ad->list_to_refresh,
                NULL);

        elm_object_disabled_set(uninstall_button, EINA_FALSE);


    } else { // No content
        Evas_Object *no_content = create_no_content_layout(ad);

        if(!no_content){
            LOGD("Cannot create no_content layout (NULL); return");
            return;
        }
        itm = elm_naviframe_item_push(
                ad->navi_bar,
                dgettext(PACKAGE, "IDS_ST_BODY_EXTERNAL_CERTIFICATES"),
                NULL,
                NULL,
                no_content,
                NULL);

        elm_object_disabled_set(uninstall_button, EINA_TRUE);
    }

    elm_object_item_part_content_set(itm, "toolbar_button1", install_button);
    elm_object_item_part_content_set(itm, "toolbar_button2", uninstall_button);

    Evas_Object *back = NULL;
    back = elm_object_item_part_content_get(itm, "prev_btn");
    evas_object_smart_callback_add(back, "clicked", back_cb, ad);
}
