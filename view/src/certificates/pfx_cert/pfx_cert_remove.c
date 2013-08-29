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

#include <dlog.h>

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
static Eina_Bool *state_pointer = NULL; //check states
static int list_length = 0;
static Elm_Object_Item *uninstallButton = NULL;
static Evas_Object *select_all_layout = NULL;
static Evas_Object *genlist = NULL;
static int checked_count = 0;

static void _pfx_cert_remove_cleanup()
{
	free(state_pointer);
	state_pointer = NULL;
	certsvc_string_list_free(stringList);
}
static void uninstall_button_set () {

    int i;
    for (i = 1; i <= list_length; ++i) {
        if (EINA_TRUE == state_pointer[i]){
            elm_object_item_disabled_set(uninstallButton, EINA_FALSE);
            return;
        }
    }
    elm_object_item_disabled_set(uninstallButton, EINA_TRUE);
}


static void _select_all_chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	Eina_Bool state = elm_check_state_get(obj);

	if (state) {
		checked_count = elm_genlist_items_count(genlist) - 1;
	}
	else {
		checked_count = 0;
	}

	LOGD("check Select ALL changed, count: %d / %d\n", checked_count, elm_genlist_items_count(genlist));

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while(it) {
		int index = (int)elm_object_item_data_get(it);
		// For realized items, set state of real check object
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.icon");
		if (ck) elm_check_state_set(ck, state);
		// For all items (include unrealized), just set pointer state
		state_pointer[index] = state;
		it = elm_genlist_item_next_get(it);
	}

	uninstall_button_set();
}

static void _select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Evas_Object *check = elm_object_part_content_get(select_all_layout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	_select_all_chk_changed_cb(data, check, NULL);
}

static Evas_Object *
_create_select_all_layout(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, _select_all_layout_down_cb, NULL);
	evas_object_show(layout);

	Evas_Object *check = elm_check_add(layout);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_smart_callback_add(check, "changed", _select_all_chk_changed_cb, NULL);
	elm_object_part_content_set(layout, "elm.icon", check);
	elm_object_part_text_set(layout, "elm.text", dgettext(PACKAGE, "IDS_ST_BODY_SELECT_ALL"));
	return layout;
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	Eina_Bool state = elm_check_state_get(obj);
	if (state) {
		checked_count++;
	}
	else {
		checked_count--;
	}
	
	LOGD("check changed, count: %d / %d\n", checked_count, elm_genlist_items_count(genlist));

	Evas_Object *check = elm_object_part_content_get(select_all_layout, "elm.icon");
	if (elm_genlist_items_count(genlist) == checked_count + 1)	{
		elm_check_state_set(check, EINA_TRUE);
	} else {
		elm_check_state_set(check, EINA_FALSE);
	}

	uninstall_button_set();
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	Elm_Object_Item *item = ei;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	// Update check button
	Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.icon");
	Eina_Bool state = elm_check_state_get(ck);
	elm_check_state_set(ck, !state);

	_chk_changed_cb(data, ck, NULL);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part) {
    int index = (int) data;
    Evas_Object *check;

    if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
        check = elm_check_add(obj);
		elm_object_style_set(check, "default/genlist");
		//set the State pointer to keep the current UI state of Checkbox.
		elm_check_state_pointer_set(check, &(state_pointer[index]));
		// Repeat events to below object (genlist)
		// So that if check is clicked, genlist can be clicked.
		evas_object_repeat_events_set(check, EINA_TRUE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
        return check;
    }

    return NULL;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

static char* _gl_get_text_sa(void *data, Evas_Object *obj, const char *part) {

    return strdup(dgettext(PACKAGE, "IDS_ST_BODY_SELECT_ALL"));
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
        return strdup(get_email(buffer));
    }

    return NULL;
}


