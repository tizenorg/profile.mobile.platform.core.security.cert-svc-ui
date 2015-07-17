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
#include <efl_extension.h>
#include <dlog.h>

#include <stdio.h>
#include <dirent.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "cert-selection-uigadget.h"

#define CERT_MAX_DATA_SIZE  256
#define CERT_SVC_UI_RES_PATH "/usr/ug/res/images/cert-svc-ui"

static CertSvcInstance   instance;
static CertSvcStringList stringList;

static int             state_index    = -1; //selected radio index
static char            *selected_name = NULL;
static Eina_Bool       selected       = EINA_FALSE;
static Evas_Object     *radio_main    = NULL;
static Elm_Object_Item *open_button   = NULL;
static Elm_Genlist_Item_Class itc;

typedef struct item_data
{
	int status;
	char *gname;
	int storeType;
	int index;
	char *title;
} item_data_s;

/* ***************************************************
 **
 **general func
 **
 ****************************************************/
static void create_selection_list(struct ug_data *ad);
static void _move_more_ctxpopup(void *data);

static void _cert_selection_cleanup()
{
	LOGD("_cert_selection_cleanup");
	if (selected_name)
	{
		free(selected_name);
		selected_name = NULL;
	}

	certsvc_string_list_free(stringList);
	memset(&stringList, 0, sizeof(stringList));
	certsvc_instance_free(instance);
}

static Eina_Bool _quit_cb(void *data, Elm_Object_Item *it)
{
    LOGD("_quit_cb");
    struct ug_data *ad = (struct ug_data*) data;

    if (ad->ug) {
          _cert_selection_cleanup();
         ug_destroy_me(ad->ug);
         ad->ug = NULL;
    }

    return EINA_FALSE;
}

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("_dismissed_cb");
	struct ug_data *ad = (struct ug_data*) data;
	if(ad->ctx_popup)
	{
		Evas_Object *win = elm_object_top_widget_get(ad->win_main);
		evas_object_smart_callback_del(win, "rotation,changed", _move_more_ctxpopup);
		evas_object_smart_callback_del(ad->ctx_popup, "dismissed", _dismissed_cb);
		evas_object_del(ad->ctx_popup);
		ad->ctx_popup = NULL;
	}
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

	evas_object_del(ad->popup);
	ad->popup = NULL;

	cbs->layout_cb = __cert_layout_ug_cb;
	cbs->result_cb = NULL;
	cbs->destroy_cb = __destroy_certificates_ug_cb;
	cbs->priv = (void *)ad;

	app_control_h service;
	app_control_create(&service);
	app_control_add_extra_data(service, "selected-cert", "send-cert");
	ug_create(ad->ug, "setting-manage-certificates-efl", UG_MODE_FULLVIEW, service, cbs);
	app_control_destroy(service);
	free(cbs);
}

static void _popup_quit_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data*) data;

	evas_object_del(ad->popup);
	ad->popup = NULL;
}

static void create_yes_no_pop_cb(void *data, Evas_Object *obj, void *event_info) {


	if (NULL == data){
		return;
	}
	struct ug_data *ad = (struct ug_data *) data;

	_dismissed_cb(data, obj, event_info);

	const char *contentId = "IDS_ST_POP_CERTIFICATES_FROM_THE_DEVICE_MEMORY_AND_SD_CARD_WILL_BE_INSTALLED";


	if (ad->popup){
		// this maybe not good solution(better to use evas_object_show elm without deleting)
		LOGD("pop up is not null");
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	ad->popup = elm_popup_add(ad->navi_bar);
	if (NULL == ad->popup){
		LOGD("pop up is null");
		return;
	}

	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	//no button
	Evas_Object *btn_no = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_no, PACKAGE, "IDS_ST_BUTTON_CANCEL");
	elm_object_style_set(btn_no, "popup");

	//yes button
	Evas_Object *btn_yes = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_yes, PACKAGE, "IDS_ST_BUTTON_INSTALL");
	evas_object_smart_callback_add(btn_yes, "clicked", _cert_install_cb, ad);
	elm_object_style_set(btn_yes, "popup");

	evas_object_smart_callback_add(btn_no, "clicked", _popup_quit_cb, ad);
	eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _popup_quit_cb, ad);

	elm_object_domain_translatable_text_set(ad->popup, PACKAGE, contentId);
	//evas_object_smart_callback_add(ad->popup, "language,changed", _popup_lang_changed, contentId);

	elm_object_part_content_set(ad->popup, "button1", btn_no);
	elm_object_part_content_set(ad->popup, "button2", btn_yes);
	elm_object_domain_translatable_part_text_set(ad->popup, "title,text", PACKAGE, "IDS_ST_HEADER_INSTALL_CERTIFICATE_ABB2");
	evas_object_show(ad->popup);
	LOGD("pop up end");
}

