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
 * @file        put_password_certificate.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */
#define _GNU_SOURCE

#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static const char * const dir_path = PATH_SDCARD;

static Elm_Genlist_Item_Class itc_btn;
static Elm_Genlist_Item_Class itc_entry;
static Elm_Genlist_Item_Class itc_entry_passwd;
static struct ListElement *current_file;

static void _install_button_cb(void *data, Evas_Object *obj, void *event_info);
static void _cancell_button_cb(void *data, Evas_Object *obj, void *event_info);

struct Evas_Object_Menu_Entry {
    char *label;
    char *content;
    Evas_Object *eo;
    Evas_Smart_Cb callback;
};

static Elm_Object_Item *passwd_entry, *name_entry;

static void _content_changed_cb(void *data, Evas_Object *_obj, void *event_info)
{
    LOGD("_content_changed_cb");
    struct Evas_Object_Menu_Entry *mdata = (struct Evas_Object_Menu_Entry*)data;
    LOGD("content get = %s", elm_entry_entry_get(mdata->eo));
    strncpy(mdata->content, elm_entry_entry_get(mdata->eo), MAX_PATH_LENGHT);
    LOGD("_content_changed_cb --- finished");
}

static Evas_Object *_gl_button_get(void *data, Evas_Object *obj, const char *part)
{
    struct Evas_Object_Menu_Entry *mdata = (struct Evas_Object_Menu_Entry*)data;

    mdata->eo = elm_button_add(obj);
    elm_object_text_set(mdata->eo, mdata->label);
    evas_object_smart_callback_add(mdata->eo, "clicked", mdata->callback, obj);

    return mdata->eo;
}

static Evas_Object *_gl_entry_get(void *data, Evas_Object *obj, const char *part)
{
    struct Evas_Object_Menu_Entry *mdata = (struct Evas_Object_Menu_Entry*)data;

    mdata->eo = elm_entry_add(obj);
    elm_object_text_set(mdata->eo, mdata->content);
    elm_entry_single_line_set(mdata->eo, EINA_TRUE);
    evas_object_smart_callback_add(mdata->eo, "changed",_content_changed_cb, mdata);
    return mdata->eo;
}

static Evas_Object *_gl_entry_get_passwd(void *data, Evas_Object *obj, const char *part)
{
    struct Evas_Object_Menu_Entry *mdata = (struct Evas_Object_Menu_Entry*)data;

    mdata->eo = elm_entry_add(obj);
    elm_object_text_set(mdata->eo, mdata->content);
    elm_entry_single_line_set(mdata->eo, EINA_TRUE);
    elm_entry_password_set(mdata->eo, EINA_TRUE);
    elm_entry_input_panel_return_key_type_set(mdata->eo, ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);
    elm_entry_line_wrap_set(mdata->eo, ELM_WRAP_NONE);
    evas_object_smart_callback_add(mdata->eo, "changed",_content_changed_cb, mdata);
    return mdata->eo;
}

static char *_gl_label_eo_get(void *data, Evas_Object *obj, const char *part)
{
    struct Evas_Object_Menu_Entry *dd = (struct Evas_Object_Menu_Entry*)data;

    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        return strdup(dd->label);
    }
    return NULL;
}

static void _gl_del_eo(void *data, Evas_Object *_obj)
{
    struct Evas_Object_Menu_Entry *dd = (struct Evas_Object_Menu_Entry*)data;

    if (dd->content) {
        free(dd->content);
        dd->content = NULL;
    }
}

static void _set_itc_classes(void)
{
    itc_btn.item_style = "dialogue/1icon";
    itc_btn.func.content_get = _gl_button_get;

    itc_entry.item_style = "dialogue/1text.1icon.5";
    itc_entry.func.content_get = _gl_entry_get;
    itc_entry.func.text_get = _gl_label_eo_get;
    itc_entry.func.del = _gl_del_eo;

    itc_entry_passwd.item_style = "dialogue/1text.1icon.5";
    itc_entry_passwd.func.content_get = _gl_entry_get_passwd;
    itc_entry_passwd.func.text_get = _gl_label_eo_get;
    itc_entry_passwd.func.del = _gl_del_eo;
}

static struct Evas_Object_Menu_Entry ENTRIES[] = {
        { .callback = _content_changed_cb },
        { .callback = _content_changed_cb }
};

static struct Evas_Object_Menu_Entry BUTTON_INSTALL = {
        .callback = _install_button_cb
};

static struct Evas_Object_Menu_Entry BUTTON_CANCEL = {
        .callback = _cancell_button_cb
};


static void _cancell_button_cb(void *data, Evas_Object *obj, void *event_info){

    struct ug_data *ad = get_ug_data();

    elm_naviframe_item_pop(ad->navi_bar);
    elm_naviframe_item_pop(ad->navi_bar);
}

