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
 * @file        pfx_cert.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 *              Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief
 */

#include <efl_extension.h>
#include <Elementary.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "common-utils.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static CertSvcStringList stringList;
static char **alias_list;
static char **email_list;
static int max_length;
static Elm_Genlist_Item_Class itc_2text;

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part);
static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);

void clear_pfx_genlist_data(void)
{
	LOGD("clear_pfx_genlist_data()");
	int i;
	if (max_length < 1) {
		alias_list = NULL;
		email_list = NULL;
		return;
	}

	if (alias_list) {
		for (i = 0; i < max_length; ++i)
			free (alias_list[i]);

		free(alias_list);
		alias_list = NULL;
	}

	if (email_list) {
		for (i = 0; i < max_length; ++i)
			free (email_list[i]);

		free(email_list);
		email_list = NULL;
	}

	max_length = 0;

	certsvc_string_list_free(stringList);
	memset(&stringList, 0, sizeof(stringList));
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	item_data_s *id = (item_data_s *)data;
	CertSvcInstance instance;
	CertSvcString alias;

	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("Failed to certsvc_instance_new().");
		return;
	}

	if (certsvc_string_new(
			instance,
			id->gname,
			strlen(id->gname),
			&alias) != CERTSVC_SUCCESS) {
		LOGE("certsvc_string_new failed");
		certsvc_instance_free(instance);
		return;
	}

	if (certsvc_pkcs12_set_certificate_status_to_store(
			instance,
			id->storeType,
			DISABLED,
			alias,
			(elm_check_state_get(obj) ? ENABLED : DISABLED)) != CERTSVC_SUCCESS) {
		LOGE("Failed to enable/disable status.");
		certsvc_instance_free(instance);
		return;
	}

	certsvc_instance_free(instance);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;
	Evas_Object *content = NULL;
	item_data_s *id = (item_data_s *)data;
	Eina_Bool status = certStatusToEina(id->status);

	if (!strcmp(part, "elm.icon.2")) {
		content = elm_layout_add(obj);
		elm_layout_theme_set(content, "layout", "list/C/type.3", "default");
		check = elm_check_add(content);
		elm_object_style_set(check, "on&off");
		elm_check_state_set(check, status);
		evas_object_repeat_events_set(check, EINA_FALSE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_smart_callback_add(check, "changed", _chk_changed_cb, data);
		elm_layout_content_set(content, "elm.swallow.content", check);

		return content;
	}

	return NULL;
}

static void _cert_selection_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	struct ug_data *ad = get_ug_data();
	if (ad->popup)
		evas_object_del(ad->popup);
	ad->popup = NULL;

	struct ListElement *current = (struct ListElement *)data;

	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_selected_item_get(obj);
	if (!it) {
		LOGE("Item object is null.");
		return;
	}

	elm_genlist_item_selected_set(it, EINA_FALSE);
	ad->data = (void *)strdup(current->gname);

	get_info_cert_from_file_cb(ad, data);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = data;

	if (!strcmp(part, "elm.text.main.left"))
		return strdup(id->title);

	return  NULL;
}

static Eina_Bool
pfx_cert_naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	struct ug_data *ad = get_ug_data();

	evas_object_del(ad->more_popup2);
	ad->more_popup2 = NULL;

	clear_pfx_genlist_data();

	return EINA_TRUE;
}

void more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("more_button_cb");
	struct ug_data *ad = (struct ug_data *)data;
	Evas_Object *popup = NULL;

	if (ad->more_popup2 != NULL)
		return;

	popup = common_more_ctxpopup(obj);

	elm_ctxpopup_item_append(popup, dgettext(PACKAGE, "IDS_ST_BUTTON_INSTALL"), NULL, install_button_cb, ad);
	elm_ctxpopup_item_append(popup, dgettext(PACKAGE, "IDS_ST_BUTTON_UNINSTALL"), NULL, pfx_cert_remove_cb, ad);

	evas_object_show(popup);

	ad->more_popup2 = popup;
}

