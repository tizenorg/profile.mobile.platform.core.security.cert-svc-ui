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
 * @file        certificate_util.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
                Sangwan Kwon (Sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief
 */
#include "mgr-app-uigadget.h"
#include "common-utils.h"
#include "certificates/certificates.h"

#include <tzplatform_config.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

#include <efl_extension.h>
#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"

static void _cert_selection_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_selected_item_get(obj);
	if (!it)
		return;

	elm_genlist_item_selected_set(it, EINA_FALSE);
	struct ug_data *ad = get_ug_data();
	ad->data = NULL;

	get_info_cert_from_file_cb(ad, data);
}

static void _info_pop_response_no_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;
	if (ad->popup)
		evas_object_del(ad->popup);

	ad->popup = NULL;
	ad->more_popup2 = NULL;
}

static void _info_pop_response_yes_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;
	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	pfx_cert_install(ad);
}

static void _info_pop_response_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();
	if (ad->popup)
		evas_object_del(ad->popup);

	ad->popup = NULL;
}

static void _popup_quit_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;
	if (ad->popup)
		evas_object_del(ad->popup);

	ad->popup = NULL;
	ad->more_popup2 = NULL;
	if (ad->ug) {
		ug_destroy_me(ad->ug);
		ad->ug = NULL;
	}
}

item_data_s *item_data_create(
	const char *gname,
	const char *title,
	CertStatus status,
	CertStoreType storeType,
	size_t index)
{
	item_data_s *id = malloc(sizeof(item_data_s));
	if (!id) {
		LOGE("Fail to allocate item_data_s class");
		return NULL;
	}

	id->gname = strdup(gname);
	id->title = strdup(title);

	if (!id->gname || !id->title) {
		LOGE("Failed to allocate memory");
		item_data_free(id);
		return NULL;
	}

	id->status = status;
	id->storeType = storeType;
	id->index = index;

	return id;
}

void item_data_free(item_data_s *id)
{
	if (id) {
		free(id->gname);
		free(id->title);
		free(id);
	}
}

Eina_Bool certStatusToEina(CertStatus status)
{
	return status == ENABLED ? EINA_TRUE : EINA_FALSE;
}

Eina_Bool quit_cb(void *data, Elm_Object_Item *it)
{
	LOGD("Start quit cb function.");
	struct ug_data *ad = (struct ug_data *)data;

	if (ad->ug == NULL)
		return EINA_FALSE;

	ug_destroy_me(ad->ug);
	ad->ug = NULL;
	ad->more_popup2 = NULL;

	return EINA_FALSE;
}

void list_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)elm_list_selected_item_get(obj);
	if (!it)
		return;

	elm_list_item_selected_set(it, EINA_FALSE);
}

void genlist_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_selected_item_get(obj);
	if (it == NULL)
		return;

	elm_genlist_item_selected_set(it, EINA_FALSE);
}

static void _popup_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_text_set(obj, dgettext(PACKAGE, data));
}

Evas_Object *create_yes_no_pop(struct ug_data *ad, const char *contentId)
{
	if (!ad)
		return NULL;

	ad->popup = elm_popup_add(ad->navi_bar);
	if (!ad->popup)
		return NULL;

	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* no button */
	Evas_Object *btn_no = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_no, PACKAGE, "IDS_ST_BUTTON_CANCEL");
	elm_object_style_set(btn_no, "popup");

	/* yes button */
	Evas_Object *btn_yes = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_yes, PACKAGE, "IDS_ST_BUTTON_INSTALL");
	evas_object_smart_callback_add(btn_yes, "clicked", _info_pop_response_yes_cb, ad);
	elm_object_style_set(btn_yes, "popup");

	if (ad->type_of_screen == PKCS12_SCREEN) {
		evas_object_smart_callback_add(btn_no, "clicked", _popup_quit_cb, ad);
		eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _popup_quit_cb, ad);
	} else {
		evas_object_smart_callback_add(btn_no, "clicked", _info_pop_response_no_cb, ad);
		eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _info_pop_response_no_cb, ad);
	}

	elm_object_domain_translatable_text_set(ad->popup, PACKAGE, contentId);
	evas_object_smart_callback_add(ad->popup, "language,changed", _popup_lang_changed, contentId);

	elm_object_part_content_set(ad->popup, "button1", btn_no);
	elm_object_part_content_set(ad->popup, "button2", btn_yes);
	elm_object_domain_translatable_part_text_set(ad->popup, "title,text", PACKAGE, "IDS_ST_HEADER_INSTALL_CERTIFICATE_ABB2");
	evas_object_show(ad->popup);

	return ad->popup;
}