static void _install_button_cb(void *data, Evas_Object *obj, void *event_info){

    LOGD("_install_button_cb");

    struct ug_data *ad = get_ug_data();

    char *alias, *password, *path;
    CertSvcString Alias, Path, Password;
    int returned_value;
    int as_return;
    int is_unique = CERTSVC_FALSE;
    CertSvcString pkcs_alias;

    password = ENTRIES[0].content;
    if(strlen(password)==0)
        password = NULL;
    alias = ENTRIES[1].content;
    as_return = asprintf(&path, "%s/%s", dir_path, current_file->name);
    if(as_return == -1){
        LOGD("asprintf failed (-1)");
        return;
    }

    certsvc_string_new(ad->instance, alias, strlen(alias), &pkcs_alias);
    certsvc_pkcs12_alias_exists(ad->instance, pkcs_alias, &is_unique);
    if( CERTSVC_FALSE == is_unique || 1 > strlen(alias) ){
        LOGD("alias %s already exist", alias);
        char buf[BUF256];
        strncpy(buf, dgettext(PACKAGE, "SUCH_ALIAS_ALREADY_EXIST"), BUF256);
        create_ok_pop(ad, buf);
        return;
    }
    LOGD("certsvc_pkcs12_import_from_file( %s, %s, %s)", path, password, alias);
    certsvc_string_new(ad->instance, alias, strlen(alias), &Alias);
    certsvc_string_new(ad->instance, path, strlen(path), &Path);
    certsvc_string_new(ad->instance, (password) ? password : "", (password) ? strlen(password) : 0, &Password);
    returned_value = certsvc_pkcs12_import_from_file(ad->instance, Path, Password, Alias);
    LOGD("certsvc_pkcs12_import_from_file -- result:");

    switch (returned_value){
    case CERTSVC_SUCCESS:
        LOGD("Certificate %s import success", current_file->name);
        pfx_cert_create_list(ad);
        elm_naviframe_item_pop(ad->navi_bar);
        elm_naviframe_item_pop(ad->navi_bar);
        return;

    case CERTSVC_INVALID_PASSWORD:
        LOGD("Invalid password to %s", current_file->name);
        return;

    case CERTSVC_IO_ERROR:
        LOGD("There's no such file!");
        elm_naviframe_item_pop(ad->navi_bar);
        elm_naviframe_item_pop(ad->navi_bar);
        return;

    case CERTSVC_WRONG_ARGUMENT:
        LOGD("Wrong PKCS12 or PFX file.");
        elm_naviframe_item_pop(ad->navi_bar);
        elm_naviframe_item_pop(ad->navi_bar);
        return;

    case CERTSVC_DUPLICATED_ALIAS:
         LOGD("Failed. Such alias already exist.");
         return;

    case CERTSVC_FAIL:
        LOGD("Failed. Wrong password probably.");
        char buf[BUF256];
        strncpy(buf, dgettext(PACKAGE, "WRONG_PASSWORD"), BUF256);
        create_ok_pop(ad, buf);
        return;

    default:
        LOGD("DEFAULT");
        break;
    }
}

void put_pkcs12_name_and_pass_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("put_pkcs12_name_and_pass_cb");
    struct ug_data *ad = get_ug_data();
    current_file = (struct ListElement *) data;

    //allocate mem buffer for entries  - it will be free by genlist
    ENTRIES[0].content = calloc(1, MAX_PATH_LENGHT);
    ENTRIES[1].content = calloc(1, MAX_PATH_LENGHT);

    ENTRIES[0].label = dgettext(PACKAGE, "ENTER_PASSWORD");
    ENTRIES[1].label = dgettext(PACKAGE, "ENTER_KEY_ALIAS");

    BUTTON_INSTALL.label = dgettext(PACKAGE, "INSTALL");
    BUTTON_CANCEL.label  = dgettext(PACKAGE, "CANCEL");

    _set_itc_classes();
    Evas_Object *genlist = elm_genlist_add(ad->win_main);

    passwd_entry = elm_genlist_item_append(
                       genlist,
                       &itc_entry_passwd,
                       &ENTRIES[0],
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

    name_entry = elm_genlist_item_append(
                       genlist,
                       &itc_entry,
                       &ENTRIES[1],
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

    elm_genlist_item_append(
                       genlist,
                       &itc_btn,
                       &BUTTON_INSTALL,
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

    elm_genlist_item_append(
                       genlist,
                       &itc_btn,
                       &BUTTON_CANCEL,
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

    Elm_Object_Item *navi_it = NULL;
    navi_it = elm_naviframe_item_push(ad->navi_bar, "Install certificate", NULL, NULL, genlist, NULL);
}


void put_pkcs12_name_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("put_pkcs12_name_cb");

    struct ug_data *ad = get_ug_data();
    current_file = (struct ListElement *) data;

    //allocate mem buffer for entries  - it will be free by genlist
    ENTRIES[0].content = calloc(1, MAX_PATH_LENGHT);
    ENTRIES[1].content = calloc(1, MAX_PATH_LENGHT);

    ENTRIES[0].label = dgettext(PACKAGE, "ENTER_PASSWORD");
    ENTRIES[1].label = dgettext(PACKAGE, "ENTER_KEY_ALIAS");

    BUTTON_INSTALL.label = dgettext(PACKAGE, "INSTALL");
    BUTTON_CANCEL.label  = dgettext(PACKAGE, "CANCEL");

    _set_itc_classes();
    Evas_Object *genlist = elm_genlist_add(ad->win_main);
    Elm_Object_Item *name_entry;

    name_entry = elm_genlist_item_append(
                       genlist,
                       &itc_entry,
                       &ENTRIES[1],
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

    elm_genlist_item_append(
                       genlist,
                       &itc_btn,
                       &BUTTON_INSTALL,
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

        elm_genlist_item_append(
                       genlist,
                       &itc_btn,
                       &BUTTON_CANCEL,
                       NULL,
                       ELM_GENLIST_ITEM_NONE,
                       NULL,
                       NULL );

    Elm_Object_Item *navi_it = NULL;
    navi_it = elm_naviframe_item_push(ad->navi_bar, "Install certificate", NULL, NULL, genlist, NULL);
}
