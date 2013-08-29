/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        cert-selection.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <Elementary.h>
#include <efl_assist.h>
#include <dlog.h>

#include <stdio.h>
#include <dirent.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "cert-selection-uigadget.h"

#define CERT_MAX_DATA_SIZE  256

static CertSvcInstance   instance;
static CertSvcStringList stringList;

static int             state_index    = -1; //selected radio index
static char            *selected_name = NULL;
static Eina_Bool       selected       = EINA_FALSE;
static Evas_Object     *radio_main    = NULL;
static Elm_Object_Item *open_button   = NULL;
static Elm_Genlist_Item_Class itc;

/* ***************************************************
 **
 **general func
 **
 ****************************************************/
static void create_selection_list(struct ug_data *ad);

							static void _cert_selection_cleanup()
{
	if (selected_name)
	{
		free(selected_name);
		selected_name = NULL;
	}

	certsvc_string_list_free(stringList);

	certsvc_instance_free(instance);
}

static Eina_Bool _quit_cb(void *data, Elm_Object_Item *it)
{
    struct ug_data *ad = (struct ug_data*) data;

    _cert_selection_cleanup();

    if (ad->ug) {
         ug_destroy_me(ad->ug);
         ad->ug = NULL;
    }

    return EINA_TRUE;
}

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_del(obj,"dismissed", _dismissed_cb);
	evas_object_del(obj);
}

static void __cert_layout_ug_cb(ui_gadget_h ug, enum ug_mode mode,
				  void *priv)
{
	LOGD("__cert_layout_ug_cb");
	Evas_Object *base;

    if (!ug || !priv) {
        return;
    }
	base = (Evas_Object *) ug_get_layout(ug);
	if (!base) {
		return;
	}
	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

static void __destroy_certificates_ug_cb(ui_gadget_h ug, void *priv)
{
	/* restore the '<-' button on the navigate bar */

	if (ug) {
		ug_destroy(ug);
	}

	//refresh
	create_selection_list(priv);

}

static void _cert_install_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;
	if (!ad) {
		return;
	}

	struct ug_cbs *cbs = (struct ug_cbs *)calloc(1, sizeof(struct ug_cbs));
	if (!cbs) {
		return;
	}

	_dismissed_cb(data, obj, event_info);

	cbs->layout_cb = __cert_layout_ug_cb;
	cbs->result_cb = NULL;
	cbs->destroy_cb = __destroy_certificates_ug_cb;
	cbs->priv = (void *)ad;

	service_h service;
	service_create(&service);
	service_add_extra_data(service, "selected-cert", "send-cert");
	ug_create(ad->ug, "setting-manage-certificates-efl", UG_MODE_FULLVIEW, service, cbs);
	service_destroy(service);
	free(cbs);
}


static void _open(void *data, Evas_Object *obj, void *event_info) {

    LOGD("selected index = %d", state_index);
    (void)obj;
    (void)event_info;

    struct ug_data *ad = (struct ug_data*) data;

    if (selected) {
        service_h service;
        service_create(&service);
        service_add_extra_data(service, "selected-cert", selected_name);
        ug_send_result(ad->ug, service);
        service_destroy(service);
        LOGD("result send");
    }

    _quit_cb(data, NULL);
}