static void _open(void *data, Evas_Object *obj, void *event_info) {

    LOGD("selected index = %d", state_index);
    (void)obj;
    (void)event_info;

    struct ug_data *ad = (struct ug_data*) data;

    if (selected) {
        app_control_h service;
        app_control_create(&service);
        app_control_add_extra_data(service, "selected-cert", selected_name);
        ug_send_result(ad->ug, service);
        app_control_destroy(service);
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
	item_data_s *id = (item_data_s *) data;

	LOGD("title:%s",id->title);
	if (!strcmp(part, "elm.text.main.left"))  {
		return strdup(id->title);
	}
	return  NULL;
}

static void _update_list_cb(void *data, Evas_Object *obj, void *event_info)
{

	item_data_s *id = (item_data_s *) data;

    int pkcs_index = id->index;
    LOGD("pkcs_index = %d", pkcs_index);
    state_index = pkcs_index;

    selected = EINA_TRUE;

    elm_object_disabled_set(open_button, EINA_FALSE);

    if(selected_name)
        free(selected_name);
    LOGD("selected_name gname is %s",id->gname);
    selected_name = malloc((strlen(id->gname)+1) * sizeof(char));
    strncpy(selected_name, id->gname, strlen(id->gname)+1);

    SECURE_LOGD("SELECTED NAME = %s", selected_name);

}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part) {

	item_data_s *id = (item_data_s *) data;
	LOGD("title:%s",id->title);

	Evas_Object *radio;

    if (!strcmp(part, "elm.icon.right")) {
        radio = elm_radio_add(obj);
        elm_radio_state_value_set(radio, id->index);
        elm_radio_group_add(radio, radio_main);
        elm_radio_value_pointer_set(radio, &state_index);
        evas_object_repeat_events_set(radio, EINA_FALSE);
        evas_object_propagate_events_set(radio, EINA_FALSE);
        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(radio, "changed", _update_list_cb, id);
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

    Elm_Object_Item *item = (Elm_Object_Item *) event_info;
    elm_genlist_item_selected_set(item, EINA_FALSE);
    elm_genlist_item_update(item);

   _update_list_cb(data, obj, event_info);
}

static Evas_Object *create_no_content_layout (struct ug_data *ad) {

    Evas_Object *no_content = elm_layout_add(ad->win_main);
	char buf[256] = {0,};
    if(NULL == no_content){
        return NULL;
    }

    elm_layout_theme_set(no_content, "layout", "nocontents", "default");
    evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object* icon = elm_image_add(no_content);

	snprintf(buf, sizeof(buf), "%s/00_nocontents_text_gray.png", CERT_SVC_UI_RES_PATH);

    elm_image_file_set(icon, buf, NULL);
    elm_object_part_content_set(no_content, "nocontents.image", icon);

    elm_object_domain_translatable_part_text_set(no_content, "elm.text", PACKAGE, "IDS_ST_BODY_NO_CONTENT");
    evas_object_show(no_content);
    return no_content;
}

static void create_selection_list(struct ug_data *ad)
{
    int i = 0;
    int Length = 0;
    Evas_Object *no_content = NULL;
    Evas_Object *genlist = NULL;

	int result = -1;
	CertSvcStoreCertList* certList = NULL;
	CertStoreType storeType = (CertStoreType) EMAIL_STORE;

	if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
		LOGE("CERTSVC_FAIL to create instance.");
		return;
   	}

	result = certsvc_pkcs12_get_end_user_certificate_list_from_store(instance, storeType, &certList, &Length);
	if (result!=CERTSVC_SUCCESS) {
		LOGE("Fail to get the certificate list from store.");
		return;
	}

	LOGD("create_selection_list: Number Of Certs [%d]", Length);

	radio_main = NULL;

    // Refresh logic: 2. Make new layouts on Main screen
	if(1 > Length){
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
		elm_list_mode_set(genlist, ELM_LIST_COMPRESS);
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
		evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

		radio_main = elm_radio_add(genlist);
		elm_radio_state_value_set(radio_main, 0);
		elm_radio_value_set(radio_main, 0);

		for (i = 0; i < Length; i++) {

			item_data_s *id = calloc(sizeof(item_data_s), 1);
            if(id == NULL) {
                LOGE("fail to allocate memory");
                break;
            }
			id->title = (char *)malloc(strlen(certList->title)+1);
			id->gname = (char *)malloc(strlen(certList->gname)+1);
			id->storeType = certList->storeType;
            if(id->gname != NULL)
			    strncpy(id->gname,certList->gname,strlen(certList->title)+1);
            if(id->title != NULL)
			    strncpy(id->title,certList->title,strlen(certList->gname)+1);
			id->index = i;
			elm_genlist_item_append(genlist, &itc, id, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, id);
			certList = certList->next;
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
			elm_object_disabled_set(open_button, EINA_FALSE);
	    } else {
			elm_object_disabled_set(open_button, EINA_TRUE);
	    }
	}
}