void install_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;

	common_dismissed_cb(ad->win_main, obj, event_info);

	create_yes_no_pop(ad, "IDS_ST_POP_CERTIFICATES_FROM_THE_DEVICE_MEMORY_AND_SD_CARD_WILL_BE_INSTALLED");
}

Evas_Object *create_ok_pop(struct ug_data *ad, const char *contentId)
{
	if (!ad)
		return NULL;

	ad->popup = elm_popup_add(ad->navi_bar);
	if (!ad->popup)
		return NULL;

	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *btn_ok = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_ok, PACKAGE, "IDS_ST_SK2_OK");
	elm_object_style_set(btn_ok, "popup");
	evas_object_smart_callback_add(btn_ok, "clicked", _info_pop_response_ok_cb, ad->popup);

	/* set_object_text_with_alignment(ad->popup, dgettext(PACKAGE, contentId)); */
	elm_object_domain_translatable_text_set(ad->popup, PACKAGE, contentId);

	evas_object_smart_callback_add(ad->popup, "language,changed", _popup_lang_changed, contentId);
	elm_object_domain_translatable_part_text_set(ad->popup, "title,text", PACKAGE, "IDS_ST_HEADER_UNABLE_TO_INSTALL_CERTIFICATE_ABB");

	elm_object_part_content_set(ad->popup, "button1", btn_ok);

	eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _info_pop_response_ok_cb, ad->popup);

	evas_object_show(ad->popup);

	return ad->popup;
}

char *path_cat(const char *str1, char *str2)
{
	size_t str1_len = strlen(str1);
	char *result = NULL;
	int as_result;
	if (str1[str1_len - 1] != '/')
		as_result = asprintf(&result, "%s/%s", str1, str2);
	else
		as_result = asprintf(&result, "%s%s", str1, str2);

	if (as_result >= 0)
		return result;
	else
		return NULL;
}

struct ListElement *nextElement(struct ListElement *listElement)
{
	if (!listElement)
		return NULL;

	return listElement->next;
}

struct ListElement *initList()
{
	struct ListElement *firstListElement = (struct ListElement *)malloc(sizeof(struct ListElement));
	if (!firstListElement)
		return NULL;

	firstListElement->prev = NULL;
	firstListElement->next = NULL;
	firstListElement->gname = NULL;
	firstListElement->name = NULL;
	firstListElement->path = NULL;
	firstListElement->title = NULL;
	firstListElement->isChecked = EINA_FALSE;

	return firstListElement;
}

void freeListElement(struct ListElement *listElementToRemove)
{
	if (!listElementToRemove)
		return;

	if (listElementToRemove->name) {
		free(listElementToRemove->name);
		listElementToRemove->name = NULL;
	}

	if (listElementToRemove->gname) {
		free(listElementToRemove->gname);
		listElementToRemove->gname = NULL;
	}

	if (listElementToRemove->title) {
		free(listElementToRemove->title);
		listElementToRemove->title = NULL;
	}

	if (listElementToRemove->path) {
		free(listElementToRemove->path);
		listElementToRemove->path = NULL;
	}

}

struct ListElement *addListElement(
					struct ListElement *lastListElement,
					const char *title,
					const char *gname,
					const char *name,
					const char *path)
{
	if (!lastListElement)
		return NULL;

	struct ListElement *newListElement = initList();
	if (!newListElement)
		goto error;

	if(title != NULL) {
		newListElement->title = strdup(title);
		if (!newListElement->title)
			goto error;
	}

	if(gname != NULL) {
		newListElement->gname = strdup(gname);
		if (!newListElement->gname)
			goto error;
	}

	if(name != NULL) {
		newListElement->name = strdup(name);
		if (!newListElement->name)
			goto error;
	}

	if(path != NULL) {
		newListElement->path = strdup(path);
		if (!newListElement->path)
			goto error;
	}

	lastListElement->next = newListElement;
	newListElement->prev = lastListElement;
	newListElement->next = NULL;
	newListElement->isChecked = EINA_FALSE;

	return newListElement;

error:
	LOGW("Memory allocation error.");
	if(newListElement == NULL)
		freeListElement(newListElement);

	return NULL;
}

