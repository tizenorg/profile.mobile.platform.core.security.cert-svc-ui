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

#include <dlog.h>
#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static const char * const dir_path = PATH_SDCARD;

static Elm_Genlist_Item_Class itc_group;
static Elm_Genlist_Item_Class itc_entry;
static Elm_Genlist_Item_Class itc_entry_passwd;
static struct ListElement *current_file;

static void _install_button_cb(void *data, Evas_Object *obj, void *event_info);
static void _cancel_button_cb(void *data, Evas_Object *obj, void *event_info);

static Elm_Object_Item *passwd_entry, *passwd_entry_label, *name_entry, *name_entry_label;
Evas_Object *_entry = NULL;
Evas_Object *_entry_pass = NULL;

static Evas_Object *_singleline_editfield_add(Evas_Object *parent) {
    LOGD("_singleline_editfield_add");
    Evas_Object *layout;

    layout = elm_layout_add(parent);
    elm_layout_theme_set(layout, "layout", "editfield", "default"); // Default editfield layout style without top title.

    _entry = elm_entry_add(parent);
    elm_entry_scrollable_set(_entry, EINA_TRUE); // Make entry as scrollable single line.
    elm_entry_single_line_set(_entry, EINA_TRUE);
    evas_object_smart_callback_add(_entry, "activated", _install_button_cb, NULL);
    elm_object_part_content_set(layout, "elm.swallow.content", _entry);
    return layout;
}

static void _next_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	 elm_object_focus_set(_entry, EINA_TRUE);
}

static void _focus_set_cb(void *data, Evas_Object *obj, void *event_info)
{
	 struct ug_data *ad = (struct ug_data *)data;
	 elm_object_focus_set(_entry_pass, EINA_TRUE);
	 evas_object_smart_callback_del(ad->navi_bar, "transition,finished", _focus_set_cb);
}

static Evas_Object *_singleline_passfield_add(Evas_Object *parent) {
    LOGD("_singleline_passfield_add");
    Evas_Object *layout;

    layout = elm_layout_add(parent);
    elm_layout_theme_set(layout, "layout", "editfield", "default"); // Default editfield layout style without top title.

    _entry_pass = elm_entry_add(parent);
    elm_entry_scrollable_set(_entry_pass, EINA_TRUE); // Make entry as scrollable single line password.
    elm_entry_single_line_set(_entry_pass, EINA_TRUE);
    elm_entry_password_set(_entry_pass, EINA_TRUE);
    elm_entry_prediction_allow_set(_entry_pass, EINA_FALSE);
    evas_object_smart_callback_add(_entry_pass, "activated", _next_button_cb, NULL);
    elm_object_part_content_set(layout, "elm.swallow.content", _entry_pass);
    return layout;
}

static Evas_Object *_gl_content_edit_get(void *data, Evas_Object *obj, const char *part) {
    if (!strcmp(part, "elm.icon")) {
        return _singleline_editfield_add(obj);
    }
    return NULL;
}

