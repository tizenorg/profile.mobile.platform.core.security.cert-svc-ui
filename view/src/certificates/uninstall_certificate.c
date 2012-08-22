/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of the Manage Applications
 * Written by Eunmi Son <eunmi.son@samsung.com>
 *
 * Author of this file:
 * Janusz Kozerski <j.kozerski@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with SAMSUNG ELECTRONICS.
 *
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not limited
 * to the implied warranties of merchantability, fitness for a particular
 * purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or
 * distributing this software or its derivatives.
 *
 */

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static char* __get_text_check(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__get_content_check(void *data, Evas_Object *eo, const char *part);
static char* __get_text_select_all(void *data, Evas_Object *obj, const char *part);
static Evas_Object *__get_content_select_all(void *data, Evas_Object *eo, const char *part);
static void select_all_fn();
static void deselect_all_fn();
static void uninstall_cb(void *data, Evas_Object *obj, void *event_info);
static void cancel_cb(void *data, Evas_Object *obj, void *event_info);

static char *dir_path;

static struct ListElement *firstListElement = NULL;
static struct ListElement *lastListElement = NULL;

static Elm_Genlist_Item_Class itc_check = { .item_style = "dialogue/1text.1icon", .func.content_get =
        __get_content_check, .func.text_get = __get_text_check };

static char* __get_text_check(void *data, Evas_Object *obj, const char *part) {
    struct ListElement *current = (struct ListElement*) data;
    LOGD("__get_text_check --- uninstall");
    return strdup(current->title);
}

static Evas_Object *__get_content_check(void *data, Evas_Object *eo, const char *part) {
    struct ListElement *current = (struct ListElement*) data;

    Evas_Object *toggle = elm_check_add(eo);

    elm_check_state_set(toggle, current->isChecked);
    LOGD("toggle <---> %s.isChecked", current->name);

    evas_object_pass_events_set(toggle, EINA_TRUE);

    return toggle;
}

static void __toggle(void *data, Evas_Object *obj, void *event_info) {
    struct ListElement *current = (struct ListElement*) data;
    if (current->isChecked) {
        current->isChecked = EINA_FALSE;
        LOGD("isChecked should be changed to FASLE --- %s", current->name);
    } else {
        current->isChecked = EINA_TRUE;
        LOGD("isChecked should be changed to TRUE --- %s", current->name);
    }

    elm_genlist_item_update(event_info);
    elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

//----------------------------------

static Eina_Bool select_all = EINA_FALSE;

static Elm_Genlist_Item_Class itc_select_all = { .item_style = "dialogue/1text.1icon", .func.content_get =
        __get_content_select_all, .func.text_get = __get_text_select_all };

static char* __get_text_select_all(void *data, Evas_Object *obj, const char *part) {
    return strdup(dgettext(PACKAGE, "SELECT_ALL"));
}

static Evas_Object *__get_content_select_all(void *data, Evas_Object *eo, const char *part) {

    Evas_Object *toggle = elm_check_add(eo);
    elm_check_state_set(toggle, select_all);
    evas_object_pass_events_set(toggle, EINA_TRUE);

    return toggle;
}

static void __toggle_select_all(void *data, Evas_Object *obj, void *event_info) {

    if (!select_all) {
        select_all = EINA_TRUE;
        select_all_fn();
        LOGD("select_all should be changed to FASLE");
    } else {
        select_all = EINA_FALSE;
        deselect_all_fn();
        LOGD("select_all should be changed to TRUE");
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

//----------------------------------

void delete_cert_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("delete_cert_cb -------------------------");

    struct ug_data *ad = get_ug_data();

    Evas_Object *genlist = NULL;
    Eina_Bool onlyOnce = EINA_TRUE;
    firstListElement = initList();
    lastListElement = firstListElement;

    dir_path = (char *) data;

    Evas_Object *toolbar = elm_toolbar_add(ad->win_main);
    if (!toolbar)
        return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);

    elm_toolbar_item_append(toolbar, NULL, dgettext(PACKAGE, "UNINSTALL"), uninstall_cb, ad);
    elm_toolbar_item_append(toolbar, NULL, dgettext(PACKAGE, "CANCEL"), cancel_cb, ad);

    genlist = elm_genlist_add(ad->win_main);

    if (!make_list(ad, genlist, dir_path, lastListElement, TO_UNINSTALL)) {
        struct ListElement *current = findFirstElement(firstListElement);
        current = nextElement(current);
        if (onlyOnce) {
            onlyOnce = EINA_FALSE;
            elm_genlist_item_append(genlist, &itc_select_all, NULL, NULL, ELM_GENLIST_ITEM_NONE, __toggle_select_all,
                    NULL);
            LOGD("Select All field added");
        }
        while (current) {
            LOGD("current->title: %s", current->title);
            Elm_Object_Item * it;
            it = elm_genlist_item_append(genlist, &itc_check, current, NULL, ELM_GENLIST_ITEM_NONE, __toggle, current);
            current->it = it;
            LOGD("state pointer set to: %s", current->name);
            current = nextElement(current);
        }
    }

    Elm_Object_Item *itm = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "UNINSTALL"), NULL, NULL, genlist,
            NULL);
    elm_object_item_part_content_set(itm, "controlbar", toolbar);

    Evas_Object *back = NULL;
    back = elm_object_item_part_content_get(itm, "prev_btn");
    evas_object_smart_callback_add(back, "clicked", NULL, NULL);

}

static void uninstall_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("uninstall_cb");

    struct ug_data *ad = (struct ug_data *) data;

    struct ListElement* current = firstListElement->next; // TODO: first list element is not passed by args!!!
    Eina_Bool nothingToUninstall = EINA_TRUE;

    struct ListElement* tmpListElement;

    char buf[MAX_PATH_LENGHT];

    while (current != NULL) {
        if (current->isChecked) {
            sprintf(buf, "%s/%s", dir_path, current->name);
            LOGD("remove ( %s )", buf);
            remove(buf);
            LOGD("Uninstalled succesful -- %s!", current->name);
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
}

static void cancel_cb(void *data, Evas_Object *obj, void *event_info) {
    struct ug_data *ad = (struct ug_data *) data;

    elm_naviframe_item_pop(ad->navi_bar);
}