/*
 * Function returns position of removed element.
 */
enum PositionOfElement removeListElement(struct ListElement *listElementToRemove)
{
	if (!listElementToRemove)
		return NONE;

	struct ListElement *prev = listElementToRemove->prev;
	struct ListElement *next = listElementToRemove->next;

	if (prev && next) {
		prev->next = next;
		next->prev = prev;
		freeListElement(listElementToRemove);
		return IN_THE_MIDDLE;
	}

	if (prev && !next) {
		prev->next = NULL;
		freeListElement(listElementToRemove);
		return LAST;
	}

	if (!prev && next) {
		next->prev = NULL;
		freeListElement(listElementToRemove);
		return FIRST;
	}

	freeListElement(listElementToRemove);
	return THE_LAST_ONE;
}

struct ListElement *findLastElement(struct ListElement *listElement)
{
	struct ListElement *lastListElement = listElement;
	while (lastListElement->next)
		lastListElement = lastListElement->next;

	return lastListElement;
}

struct ListElement *findFirstElement(struct ListElement *listElement)
{
	struct ListElement *firstListElement = listElement;

	while (firstListElement->prev)
		firstListElement = firstListElement->prev;

	return firstListElement;
}

void deleteList(struct ListElement *listElement)
{
	if (!listElement)
		return;

	listElement = findFirstElement(listElement);
	struct ListElement *current = listElement;
	struct ListElement *next = NULL;

	while (current) {
		next = current->next;
		freeListElement(current);
		current = next;
	}
}

Eina_Bool back_cb(void *data, Elm_Object_Item *it)
{
	LOGD("Start back cb function.");
	struct ug_data *ad = (struct ug_data *)data;

	deleteList(ad->list_element);
	ad->list_element = NULL;

	return EINA_TRUE;
}


void back_install_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("Start back install cb function.");
	struct ug_data *ad = (struct ug_data *) data;

	deleteList(ad->list_element_install);
	ad->list_element_install = NULL;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = (item_data_s *)data;
	if (strcmp(part, "elm.text"))
		return NULL;

	return strdup(id->title);
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	LOGD("Start smart callback function.");
	item_data_s *id = (item_data_s *)data;

	CertSvcInstance instance;

	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("Failed to certsvc_instance_new().");
		return;
	}

	CertSvcString alias;
	if (certsvc_string_new(
			instance,
			id->gname,
			strlen(id->gname),
			&alias) != CERTSVC_SUCCESS) {
		LOGE("certsvc_string_new failed.");
		certsvc_instance_free(instance);
		return;
	}

	Eina_Bool isOn = elm_check_state_get(obj);

	if (certsvc_pkcs12_set_certificate_status_to_store(
			instance,
			SYSTEM_STORE,
			DISABLED,
			alias,
			(isOn ? ENABLED : DISABLED)) != CERTSVC_SUCCESS) {
		LOGE("CERTSVC_FAIL to enable/disable certificate status.");
	} else {
		id->status = (int)isOn;
	}

	certsvc_instance_free(instance);
	return;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check;
	item_data_s *id = (item_data_s *) data;
	Eina_Bool status = id->status;

	if (strcmp(part, "elm.swallow.end"))
		return NULL;

	check = elm_check_add(obj);

	elm_object_style_set(check, "on&off");
	elm_check_state_set(check, status);
	evas_object_repeat_events_set(check, EINA_FALSE);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(check, "changed", _chk_changed_cb, data);
	evas_object_show(check);

	return check;
}

