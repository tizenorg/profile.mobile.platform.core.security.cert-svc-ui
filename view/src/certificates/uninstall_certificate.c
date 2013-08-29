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
 * @file        uninstall_certificate.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static char        *__get_text_check         (void *data, Evas_Object *obj, const char *part);
static Evas_Object *__get_content_check      (void *data, Evas_Object *eo, const char *part);
static char        *__get_text_select_all    (void *data, Evas_Object *obj, const char *part);
static Evas_Object *__get_content_select_all (void *data, Evas_Object *eo, const char *part);
static void select_all_fn();
static void deselect_all_fn();
static void uninstall_cb (void *data, Evas_Object *obj, void *event_info);
static void cancel_cb    (void *data, Evas_Object *obj, void *event_info);

static char *dir_path;

static Eina_Bool          select_all        = EINA_FALSE;
static struct ListElement *firstListElement = NULL;
static struct ListElement *lastListElement  = NULL;
static Elm_Object_Item    *select_all_item  = NULL;
static Evas_Object        *uninstallButton  = NULL;

static Elm_Genlist_Item_Class itc_check = {
        .item_style       = "dialogue/1text.1icon",
        .func.content_get = __get_content_check,
        .func.text_get    = __get_text_check
};

static enum CountOfSelected get_count_of_selected(struct ListElement *current){
    LOGD("get_count_of_selected");

    Eina_Bool all  = EINA_TRUE;
    Eina_Bool none = EINA_TRUE;

    if(NULL == current){
        return SELECTED_NONE;
    }

    struct ListElement *listElement = nextElement(findFirstElement(current));
    // first element is not a real element of list.
    // so if there's only one list element it's means that the list is empty.
    while(NULL != listElement){
        if(none && listElement->isChecked){
            none = EINA_FALSE;
        }
        if(all && !(listElement->isChecked)){
            all = EINA_FALSE;
        }
        listElement = nextElement(listElement);
    }

