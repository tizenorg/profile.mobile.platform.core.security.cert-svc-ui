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
 * @version     1.0
 * @brief
 */

#define _GNU_SOURCE

#include <dlog.h>

#include <efl_extension.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>

#include "mgr-app-uigadget.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#define CERT_GENLIST_1TEXT1EDIT_STYLE  "dialogue/1text.1icon.5"
#define CERT_GENLIST_TITLE_STYLE "dialogue/title"

static const char* const align_begin = "<align=center>";
static const char* const align_end   = "</align>";

static void _cert_selection_cb(void *data, Evas_Object *obj, void *event_info);

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
	if (ad->popup)
		evas_object_del(ad->popup);

	ad->popup = NULL;
	pfx_cert_install_cb(ad, NULL, NULL);
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
	struct ug_data *ad = (struct ug_data*) data;
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
    int status,
    int storeType,
    int index)
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
        free(id->gname);
        free(id->title);
        free(id);
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

Eina_Bool quit_cb(void *data, Elm_Object_Item *it)
{
	LOGD("quit_cb");
	struct ug_data *ad = (struct ug_data *)data;

	if (ad->ug) {
		deleteList(ad->list_element);
		ad->list_element = NULL;
		elm_naviframe_item_pop(ad->navi_bar);
		ug_destroy_me(ad->ug);
		ad->ug = NULL;
		ad->more_popup2 = NULL;
	}

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


void set_object_text_with_alignment(Evas_Object *object, const char *content)
{
	char *text = NULL;
	size_t i = 0;
	size_t j = 0;
    size_t align_begin_len = strlen(align_begin);
    size_t align_end_len = strlen(align_end);
    size_t content_len = strlen(content);
	text = (char *)malloc((align_begin_len + align_end_len + content_len + 1) * sizeof(char));
	if (!text) {
		LOGE("Failed to allocate memory");
		return;
	}

	for (i = 0; i < align_begin_len; ++i) {
		text[j] = align_begin[i];
		++j;
	}
	for (i = 0; i < content_len; ++i) {
		text[j] = content[i];
		++j;
	}
	for (i = 0; i < align_end_len; ++i) {
		text[j] = align_end[i];
		++j;
	}

	text[j] = 0;

	elm_object_text_set(object, text);
	free(text);
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

	//no button
	Evas_Object *btn_no = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_no, PACKAGE, "IDS_ST_BUTTON_CANCEL");
	elm_object_style_set(btn_no, "popup");

	//yes button
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

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_del(obj,"dismissed", _dismissed_cb);
	evas_object_del(obj);
}


void install_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("install_button_cb()");
	struct ug_data *ad = (struct ug_data *)data;
	if (ad)
		_dismissed_cb(data, ad->more_popup2, event_info);

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

	//ok button
	Evas_Object *btn_ok = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_ok, PACKAGE, "IDS_ST_SK2_OK");
	elm_object_style_set(btn_ok, "popup");
	evas_object_smart_callback_add(btn_ok, "clicked", _info_pop_response_ok_cb, ad->popup);

	//set_object_text_with_alignment(ad->popup, dgettext(PACKAGE, contentId));
	elm_object_domain_translatable_text_set(ad->popup, PACKAGE, contentId);

	evas_object_smart_callback_add(ad->popup, "language,changed", _popup_lang_changed, contentId);
	elm_object_domain_translatable_part_text_set(ad->popup,"title,text", PACKAGE, "IDS_ST_HEADER_UNABLE_TO_INSTALL_CERTIFICATE_ABB");

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