static void genlist_pfx_delete_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("genlist_pfx_remove_cb");
    (void)obj;
    (void)event_info;

    if(NULL == data){
       return;
    }
    struct ug_data *ad = (struct ug_data *) data;
    CertSvcString buffer;

    int i;
    if (EINA_TRUE == state_pointer[0]) {
        for (i = 0; i < list_length; i++) {
            if (certsvc_string_list_get_one(stringList, i, &buffer) == CERTSVC_SUCCESS) {
                if (certsvc_pkcs12_delete(ad->instance, buffer) == CERTSVC_SUCCESS)
                {
                    SECURE_LOGD("%s --- removed", buffer);
                }
				certsvc_string_free(buffer);
            }
        }
    } else {;
        for (i = 0; i < list_length; i++) {
            if (EINA_TRUE == state_pointer[i+1] &&
                    certsvc_string_list_get_one(stringList, i, &buffer) == CERTSVC_SUCCESS) {
                if (certsvc_pkcs12_delete(ad->instance, buffer) == CERTSVC_SUCCESS)
                {
                    SECURE_LOGD("%s --- removed", buffer);
                }
                certsvc_string_free(buffer);
            }
        }
    }
    _pfx_cert_remove_cleanup();

    elm_naviframe_item_pop(ad->navi_bar);
    if (ad && ad->refresh_screen_cb)
	{
		ad->refresh_screen_cb(ad, NULL, NULL);
	}

}

static void genlist_pfx_cancel_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("genlist_pfx_cancel_cb");
    (void)obj;
    (void)event_info;

    if(NULL == data){
        return;
    }
    struct ug_data *ad = (struct ug_data *) data;

    _pfx_cert_remove_cleanup();

    elm_naviframe_item_pop(ad->navi_bar);
}


static Evas_Object * _create_genlist(struct ug_data *ad, Evas_Object *parent)
{
	int index;
	Elm_Object_Item *git;

	//Being
	certsvc_pkcs12_get_id_list(ad->instance, &stringList);


	// Create genlist
	genlist = elm_genlist_add(parent);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *itc_sa = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();

	// Set genlist item class for "select all"
	itc_sa->item_style = "selectall_check";
	itc_sa->func.text_get = _gl_get_text_sa;
	itc_sa->func.content_get = _gl_content_get;

	// Set genlist item class
	itc->item_style = "1text.1icon.3";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;

	evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

	git = elm_genlist_item_append(genlist, itc_sa, (void *) 0, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_object_item_domain_text_translatable_set(git, PACKAGE, EINA_TRUE);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


	certsvc_string_list_get_length(stringList, &list_length);
	state_pointer = malloc((list_length+1) * sizeof(Eina_Bool));
	state_pointer[0] = EINA_FALSE;

	// Append items
	for (index = 1; index <= list_length; index++) {
		state_pointer[index] = EINA_FALSE;
		elm_genlist_item_append(
				genlist,			// genlist object
				itc,				// item class
				(void *) index,		// data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL
		);
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_sa);

	return genlist;
}

void pfx_cert_remove_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_remove_cb");

    struct ug_data *ad = (struct ug_data *) data;
    Evas_Object *genlist = NULL;
    Evas_Object *toolbar = NULL;
    Evas_Object *box = NULL;
    Elm_Object_Item *cancel_button;

    checked_count = 0;

    box = elm_box_add(ad->navi_bar);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	select_all_layout =_create_select_all_layout(box);
	evas_object_show(select_all_layout);
	elm_box_pack_end(box, select_all_layout);

	genlist = _create_genlist(ad, box);
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	evas_object_show(box);

	//toolbar
    toolbar = elm_toolbar_add(ad->navi_bar);
    if (!toolbar) return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
    elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
    elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

    uninstallButton = elm_toolbar_item_append(toolbar, NULL, "IDS_ST_BUTTON_UNINSTALL", genlist_pfx_delete_cb, ad);
    if (!uninstallButton) return;
    elm_object_item_domain_text_translatable_set(uninstallButton, PACKAGE, EINA_TRUE);

    cancel_button = elm_toolbar_item_append(toolbar, NULL, "IDS_ST_SK2_CANCEL", genlist_pfx_cancel_cb, ad);
    if (!cancel_button) return;
    elm_object_item_domain_text_translatable_set(cancel_button, PACKAGE, EINA_TRUE);

    elm_object_item_disabled_set(uninstallButton, EINA_TRUE);

    Elm_Object_Item *itm = elm_naviframe_item_push(
            ad->navi_bar,
            "IDS_ST_BUTTON_UNINSTALL",
            NULL,
            NULL,
            box,
            NULL);

    elm_object_item_domain_text_translatable_set(itm, PACKAGE, EINA_TRUE);
    elm_object_item_part_content_set(itm, "toolbar", toolbar);
    elm_object_item_part_content_unset(itm, "prev_btn");
}
