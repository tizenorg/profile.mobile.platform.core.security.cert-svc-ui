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

#include <efl_extension.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "common-utils.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static Eina_Bool *state_pointer;
static Evas_Object *genlist;
static int checked_count;
static Elm_Object_Item *select_all_btn_item;
Evas_Object *done;
static CertSvcStoreCertList *CertList;

static void _pfx_cert_remove_cleanup(void)
{
	if (state_pointer) {
		free(state_pointer);
		state_pointer = NULL;
	}

}

static void _navi_text_update(void)
{
	const char *pText = dgettext(PACKAGE, "IDS_ST_HEADER_PD_SELECTED_ABB");
	size_t size = 0;
	char *formatedText = NULL;

	if (!pText)
		return;

	/* 30 for itoa value of checked_count, should work for 64 bit int */
	size = strlen(pText) + 30;
	formatedText = (char *)malloc(sizeof(char) * size);
	if (!formatedText) {
		LOGE("Failed to allocate memory");
		return;
	}

	/* pText must contain %d */
	snprintf(formatedText, size, pText, checked_count);
	elm_object_item_part_text_set(select_all_btn_item, NULL, formatedText);
	free(formatedText);
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	if (elm_check_state_get(obj))
		checked_count++;
	else
		checked_count--;

	if (checked_count)
		elm_object_disabled_set(done, EINA_FALSE);
	else
		elm_object_disabled_set(done, EINA_TRUE);

	LOGD("check changed, count: %d / %d\n", checked_count, elm_genlist_items_count(genlist));

	_navi_text_update();
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	Evas_Object *ck;
	Elm_Object_Item *item = ei;

	elm_genlist_item_selected_set(item, EINA_FALSE);
	ck = elm_object_item_part_content_get(ei, "elm.icon.right");

	if (!ck)
		return;

	elm_check_state_set(ck, !elm_check_state_get(ck));
	_chk_changed_cb(data, ck, NULL);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = (item_data_s *)data;
	int index = (intptr_t)id->index;
	Evas_Object *check;

	if (strcmp(part, "elm.icon.right"))
		return NULL;

	check = elm_check_add(obj);
	elm_object_style_set(check, "default/genlist");

	/* set the State pointer to keep the current UI state of Checkbox */
	elm_check_state_pointer_set(check, &(state_pointer[index]));

	/*
	 * Repeat events to below object (genlist)
	 * So that if check is clicked, genlist can be clicked.
	 */
	evas_object_repeat_events_set(check, EINA_FALSE);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(check, "changed", _chk_changed_cb, (void *)(intptr_t)index);

	return check;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = (item_data_s *)data;

	if (!strcmp(part, "elm.text.main.left.top"))
		return strdup(id->title);

	if (strcmp(part, "elm.text.sub.left.bottom"))
		return NULL;

	if (id->storeType == VPN_STORE)
		return strdup("Store Type: VPN");

	if (id->storeType == WIFI_STORE)
		return strdup("Store Type: WIFI");

	return strdup("Store Type: EMAIL");
}

static void genlist_pfx_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	int i = 0;
	int ret = -1;
	struct ug_data *ad = (struct ug_data *)data;

	if (ad == NULL || state_pointer == NULL)
		return;

	ad->more_popup2 = NULL;
	evas_object_del(ad->popup);
	ad->popup = NULL;

	CertSvcString FileName;
	CertSvcInstance instance;
	CertSvcStoreCertList *certListHead = CertList;

	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("Failed to certsvc_instance_new().");
		return;
	}

	while (CertList) {
		if (EINA_TRUE == state_pointer[i]) {
			ret = certsvc_string_new(instance, CertList->gname, strlen(CertList->gname), &FileName);
			if (ret != CERTSVC_SUCCESS) {
				LOGE("Failed to certsvc_string_new.");
				certsvc_instance_free(instance);
				return;
			}

			ret = certsvc_pkcs12_delete_certificate_from_store(instance, CertList->storeType, FileName);
			if (ret != CERTSVC_SUCCESS) {
				LOGE("Fail to delete selected certificate");
				certsvc_instance_free(instance);
				return;
			}
		}
		i++;
		CertList = CertList->next;
	}

	ret = certsvc_pkcs12_free_certificate_list_loaded_from_store(instance, &certListHead);
	if (ret != CERTSVC_SUCCESS)
		LOGE("Fail to free certificate list");

	_pfx_cert_remove_cleanup();
	elm_naviframe_item_pop(ad->navi_bar);

	if (ad && ad->refresh_screen_cb)
		ad->refresh_screen_cb(ad, NULL, NULL);

	certsvc_instance_free(instance);
	LOGD("genlist_pfx_delete_cb done");
}

static void _popup_quit_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;

	if (ad == NULL)
		return;

	evas_object_del(ad->popup);
	ad->popup = NULL;
}