const char* get_email(CertSvcString alias) {
    LOGD("get_email()");

    const char *char_buffer;

    CertSvcCertificateList certificateList;
    CertSvcCertificate certificate;
    CertSvcString email_buffer;
    if (CERTSVC_SUCCESS != certsvc_pkcs12_load_certificate_list(
            instance,
            alias,
            &certificateList)) {
        return NULL;
    }
    if (CERTSVC_SUCCESS != certsvc_certificate_list_get_one(
            certificateList,
            0,
            &certificate)) {
        return NULL;
    }
    if (CERTSVC_SUCCESS != certsvc_certificate_get_string_field(
            certificate,
            CERTSVC_SUBJECT_EMAIL_ADDRESS,
            &email_buffer)) {
        return NULL;
    }
    certsvc_string_to_cstring(email_buffer, &char_buffer, NULL);
    return char_buffer;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

    (void)obj;
    int index = (int) data;
    char *char_buffer = NULL;
    CertSvcString buffer;

    if (certsvc_string_list_get_one(stringList, index, &buffer) != CERTSVC_SUCCESS) {
        return strdup("ERROR WHILE LOADING STRING");
    }

    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        char_buffer = strndup(buffer.privateHandler, buffer.privateLength);
    } else if (!strcmp(part, "elm.text.2")) {
    	char_buffer = (char*) get_email(buffer);
    	if (char_buffer) {
    		char_buffer = strdup(char_buffer);
    	}
    }

    certsvc_string_free(buffer);
    return char_buffer;

}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part) {

    LOGD("_gl_content_get");

    int index = (int) data;
    Evas_Object *radio;
    LOGD("index = %d", index);

    if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
        radio = elm_radio_add(obj);
        elm_radio_state_value_set(radio, index);
        elm_radio_group_add(radio, radio_main);
        elm_radio_value_pointer_set(radio, &state_index);
        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

        return radio;
    }
    return NULL;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part) {

    return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj) {

   return;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info) {

    LOGD("_gl_sel");
    (void)obj;

    if(NULL == event_info){
        return;
    }

    int pkcs_index = (int) data;
    LOGD("pkcs_index = %d", pkcs_index);

    Elm_Object_Item *item = (Elm_Object_Item *) event_info;
    elm_genlist_item_selected_set(item, EINA_FALSE);

    state_index = pkcs_index;
    elm_genlist_item_update(item);

    selected = EINA_TRUE;

    CertSvcString buffer;
    if (certsvc_string_list_get_one(stringList, pkcs_index, &buffer) != CERTSVC_SUCCESS) {
        selected = EINA_FALSE;
        return;
    }
    elm_object_item_disabled_set(open_button, EINA_FALSE);

    if(selected_name)
        free(selected_name);
    selected_name = malloc((buffer.privateLength+1) * sizeof(char));
    strncpy(selected_name, buffer.privateHandler, buffer.privateLength);
    selected_name[buffer.privateLength] = 0;
    SECURE_LOGD("SELECTED NAME = %s", selected_name);
    certsvc_string_free(buffer);
}

static Evas_Object *create_no_content_layout (struct ug_data *ad) {

    Evas_Object *no_content = elm_layout_add(ad->win_main);
    if(NULL == no_content){
        return NULL;
    }
    elm_layout_theme_set(no_content, "layout", "nocontents", "text");
    elm_object_part_text_set(no_content, "elm.text", dgettext(PACKAGE, "IDS_ST_BODY_NO_CONTENT"));
    evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(no_content);
    return no_content;
}

static void create_selection_list(struct ug_data *ad)
{
    int i = 0;
    int list_length = 0;
    Evas_Object *no_content = NULL;
    Evas_Object *genlist = NULL;

	certsvc_pkcs12_get_id_list(instance, &stringList);

	certsvc_string_list_get_length(stringList, &list_length);

	LOGD("create_selection_list: Number Of Certs [%d]", list_length);

	radio_main = NULL;

    // Refresh logic: 2. Make new layouts on Main screen
	if(1 > list_length){
		// No Content message when list is empty
		no_content = create_no_content_layout(ad);
		if(!no_content) {
			LOGD("create_selection_list: Failed to create NO Layout");
			return;
		}

		if (ad->user_cert_list_item) {
			elm_object_item_part_content_set(ad->user_cert_list_item, NULL, no_content);
		}
		else {
			ad->user_cert_list_item = elm_naviframe_item_push(
						ad->navi_bar,
						"IDS_EMAIL_BUTTON_CLIENT_CERTIFICATE",
						NULL,
						NULL,
						no_content,
						NULL);

		}
	}
	else {
	    // Create genlist;
		genlist = elm_genlist_add(ad->win_main);

		evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

		radio_main = elm_radio_add(genlist);
		elm_radio_state_value_set(radio_main, 0);
		elm_radio_value_set(radio_main, 0);

	    for (i = 0; i < list_length; i++) {
			elm_genlist_item_append(genlist, &itc, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, (void*) i);
		}

		if (ad->user_cert_list_item) {
			elm_object_item_part_content_set(ad->user_cert_list_item, NULL, genlist);
		}
		else {
			ad->user_cert_list_item = elm_naviframe_item_push(
				ad->navi_bar,
				"IDS_EMAIL_BUTTON_CLIENT_CERTIFICATE",
				NULL,
				NULL,
				genlist,
				NULL);
		}
	}

	if (ad->user_cert_list_item != NULL) {
		elm_object_item_domain_text_translatable_set(ad->user_cert_list_item, PACKAGE, EINA_TRUE);
	}

	if (open_button) {
		if(state_index != -1) {
	    	elm_object_item_disabled_set(open_button, EINA_FALSE);
	    } else {
	    	elm_object_item_disabled_set(open_button, EINA_TRUE);
	    }
	}

}