void create_genlist_cb(void *data, CertStoreType storeType)
{
	Evas_Object *parent = (Evas_Object *)data;
	Evas_Object *genlist_pfx = NULL;
	Evas_Object *no_content = NULL;
	struct ListElement *current = NULL;
	struct ListElement *lastListElement = NULL;
	struct ug_data *ad = get_ug_data();

	size_t Length = 0;
	size_t i = 0;
	int result = 0;

	CertSvcStoreCertList *certList = NULL;
	CertSvcStoreCertList *certListHead = NULL;

	CertSvcInstance instance;

	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("Failed to certsvc_instance_new().");
		return;
	}

	result = certsvc_pkcs12_get_certificate_list_from_store(instance, storeType, DISABLED, &certList, &Length);
	if (result != CERTSVC_SUCCESS) {
		certsvc_instance_free(instance);
		LOGE("Fail to get the certificate list from store.");
		return;
	}

	certListHead = certList;

	genlist_pfx = common_genlist(parent);
	if (!genlist_pfx) {
		certsvc_instance_free(instance);
		return;
	}

	if (Length) {
		for (i = 0; i < Length; i++) {
			item_data_s *id = item_data_create(
				certList->gname,
				certList->title,
				certList->status,
				storeType,
				i);

			if (!id) {
				certsvc_instance_free(instance);
				LOGE("fail to allocate memory");
				return;
			}

			current = addListElement(
				lastListElement,
				NULL,
				certList->gname,
				NULL,
				NULL);

			if (!current) {
				item_data_free(id);
				certsvc_instance_free(instance);
				LOGE("Failed to allocate memory");
				return;
			}

			lastListElement = current;
			current->storeType = storeType;
			certList = certList->next;

			itc_2text.item_style = "1line";
			itc_2text.func.text_get = _gl_text_get;
			itc_2text.func.content_get = _gl_content_get;
			itc_2text.func.del = NULL;

			elm_genlist_item_append(genlist_pfx, &itc_2text, id, NULL, ELM_GENLIST_ITEM_NONE, _cert_selection_cb, current);
		}

		result = certsvc_pkcs12_free_certificate_list_loaded_from_store(instance, &certListHead);
		if (result != CERTSVC_SUCCESS)
			LOGE("Fail to free certificate list.");

		/* delete the previous object set */
		if (ad->user_cert_list_item)
			elm_object_item_part_content_set(ad->user_cert_list_item, NULL, genlist_pfx);
		else
			ad->user_cert_list_item = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_USER_CERTIFICATES", common_back_btn(ad), NULL, genlist_pfx, NULL);

	} else {
		no_content = create_no_content_layout(ad);
		if (!no_content) {
			LOGE("Cannot create no_content layout");
			certsvc_instance_free(instance);
			return;
		}

		/* delete the previous object set */
		if (ad->user_cert_list_item)
			elm_object_item_part_content_set(ad->user_cert_list_item, NULL, no_content);
		else
			ad->user_cert_list_item = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_USER_CERTIFICATES", common_back_btn(ad), NULL, no_content, NULL);
	}

	certsvc_instance_free(instance);
}

void create_VPN_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("Create list about VPN.");
	create_genlist_cb(data, VPN_STORE);
}

void create_WIFI_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("Create list about WIFI.");
	create_genlist_cb(data, WIFI_STORE);
}

void create_EMAIL_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("Create list about EMAIL.");
	create_genlist_cb(data, EMAIL_STORE);
}

Evas_Object *create_2_text_with_title_tabbar(Evas_Object *parent)
{
	Evas_Object *toolbar = elm_toolbar_add(parent);

	elm_object_style_set(toolbar, "tabbar_with_title");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);

	elm_toolbar_item_append(toolbar, NULL, "VPN", create_VPN_list_cb, parent);
	elm_toolbar_item_append(toolbar, NULL, "WIFI", create_WIFI_list_cb, parent);
	elm_toolbar_item_append(toolbar, NULL, "EMAIL", create_EMAIL_list_cb, parent);

	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);

	return toolbar;
}

void pfx_cert_create_list(struct ug_data *ad)
{
	LOGD("Create list about pfx_cert.");
	Evas_Object *no_content = NULL;
	Evas_Object *tabbar;

	clear_pfx_genlist_data();

	if (!ad)
		return;

	no_content = create_no_content_layout(ad);
	if (!no_content) {
		LOGE("Cannot create no_content layout");
		return;
	}

	if (ad->user_cert_list_item)
		elm_object_item_part_content_set(ad->user_cert_list_item, NULL, no_content);
	else
		ad->user_cert_list_item = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_USER_CERTIFICATES", common_back_btn(ad), NULL, no_content, NULL);

	elm_naviframe_item_style_set(ad->user_cert_list_item, "tabbar");
	tabbar = create_2_text_with_title_tabbar(ad->win_main);
	elm_object_item_part_content_set(ad->user_cert_list_item, "tabbar", tabbar);
	elm_object_item_part_text_set(ad->user_cert_list_item, NULL, dgettext(PACKAGE, "IDS_ST_BODY_USER_CERTIFICATES"));
}

void refresh_pfx_cert_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();

	evas_object_del(ad->more_popup2);
	ad->more_popup2 = NULL;

	pfx_cert_create_list(ad);
}

void pfx_cert_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();

	ad->user_cert_list_item = NULL;
	pfx_cert_create_list(ad);

	if (ad->more_popup2 != NULL) {
		evas_object_del(ad->more_popup2);
		ad->more_popup2 = NULL;
	}

	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_MORE, more_button_cb, ad);

	elm_naviframe_item_pop_cb_set(ad->user_cert_list_item, pfx_cert_naviframe_pop_cb, ad);

	ad->refresh_screen_cb = refresh_pfx_cert_cb;
}