// extract title from certificate ------------------
char *extractDataFromCert(char *path)
{
	struct ug_data *ad = get_ug_data();

	CertSvcCertificate cert;
	CertSvcString buffer;
	char *char_buffer = NULL;

	if (CERTSVC_SUCCESS != certsvc_certificate_new_from_file(ad->instance, path, &cert)) {
		LOGD("certsvc_certificate_new_from_file has been succeeded");
		return NULL;
	}

	if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_COMMON_NAME, &buffer) && buffer.privateLength > 0) {
		LOGD("certsvc_certificate_get_string_field for the CN field has been succeeded");
		goto CATCH;
	}

	certsvc_string_free(buffer);

	if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_ORGANIZATION_NAME, &buffer) && buffer.privateLength > 0) {
		LOGD("certsvc_certificate_get_string_field for the O field has been succeeded");
		goto CATCH;
	}

	certsvc_string_free(buffer);

	if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_ORGANIZATION_UNIT_NAME, &buffer) && buffer.privateLength > 0) {
		LOGD("certsvc_certificate_get_string_field for the OU field has been succeeded");
		goto CATCH;
	}

	certsvc_string_free(buffer);

	if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_EMAIL_ADDRESS, &buffer) && buffer.privateLength > 0) {
		LOGD("certsvc_certificate_get_string_field for the emailAddress field has been succeeded");
		goto CATCH;
	}

	certsvc_string_free(buffer);
	certsvc_certificate_free(cert);
	return NULL;

CATCH:
	char_buffer = strndup(buffer.privateHandler, buffer.privateLength);
	LOGD("char_buffer : %s", char_buffer);

	certsvc_string_free(buffer);
	certsvc_certificate_free(cert);

	return char_buffer;
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

struct ListElement *addListElement(struct ListElement *lastListElement, const char *name)
{
	if (!lastListElement)
		return NULL;

	struct ListElement *newListElement = initList();
	if (!newListElement)
		return NULL;

	SECURE_LOGD("name %s", name);
	newListElement->name = malloc((strlen(name) + 1) * sizeof(char));
	if (!newListElement->name) {
		free(newListElement);
		return NULL;
	}

	strncpy(newListElement->name, name, strlen(name) + 1);
	SECURE_LOGD("name %s", newListElement->name);

	newListElement->title = NULL;
	newListElement->path = NULL;

	lastListElement->next = newListElement;
	newListElement->prev = lastListElement;
	newListElement->next = NULL;
	newListElement->isChecked = EINA_FALSE;

	return newListElement;
}

struct ListElement *addListElementWithPath(struct ListElement *lastListElement, const char *name, const char *path)
{
	struct ListElement *newListElement = addListElement(lastListElement, name);
	if (!newListElement)
		return NULL;

	newListElement->path = strdup(path);
	if (!newListElement->path) {
		free(newListElement->name);
		free(newListElement);
		return NULL;
	}

	lastListElement->next = newListElement;
	newListElement->prev = lastListElement;
	newListElement->next = NULL;
	newListElement->isChecked = EINA_FALSE;

	return newListElement;
}

struct ListElement *addListElementWithPathAndTitle(struct ListElement *lastListElement, const char *gname, const char *path, const char *title)
{
	struct ListElement *newListElement = initList();
	if (!newListElement)
		return NULL;

	newListElement->title = strdup(title);
	newListElement->gname = strdup(gname);

	if (!newListElement->title || !newListElement->gname) {
		free(newListElement->title);
		free(newListElement->gname);
		free(newListElement);
		return NULL;
	}

	if (lastListElement) {
		lastListElement->next = newListElement;
		newListElement->prev  = lastListElement;
	}

	newListElement->next = NULL;
	newListElement->isChecked = EINA_FALSE;

	return newListElement;
}