Eina_Bool make_list(struct ug_data *ad, Evas_Object *genlist, const char *dir_path, struct ListElement *lastListElement)
{
	LOGD("Start to make list.");
	size_t length = 0;
	size_t index = 0;
	int result = 0;
	Elm_Object_Item *it = NULL;
	struct ListElement *current = NULL;
	Eina_Bool no_content_bool = EINA_TRUE;
	CertSvcStoreCertList *certList = NULL;
	CertSvcStoreCertList *certListHead = NULL;
	CertSvcInstance instance;

	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("Failed to certsvc_instance_new().");
		return no_content_bool;
	}

	result = certsvc_pkcs12_get_certificate_list_from_store(instance, SYSTEM_STORE, DISABLED, &certList, &length);
	if (result != CERTSVC_SUCCESS) {
		LOGD("Unable to get the certificate list from store. ret[%d]", result);
		certsvc_instance_free(instance);
		return no_content_bool;
	}

	certListHead = certList;
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	if (!itc) {
		certsvc_instance_free(instance);
		LOGE("Fail to allocate Elm_Genlist_Item_Class");
		return no_content_bool;
	}

	itc->item_style = "default";
	itc->func.content_get = _gl_content_get;
	itc->func.text_get = _gl_text_get;

	while (index < length) {
		no_content_bool = EINA_FALSE;
		item_data_s *id = item_data_create(
				certList->gname,
				certList->title,
				certList->status,
				SYSTEM_STORE,
				index);
		if (!id) {
			LOGE("Failed to allocate memory for item_data");
			certsvc_instance_free(instance);
			elm_genlist_item_class_free(itc);
			return no_content_bool;
		}

		current = addListElement(lastListElement, certList->title, certList->gname, NULL, NULL);
		if (!current) {
			certsvc_instance_free(instance);
			elm_genlist_item_class_free(itc);
			item_data_free(id);
			LOGE("currrent is NULL. ");
			return no_content_bool;
		}
		current->storeType = SYSTEM_STORE;
		lastListElement = current;

		it = elm_genlist_item_append(
				genlist,
				itc,
				id,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_cert_selection_cb,
				current);

		if (!it)
			LOGE("Error in elm_list_item_append");

		index++;
		certList = certList->next;
	}

	certsvc_pkcs12_free_certificate_list_loaded_from_store(instance, &certListHead);
	ad->list_to_refresh = genlist;
	elm_genlist_item_class_free(itc);
	certsvc_instance_free(instance);

	return no_content_bool;
}

Evas_Object *create_no_content_layout(struct ug_data *ad)
{
	char buf[256] = {0,};

	if (ad == NULL)
		return NULL;

	Evas_Object *no_content = elm_layout_add(ad->win_main);
	if (no_content == NULL)
		return NULL;

	elm_layout_theme_set(no_content, "layout", "nocontents", "default");
	evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *icon = elm_image_add(no_content);

	snprintf(buf, sizeof(buf), "%s/00_nocontents_text_gray.png", IMGDIR);

	elm_image_file_set(icon, buf, NULL);
	elm_object_part_content_set(no_content, "nocontents.image", icon);

	elm_object_domain_translatable_part_text_set(no_content, "elm.text", PACKAGE, "IDS_ST_BODY_NO_CONTENT");
	evas_object_show(no_content);

	return no_content;
}

bool isEmptyStr(const char *str)
{
	if (!str || '\0' == str[0])
		return TRUE;

	return FALSE;
}

int safeStrCmp(const char *s1, const char *s2)
{
	if (isEmptyStr(s1) && isEmptyStr(s2))
		return 0;

	if (isEmptyStr(s1))
		return 1;

	if (isEmptyStr(s2))
		return -1;

	return strcmp(s1, s2);
}

static char *get_user_name(void)
{
	struct passwd pwd;
	struct passwd *result;
	char *buf;
	char *user_name;
	int buf_size;

	buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (buf_size == -1)
		buf_size = 16384;

	buf = (char *)malloc(buf_size);
	if (buf == NULL)
		return NULL;

	if (getpwuid_r(getuid(), &pwd, buf, buf_size, &result) != 0) {
		free(buf);
		return NULL;
	}

	user_name = strdup(pwd.pw_name);
	free(buf);
	return (user_name == NULL) ? NULL : user_name;
}

static char *generate_path_with_username(const char *format)
{
	char *username = get_user_name();
	if (username == NULL)
		return NULL;

	/* -2 for %s characters in format */
	size_t len = strlen(username) + strlen(format) - 2;
	char *media_path = (char *)malloc(sizeof(char) * len + 1);
	if (media_path == NULL) {
		free(username);
		return NULL;
	}

	snprintf(media_path, len + 1, format, username);

	free(username);

	return media_path;
}

char *get_media_path(void)
{
	const char *format = tzplatform_mkpath(TZ_SYS_HOME, "%s/content");
	return generate_path_with_username(format);
}

char *get_media_downloads_path(void)
{
	const char *format = tzplatform_mkpath(TZ_SYS_HOME, "%s/content/Downloads");
	return generate_path_with_username(format);
}

char *get_sdcard_path(void)
{
	/* TODO: find sdcard path */
	return get_media_downloads_path();
}