static Evas_Object *_gl_content_pass_get(void *data, Evas_Object *obj, const char *part) {
    if (!strcmp(part, "elm.icon")) {
        return _singleline_passfield_add(obj);
    }
    return NULL;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
    if (event_info)
        elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static char* _gl_get_text_group(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;
    if (0 == index) {
        LOGD("IDS_ST_BODY_ENTER_PASSWORD_C");
        return strdup(dgettext(PACKAGE, "IDS_ST_BODY_ENTER_PASSWORD_C"));
    }
    else if (1 == index) {
        LOGD("IDS_ST_BODY_ENTER_KEY_ALIAS_C");
        return strdup(dgettext(PACKAGE, "IDS_ST_BODY_ENTER_KEY_ALIAS_C"));
    }
    else
        LOGD("Wrong index - return NULL");

    return NULL;
}

static void _set_itc_classes(void) {

    itc_group.item_style = "dialogue/grouptitle";
    itc_group.func.text_get = _gl_get_text_group;
    itc_entry.func.state_get = NULL;
    itc_entry.func.del = NULL;

    itc_entry.item_style = "dialogue/1icon";
    itc_entry.func.text_get = NULL;
    itc_entry.func.content_get = _gl_content_edit_get;
    itc_entry.func.state_get = NULL;
    itc_entry.func.del = NULL;

    itc_entry_passwd.item_style = "dialogue/1icon";
    itc_entry_passwd.func.text_get = NULL;
    itc_entry_passwd.func.content_get = _gl_content_pass_get;
    itc_entry_passwd.func.state_get = NULL;
    itc_entry_passwd.func.del = NULL;

}

static void _cancel_button_cb(void *data, Evas_Object *obj, void *event_info) {

    struct ug_data *ad = get_ug_data();

    elm_naviframe_item_pop(ad->navi_bar);
}

static void _install_button_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("_install_button_cb");

    struct ug_data *ad = get_ug_data();
    if(!ad){
        LOGE("ug_data can't be null!");
        return;
    }
    char *alias, *password, *path;
    CertSvcString Alias, Path, Password;
    int returned_value;
    int is_unique = CERTSVC_FALSE;
    CertSvcString pkcs_alias;
    SECURE_LOGD("alias:    %s", elm_entry_entry_get(_entry));

    if(NULL == elm_entry_entry_get(_entry_pass))
        password = NULL;
    else {
        password = strdup(elm_entry_entry_get(_entry_pass));
        if (0 == strlen(password)){
            free(password);
            password = NULL;
        }
    }
    alias = strdup(elm_entry_entry_get(_entry));
    path = path_cat(current_file->path, current_file->name);
    if (NULL == path) {
        LOGD("patch_cat failed; return");
        goto exit;
    }

    // Empty alias
    if (NULL == alias || 1 > strlen(alias)) {
        LOGD("alias is empty");
        create_ok_pop(ad, "IDS_ST_BODY_ALIAS_CANNOT_BE_EMPTY");
        free(path);
        goto exit;
    }

    certsvc_string_new(ad->instance, alias, strlen(alias), &pkcs_alias);
    certsvc_pkcs12_alias_exists(ad->instance, pkcs_alias, &is_unique);
    // Alias already exists
    if (CERTSVC_FALSE == is_unique || 1 > strlen(alias)) {
        SECURE_LOGD("alias %s already exist", alias);
        create_ok_pop(ad, "IDS_ST_BODY_ALIAS_ALREADY_EXISTS_ENTER_A_UNIQUE_ALIAS");
        free(path);
        goto exit;
    }
    SECURE_LOGD("certsvc_pkcs12_import_from_file(%s, %s)", path, alias);
    certsvc_string_new(ad->instance, alias, strlen(alias), &Alias);
    certsvc_string_new(ad->instance, path, strlen(path), &Path);
    certsvc_string_new(ad->instance, (password) ? password : "", (password) ? strlen(password) : 1, &Password);
    returned_value = certsvc_pkcs12_import_from_file(ad->instance, Path, Password, Alias);
    LOGD("certsvc_pkcs12_import_from_file -- result:");
    free(path);
    
    switch (returned_value) {
    case CERTSVC_SUCCESS:
        SECURE_LOGD("Certificate %s import success", current_file->name);
        if (ad->user_cert_list_item)
        {
        	elm_naviframe_item_pop_to(ad->user_cert_list_item);
        }
        else if (ad->type_of_screen == PKCS12_SCREEN) {
        	quit_cb(ad, NULL); //Exit from UG called directly by cert select UG.
        }
        else {
        	elm_naviframe_item_pop(ad->navi_bar);
        }

        if (ad && ad->refresh_screen_cb)
        {
        	ad->refresh_screen_cb(ad, NULL, NULL);
        }
        break;

    case CERTSVC_INVALID_PASSWORD:
        SECURE_LOGD("Invalid password to %s", current_file->name);
        break;

    case CERTSVC_IO_ERROR:
        LOGD("There's no such file!");
        elm_naviframe_item_pop(ad->navi_bar);
        break;

    case CERTSVC_WRONG_ARGUMENT:
        LOGD("Wrong PKCS12 or PFX file.");
        elm_naviframe_item_pop(ad->navi_bar);
        break;

    case CERTSVC_DUPLICATED_ALIAS:
        // This case should not happened - alias was already checked
        LOGD("Failed. Such alias already exist. This should not happen");
        break;

    case CERTSVC_FAIL:
        LOGD("Failed. Wrong password probably.");
        create_ok_pop(ad, "IDS_ST_BODY_INCORRECT_PASSWORD");
        // Do NOT delete list in this case!!!
        goto exit;

    default:
        LOGD("DEFAULT");
        break;
    }

    deleteList(current_file);

exit:
    free(password);
    free(alias);
    return;
}