void freeListElement(struct ListElement *listElementToRemove)
{
	if (!listElementToRemove)
		return;

	if (listElementToRemove->gname) {
		free(listElementToRemove->gname);
		listElementToRemove->gname = NULL;
	}

	if (listElementToRemove->title) {
		free(listElementToRemove->title);
		listElementToRemove->title = NULL;
	}
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

struct ListElement *findFirstElement(struct ListElement* listElement)
{
	struct ListElement *firsListElement = listElement;
	while (firsListElement->prev)
		firsListElement = firsListElement->prev;

	return firsListElement;
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

// ------------------------------------------------
//@@@test
Eina_Bool back_cb(void *data, Elm_Object_Item *it)
{
	LOGD("back_cb");
	struct ug_data *ad = (struct ug_data *)data;

	deleteList(ad->list_element);
	ad->list_element = NULL;

	return EINA_TRUE;
}


void back_install_cb(void *data, Evas_Object *obj, void *event_info)
{
	LOGD("back_install_cb");
	struct ug_data *ad = (struct ug_data *) data;

	deleteList(ad->list_element_install);
	ad->list_element_install = NULL;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = data;
	if (!strcmp(part, "elm.text.main.left"))
		return strdup(id->title);

	return NULL;
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	item_data_s *id = (item_data_s *)data;

	CertSvcString alias;
	alias.privateHandler = strdup(id->gname);
	if (!alias.privateHandler) {
		LOGE("Failed to allocate memory");
		return;
	}
	alias.privateLength = strlen(id->gname);

	CertSvcInstance instance;
	if (certsvc_instance_new(&instance) != CERTSVC_SUCCESS) {
		LOGE("CERTSVC_FAIL to create instance");
		free(alias.privateHandler);
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
	Evas_Object *content;

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

Eina_Bool make_list(struct ug_data *ad, Evas_Object *genlist, const char *dir_path, struct ListElement *lastListElement)
{
	int length = 0;
	int result = 0;
	int index = 0;
	Elm_Object_Item *it = NULL;
	struct ListElement *current = NULL;
	Eina_Bool no_content_bool = EINA_TRUE;
	CertSvcStoreCertList* certList = NULL;
	CertSvcStoreCertList* certListHead = NULL;
	CertSvcInstance instance;

	if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
		LOGD("CERTSVC_FAIL to create instance");
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

	itc->item_style = "1line";
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

		current = addListElementWithPathAndTitle(lastListElement, certList->gname, NULL, certList->title);
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
				genlist,				/* genlist object */
				itc,					/* item class */
				id,   					/* item class user data */
				NULL,
				ELM_GENLIST_ITEM_NONE,	/* item type */
				_cert_selection_cb,		/* select smart callback */
				current);				/* smart callback user data */

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

	get_info_cert_from_file_cb(ad,data);
}

Evas_Object *create_no_content_layout(struct ug_data *ad)
{
	char buf[256] = {0,};

	if(!ad)
		return NULL;

	Evas_Object *no_content = elm_layout_add(ad->win_main);
	if(!no_content)
		return NULL;

	elm_layout_theme_set(no_content, "layout", "nocontents", "default");
	evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object* icon = elm_image_add(no_content);

	snprintf(buf, sizeof(buf), "%s/00_nocontents_text_gray.png", CERTSVC_UI_IMG_PATH);

	elm_image_file_set(icon, buf, NULL);
	elm_object_part_content_set(no_content, "nocontents.image", icon);

	elm_object_domain_translatable_part_text_set(no_content, "elm.text", PACKAGE, "IDS_ST_BODY_NO_CONTENT");
	evas_object_show(no_content);

	return no_content;
}

const char *get_email(CertSvcString alias)
{
	struct ug_data *ad = get_ug_data();

	const char *char_buffer;

	CertSvcCertificateList certificateList;
	CertSvcCertificate certificate;
	CertSvcString email_buffer;
	if (CERTSVC_SUCCESS != certsvc_pkcs12_load_certificate_list(
			ad->instance,
			alias,
			&certificateList))
		return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");

	if (CERTSVC_SUCCESS != certsvc_certificate_list_get_one(
			certificateList,
			0,
			&certificate))
		return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");

	if (CERTSVC_SUCCESS != certsvc_certificate_get_string_field(
			certificate,
			CERTSVC_SUBJECT_EMAIL_ADDRESS,
			&email_buffer))
		return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");

	certsvc_string_to_cstring(email_buffer, &char_buffer, NULL);
	if (!char_buffer) {
		LOGD("email is NULL; return NO_DATA");
		return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");
	}

	LOGD("get_email - return %s", char_buffer);
	return char_buffer;
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