static void _move_more_ctxpopup(void *data)
{
	int y=0, w=0,h=0,w1=0,h1=0;
	int rotate;
	struct ug_data *ad = (struct ug_data *) data;
	Evas_Object *elm_win = elm_object_top_widget_get(ad->win_main);
	evas_object_geometry_get(NULL, NULL, &y, &w, &h);
	elm_win_screen_size_get(elm_win, NULL, NULL, &w1, &h1);
	rotate = elm_win_rotation_get(elm_win);
	LOGD("rotate : %d, h1 : %d w1 : %d", rotate, h1, w1);

	if(rotate == -1)
		return;

	if(rotate == 90)
		evas_object_move(ad->ctx_popup, h1/2, w1);
	else if(rotate == 270)
		evas_object_move(ad->ctx_popup, h1/2, w1);
	else
		evas_object_move(ad->ctx_popup, w1/2, h1);

}

//A callback function that will be called when More event is triggered.
static void _cert_naviframe_more_cb(void *data, Evas_Object *obj, void *event_info) {
	struct ug_data *ad = (struct ug_data *) data;
	Evas_Object *more_popup = NULL;
	Elm_Object_Item* more_popup_item = NULL;
	Evas_Object *win;

	//Create a Ctxpopup if the ctxpopup is not on active.
	win = elm_object_top_widget_get(ad->win_main);
	more_popup = elm_ctxpopup_add(win);
	if (!more_popup)
		return;

	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

	elm_object_style_set(more_popup, "more/default");
	evas_object_smart_callback_add(more_popup, "dismissed", _dismissed_cb, ad);

	more_popup_item = elm_ctxpopup_item_append(more_popup, "IDS_ST_BUTTON_INSTALL", NULL, create_yes_no_pop_cb, ad);
	elm_object_item_domain_text_translatable_set(more_popup_item, PACKAGE, EINA_TRUE);

	ad->ctx_popup = more_popup;

	elm_ctxpopup_auto_hide_disabled_set(ad->ctx_popup, EINA_TRUE);

	evas_object_smart_callback_add(win, "rotation,changed", _move_more_ctxpopup, ad);

	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP,
											ELM_CTXPOPUP_DIRECTION_UNKNOWN,
											ELM_CTXPOPUP_DIRECTION_UNKNOWN,
											ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_move_more_ctxpopup(ad);

	evas_object_show(more_popup);
}

void cert_selection_install_cb(void *data, Evas_Object *obj, void *event_info)
{
    struct ug_data *ad = (struct ug_data *) data;

    state_index = -1;

    ad->user_cert_list_item = NULL;

    // Set genlist item class
    itc.item_style       = "1line";
    itc.func.text_get    = _gl_text_get;
    itc.func.content_get = _gl_content_get;
    itc.func.state_get   = _gl_state_get;
    itc.func.del         = _gl_del;

    if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
        LOGE("CERTSVC_FAIL");
        return;
    }

    create_selection_list(ad);

    if (!ad->user_cert_list_item)
    	return;

	open_button= elm_button_add(ad->navi_bar);
	if (!open_button) {
		LOGD("Error creating toolbar button");
		return;
	}

	elm_object_style_set(open_button, "bottom");
	elm_object_domain_translatable_text_set(open_button, PACKAGE, "IDS_ST_BUTTON_OPEN");
	evas_object_smart_callback_add(open_button, "clicked", _open, ad);
    elm_object_item_part_content_set(ad->user_cert_list_item, "toolbar", open_button);

    elm_naviframe_item_pop_cb_set(ad->user_cert_list_item, _quit_cb, data);

	elm_object_disabled_set(open_button, EINA_TRUE);
	evas_object_show(open_button);

    elm_naviframe_prev_btn_auto_pushed_set(ad->navi_bar, EINA_FALSE);

    eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_MORE, _cert_naviframe_more_cb, ad);
}