static void _move_more_ctxpopup(void *data, Evas_Object *ctx)
{
	Evas_Coord w = 0, h = 0;
	int pos = -1;
	struct ug_data *ad = (struct ug_data *) data;
	Evas_Object *elm_win = ad->win_main; //NOTE: if UG is created using ug_get_parent_layout
										 //then please use elm_main =	evas_object_top_get(evas_object_evas_get(ad->win_main));

	elm_win_screen_size_get(elm_win, NULL, NULL, &w, &h);

	pos = elm_win_rotation_get(elm_win);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctx, 0, h);
			break;
		case 90:
			evas_object_move(ctx, 0, w);
			break;
		case 270:
			evas_object_move(ctx, h, w);
			break;
	}
}

//A callback function that will be called when More event is triggered.
static void _cert_naviframe_more_cb(void *data, Evas_Object *obj, void *event_info) {
	struct ug_data *ad = (struct ug_data *) data;
	Evas_Object *more_popup = NULL;
	Elm_Object_Item* more_popup_item = NULL;

	//Create a Ctxpopup if the ctxpopup is not on active.
	more_popup = elm_ctxpopup_add(ad->navi_bar);
	if (!more_popup) return;

	ea_object_event_callback_add(more_popup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	ea_object_event_callback_add(more_popup, EA_CALLBACK_MORE, ea_ctxpopup_back_cb, NULL);
	elm_object_style_set(more_popup, "more/default");
	evas_object_smart_callback_add(more_popup,"dismissed", _dismissed_cb, NULL);

	more_popup_item = elm_ctxpopup_item_append(more_popup, "IDS_ST_BUTTON_INSTALL", NULL, _cert_install_cb, ad);
	elm_object_item_domain_text_translatable_set(more_popup_item, PACKAGE, EINA_TRUE);

	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP,
											ELM_CTXPOPUP_DIRECTION_UNKNOWN,
											ELM_CTXPOPUP_DIRECTION_UNKNOWN,
											ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_move_more_ctxpopup(data, more_popup);


	evas_object_show(more_popup);
}

void cert_selection_install_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("cert_selection");

    struct ug_data *ad = (struct ug_data *) data;
    Evas_Object *toolbar = NULL;

    state_index = -1;

    ad->user_cert_list_item = NULL;

    toolbar = elm_toolbar_add(ad->navi_bar);
    if (!toolbar) return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
    elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
    elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

    open_button = elm_toolbar_item_append(toolbar, NULL, "IDS_ST_BUTTON_OPEN", _open, ad);
    if (!open_button) return;

    elm_object_item_domain_text_translatable_set(open_button, PACKAGE, EINA_TRUE);

    // Set genlist item class
    itc.item_style       = "2text.1icon.2";
    itc.func.text_get    = _gl_text_get;
    itc.func.content_get = _gl_content_get;
    itc.func.state_get   = _gl_state_get;
    itc.func.del         = _gl_del;

    if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
        LOGD("CERTSVC_FAIL");
        return;
    }

    create_selection_list(ad);

    if (!ad->user_cert_list_item) {
    	return;
    }

    elm_object_item_part_content_set(ad->user_cert_list_item, "toolbar", toolbar);
    elm_naviframe_item_pop_cb_set(ad->user_cert_list_item, _quit_cb, data);

    elm_naviframe_prev_btn_auto_pushed_set(ad->navi_bar, EINA_FALSE);

    ea_object_event_callback_add(ad->navi_bar, EA_CALLBACK_BACK, ea_naviframe_back_cb, NULL);
    ea_object_event_callback_add(ad->navi_bar, EA_CALLBACK_MORE, _cert_naviframe_more_cb, ad);

    LOGD("end of cert_selection");
}