static void genlist_pfx_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *pText;
	char *formatedText;
	size_t size;
	struct ug_data *ad = (struct ug_data *)data;
	Evas_Object *btn_no;
	Evas_Object *btn_yes;

	if (ad == NULL)
		return;

	ad->popup = elm_popup_add(ad->navi_bar);
	if (!ad->popup)
		return;

	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn_no = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_no, PACKAGE, "IDS_ST_BUTTON_CANCEL");
	evas_object_smart_callback_add(btn_no, "clicked", _popup_quit_cb, ad);
	elm_object_style_set(btn_no, "popup");

	btn_yes = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_yes, PACKAGE, "IDS_ST_BUTTON_UNINSTALL");
	evas_object_smart_callback_add(btn_yes, "clicked", genlist_pfx_delete_cb, ad);
	elm_object_style_set(btn_yes, "popup");
	eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _popup_quit_cb, ad);

	if (checked_count > 1) {
		pText = dgettext(PACKAGE, "IDS_ST_POP_PD_CERTIFICATES_WILL_BE_UNINSTALLED");
		/* 30 for itoa value of checked_count, should work for 64 bit int */
		size = strlen(pText) + 30;
		formatedText = malloc(size);
		if (formatedText) {
			/* pText must contain %d */
			snprintf(formatedText, size, pText, checked_count);
			elm_object_text_set(ad->popup, formatedText);
			free(formatedText);
		}
	} else {
		pText = dgettext(PACKAGE, "IDS_ST_POP_1_CERTIFICATE_WILL_BE_UNINSTALLED");
		elm_object_text_set(ad->popup, pText);
	}

	elm_object_part_content_set(ad->popup, "button1", btn_no);
	elm_object_part_content_set(ad->popup, "button2", btn_yes);
	elm_object_domain_translatable_part_text_set(ad->popup, "title,text", PACKAGE, "IDS_ST_HEADER_UNINSTALL_CERTIFICATES_ABB");
	evas_object_show(ad->popup);
}

static void genlist_pfx_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;

	if (!ad)
		return;

	ad->more_popup2 = NULL;
	_pfx_cert_remove_cleanup();
	elm_naviframe_item_pop(ad->navi_bar);
}

static Eina_Bool genlist_pfx_back_cb(void *data, Elm_Object_Item *it)
{
	struct ug_data *ad = (struct ug_data *) data;

	if (ad) {
		ad->more_popup2 = NULL;
		_pfx_cert_remove_cleanup();
	}

	return EINA_TRUE;
}

void cert_remove_genlist_cb(void *data, CertStoreType storeType)
{
	size_t i = 0;
	size_t Length = 0;
	int result = 0;
	CertSvcStoreCertList *certList = NULL;
	CertSvcInstance instance;
	Elm_Genlist_Item_Class *itc;

	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("Failed to certsvc_instance_new().");
		return;
	}

	result = certsvc_pkcs12_get_certificate_list_from_store(instance, storeType, DISABLED, &certList, &Length);
	if (result != CERTSVC_SUCCESS) {
		LOGE("Fail to get the certificate list from store.");
		certsvc_instance_free(instance);
		return;
	}
	CertList = certList;

	if (genlist)
		elm_genlist_clear(genlist);

	if (state_pointer) {
		free(state_pointer);
		state_pointer = NULL;
	}

	itc = elm_genlist_item_class_new();
	if (!itc) {
		LOGE("Fail to malloc genlist item class");
		certsvc_instance_free(instance);
		return;
	}

	itc->item_style = "2line.top";
	itc->func.content_get = _gl_content_get;
	itc->func.text_get = _gl_text_get;

	if (Length) {
		state_pointer = malloc((Length + 1) * sizeof(Eina_Bool));
		for (i = 0; i < Length; i++) {
			state_pointer[i] = EINA_FALSE;
			item_data_s *id = item_data_create(
					certList->gname,
					certList->title,
					certList->status,
					storeType,
					i);

			if (!id) {
				certsvc_instance_free(instance);
				elm_genlist_item_class_free(itc);
				LOGE("fail to allocate memory");
				return;
			}

			certList = certList->next;
			elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, (void *)i);
		}

	}

	certsvc_instance_free(instance);
	elm_genlist_item_class_free(itc);
}

void VPN_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	cert_remove_genlist_cb(data, VPN_STORE);
}

void WIFI_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	cert_remove_genlist_cb(data, WIFI_STORE);
}

void EMAIL_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	cert_remove_genlist_cb(data, EMAIL_STORE);
}

static void _create_genlist(struct ug_data *ad)
{
	Evas_Object *tabbar;

	genlist = common_genlist(ad->navi_bar);

	Elm_Object_Item *itm = elm_naviframe_item_push(
			ad->navi_bar,
			"IDS_ST_HEADER_SELECT_ITEMS",
			common_back_btn(ad),
			NULL,
			genlist,
			NULL);
	select_all_btn_item = itm;

	elm_object_item_domain_text_translatable_set(itm, PACKAGE, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(itm, genlist_pfx_back_cb, ad);
	elm_naviframe_item_style_set(itm, "tabbar");
	tabbar = create_2_text_with_title_tabbar(ad->win_main);
	elm_object_item_part_content_set(itm, "tabbar", tabbar);

}

void pfx_cert_remove_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *) data;

	common_dismissed_cb(ad->more_popup2, obj, event_info);

	checked_count = 0;

	_create_genlist(ad);

	done = add_common_done_btn(ad, genlist_pfx_popup_cb);
	add_common_cancel_btn(ad, genlist_pfx_cancel_cb);
	elm_object_disabled_set(done, EINA_TRUE);
}