    if(none){
        LOGD("SELECTED_NONE");
        return SELECTED_NONE;
    }
    else if(all){
        LOGD("SELECTED_ALL");
        return SELECTED_ALL;
    }
    // else
    LOGD("SELECTED_FEW");
    return SELECTED_FEW;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

static char* __get_text_check(void *data, Evas_Object *obj, const char *part) {
    struct ListElement *current = (struct ListElement*) data;
    LOGD("__get_text_check --- uninstall");

    if (strlen(current->title) < 1) {
        return strdup(current->name);
    }
    return strdup(current->title);
}

static Evas_Object *__get_content_check(void *data, Evas_Object *eo, const char *part) {
    struct ListElement *current = (struct ListElement*) data;

    Evas_Object *toggle = elm_check_add(eo);

    elm_check_state_pointer_set(toggle, &(current->isChecked));
    SECURE_LOGD("toggle <---> %s.isChecked", current->name);

    return toggle;
}

static void __toggle(void *data, Evas_Object *obj, void *event_info) {

    struct ListElement *current = (struct ListElement*) data;
    if (current->isChecked) {
        current->isChecked = EINA_FALSE;
        SECURE_LOGD("isChecked should be changed to FASLE --- %s", current->name);
    } else {
        current->isChecked = EINA_TRUE;
        SECURE_LOGD("isChecked should be changed to TRUE --- %s", current->name);
    }

    Eina_Bool count_of_selected;
    count_of_selected = get_count_of_selected(current);
    if(SELECTED_ALL == count_of_selected){
        select_all = EINA_TRUE;
        elm_object_disabled_set(uninstallButton, EINA_FALSE);
    }
    else{
        select_all = EINA_FALSE;
        if(SELECTED_NONE == count_of_selected){
            elm_object_disabled_set(uninstallButton, EINA_TRUE);
        }
        else if(SELECTED_FEW == count_of_selected){
            elm_object_disabled_set(uninstallButton, EINA_FALSE);
        }
    }

    elm_genlist_item_update(event_info);
    elm_genlist_item_update(select_all_item);
    elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

//----------------------------------

static Elm_Genlist_Item_Class itc_select_all = {
        .item_style       = "dialogue/1text.1icon",
        .func.content_get = __get_content_select_all,
        .func.text_get    = __get_text_select_all
};

static char* __get_text_select_all(void *data, Evas_Object *obj, const char *part) {
    return strdup(dgettext(PACKAGE, "IDS_ST_BODY_SELECT_ALL"));
}

static Evas_Object *__get_content_select_all(void *data, Evas_Object *eo, const char *part) {

    Evas_Object *toggle = elm_check_add(eo);
    elm_check_state_pointer_set(toggle, &select_all);
    evas_object_pass_events_set(toggle, EINA_TRUE);

    return toggle;
}

static void __toggle_select_all(void *data, Evas_Object *obj, void *event_info) {

    if (!select_all) {
        select_all = EINA_TRUE;
        select_all_fn();
        elm_object_disabled_set(uninstallButton, EINA_FALSE);
        LOGD("select_all should be changed to TURE");
    } else {
        select_all = EINA_FALSE;
        deselect_all_fn();
        elm_object_disabled_set(uninstallButton, EINA_TRUE);
        LOGD("select_all should be changed to FALSE");
    }

    elm_genlist_item_update(event_info);
    elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static void select_all_fn() {
    LOGD("select_all_fn");
    struct ListElement* current = firstListElement->next;
    while (current != NULL) {
        current->isChecked = EINA_TRUE;
        elm_genlist_item_update(current->it);
        current = current->next;
    }
}

static void deselect_all_fn() {
    LOGD("deselect_all_fn");
    struct ListElement* current = firstListElement->next;
    while (current != NULL) {
        current->isChecked = EINA_FALSE;
        elm_genlist_item_update(current->it);
        current = current->next;
    }
}

static void back_uninstall_cb() {
    LOGD("back_uninstall_cb()");
    struct ug_data *ad = get_ug_data();

    deleteList(ad->list_element_install);
    ad->list_element_install = NULL;

    elm_naviframe_item_pop(ad->navi_bar);
    //TODO: This may not be the optimal solution
    // Refactoring may be needed
    switch (ad->type_of_screen) {
        case TRUSTED_ROOT_SCREEN:
            LOGD("back to TRUSTED_ROOT_SCREEN");
            trusted_root_cert_cb(ad, NULL, NULL);
            break;
        default:
            break;
    }

}

static void cancel_cb(void *data, Evas_Object *obj, void *event_info) {
    struct ug_data *ad = (struct ug_data *) data;

    elm_naviframe_item_pop(ad->navi_bar);
    back_uninstall_cb();
}

//----------------------------------

void delete_cert_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("delete_cert_cb ()");

    if(NULL == data){
        return;
    }
    struct ug_data *ad = get_ug_data();

     //*uninstall_button;
    Evas_Object *cancel_button;
    Evas_Object *genlist = NULL;
    Eina_Bool   onlyOnce = EINA_TRUE;
    firstListElement = initList();
    lastListElement  = firstListElement;
    ad->list_element_install = firstListElement;

    dir_path = (char *) data;

    uninstallButton = elm_button_add(ad->navi_bar);
    if (!uninstallButton)
        return;
    elm_object_domain_translatable_text_set(uninstallButton, PACKAGE, "IDS_ST_BUTTON_UNINSTALL");
    elm_object_style_set(uninstallButton, "naviframe/toolbar/left");
    evas_object_smart_callback_add(uninstallButton, "clicked", uninstall_cb, ad);
    elm_object_disabled_set(uninstallButton, EINA_TRUE);

    cancel_button = elm_button_add(ad->navi_bar);
    if (!cancel_button)
        return;
    elm_object_domain_translatable_text_set(cancel_button, PACKAGE, "IDS_ST_SK2_CANCEL");
    elm_object_style_set(cancel_button, "naviframe/toolbar/right");
    evas_object_smart_callback_add(cancel_button, "clicked", cancel_cb, ad);

    genlist = elm_genlist_add(ad->win_main);

    evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

    if (!make_list(ad, genlist, dir_path, lastListElement, TO_UNINSTALL)) {
        struct ListElement *current = findFirstElement(firstListElement);
        current = nextElement(current);
        if (onlyOnce) {
            onlyOnce = EINA_FALSE;
            select_all_item = elm_genlist_item_append(genlist, &itc_select_all, NULL, NULL, ELM_GENLIST_ITEM_NONE, __toggle_select_all,
                    NULL);
            select_all = EINA_FALSE;
            LOGD("Select All field added");
        }
        while (current) {
            LOGD("current->title: %s", current->title);
            Elm_Object_Item * it;
            it = elm_genlist_item_append(genlist, &itc_check, current, NULL, ELM_GENLIST_ITEM_NONE, __toggle, current);
            current->it = it;
            SECURE_LOGD("state pointer set to: %s", current->name);
            current = nextElement(current);
        }
    }

    Elm_Object_Item *itm = elm_naviframe_item_push(
            ad->navi_bar,
            dgettext(PACKAGE, "IDS_ST_BUTTON_UNINSTALL"),
            NULL,
            NULL,
            genlist,
            NULL);
    elm_object_item_domain_text_translatable_set(itm, PACKAGE, EINA_TRUE);
    elm_object_item_part_content_set(itm, "toolbar_button1", uninstallButton);
    elm_object_item_part_content_set(itm, "toolbar_button2", cancel_button);

    elm_object_item_part_content_unset(itm, "prev_btn");
}

static void uninstall_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("uninstall_cb");

    struct ug_data *ad = (struct ug_data *) data;

    struct ListElement* current = firstListElement->next;
    Eina_Bool nothingToUninstall = EINA_TRUE;

    struct ListElement* tmpListElement;

    char *buf = NULL;

    while (current != NULL) {
        if (current->isChecked) {
            buf =  path_cat(current->path, current->name);
            SECURE_LOGD("remove ( %s )", buf);
            if (remove(buf)){
                LOGE("Fail in removing path.");
            }
            SECURE_LOGD("Uninstalled succesful -- %s!", current->name);
            nothingToUninstall = EINA_FALSE;
        }
        tmpListElement = current;
        current = current->next;
        removeListElement(tmpListElement);
    }
    if (nothingToUninstall)
        LOGD("Nothing to uninstall");

    refresh_list(ad);
    elm_naviframe_item_pop(ad->navi_bar);
    back_uninstall_cb();
}