static Evas_Object *_create_title_text_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/title_text");
	elm_object_domain_translatable_text_set(btn, PACKAGE, text);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

void put_pkcs12_name_and_pass_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("put_pkcs12_name_and_pass_cb");
    struct ug_data *ad = get_ug_data();
    Evas_Object *btn = NULL;

    current_file = (struct ListElement *) data;

    _set_itc_classes();
    Evas_Object *genlist = elm_genlist_add(ad->navi_bar);

    passwd_entry_label = elm_genlist_item_append(
            genlist,
            &itc_group,
            (void *) 0,
            NULL,
            ELM_GENLIST_ITEM_NONE,
            _gl_sel,
            NULL );

    passwd_entry = elm_genlist_item_append(
            genlist,
            &itc_entry_passwd,
            NULL,
            NULL,
            ELM_GENLIST_ITEM_NONE,
            _gl_sel,
            NULL );

    name_entry_label = elm_genlist_item_append(
            genlist,
            &itc_group,
            (void *) 1,
            NULL,
            ELM_GENLIST_ITEM_NONE,
            _gl_sel,
            NULL );

    name_entry = elm_genlist_item_append(
            genlist,
            &itc_entry,
            NULL,
            NULL,
            ELM_GENLIST_ITEM_NONE,
            _gl_sel,
            NULL );

    evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);
    evas_object_smart_callback_add(ad->navi_bar, "transition,finished", _focus_set_cb, ad);

    Elm_Object_Item *navi_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_HEADER_INSTALL_CERTIFICATE_ABB", NULL, NULL, genlist, NULL);
    if (!navi_it){
        LOGE("Error in elm_naviframe_item_push");
    }

    elm_object_item_domain_text_translatable_set(navi_it, PACKAGE, EINA_TRUE);

    //Title Text Left Button
    btn = _create_title_text_btn(ad->navi_bar, "IDS_ST_BUTTON_INSTALL", _install_button_cb, NULL);
    elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

    //Title Text Right Button
    btn = _create_title_text_btn(ad->navi_bar, "IDS_ST_BUTTON_CANCEL", _cancel_button_cb, NULL);
    elm_object_item_part_content_set(navi_it, "title_left_btn", btn);
}

void put_pkcs12_name_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("put_pkcs12_name_cb");

    struct ug_data *ad = get_ug_data();
    Evas_Object *btn = NULL;

    current_file = (struct ListElement *) data;

   _set_itc_classes();
    Evas_Object *genlist = elm_genlist_add(ad->navi_bar);

    name_entry_label = elm_genlist_item_append(
                genlist,
                &itc_group,
                (void *) 1,
                NULL,
                ELM_GENLIST_ITEM_NONE,
                _gl_sel,
                NULL );

    name_entry = elm_genlist_item_append(
                genlist,
                &itc_entry,
                NULL,
                NULL,
                ELM_GENLIST_ITEM_NONE,
                _gl_sel,
                NULL );

    evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

    Elm_Object_Item *navi_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_HEADER_INSTALL_CERTIFICATE_ABB", NULL, NULL, genlist, NULL);
    if (!navi_it){
        LOGE("Error in elm_naviframe_item_push");
    }

    elm_object_item_domain_text_translatable_set(navi_it, PACKAGE, EINA_TRUE);

    //Title Text Left Button
    btn = _create_title_text_btn(ad->navi_bar, "IDS_ST_BUTTON_INSTALL", _install_button_cb, NULL);
    elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

    //Title Text Right Button
    btn = _create_title_text_btn(ad->navi_bar, "IDS_ST_BUTTON_CANCEL", _cancel_button_cb, NULL);
    elm_object_item_part_content_set(navi_it, "title_left_btn", btn);
}
