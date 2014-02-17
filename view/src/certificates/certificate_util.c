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

#include <Ecore_X.h>
#include <e_dbus-1/E_Notify.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>
#include <cert-svc/cpkcs12.h>
#include <efl_assist.h>

#include "mgr-app-uigadget.h"
#include "dlog.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#define CERT_GENLIST_1TEXT1EDIT_STYLE  "dialogue/1text.1icon.5"
#define CERT_GENLIST_TITLE_STYLE "dialogue/title"

static const char* const align_begin = "<align=center>";
static const char* const align_end   = "</align>";

static void _cert_selection_cb(void *data, Evas_Object *obj, void *event_info);

static void _info_pop_response_no_cb(void *data, Evas_Object *obj, void *event_info) {

	struct ug_data *ad = (struct ug_data *)data;
	evas_object_del(ad->popup);
	ad->popup = NULL;
}

static void _info_pop_response_yes_cb(void *data, Evas_Object *obj, void *event_info) {

    //struct ug_data *ad = get_ug_data();
	struct ug_data *ad = (struct ug_data *)data;
    pfx_cert_install_cb(ad, NULL, NULL);
    evas_object_del(ad->popup);
    ad->popup = NULL;
}

static void _info_pop_response_ok_cb(void *data, Evas_Object *obj, void *event_info) {

    if (NULL == data) {
        return;
    }
    Evas_Object *pop = (Evas_Object *) data;
    evas_object_del(pop);
}

static void _popup_quit_cb(void *data, Evas_Object *obj, void *event_info)
{
    struct ug_data *ad = (struct ug_data*) data;

	evas_object_del(ad->popup);
	ad->popup = NULL;

	if (ad->ug) {
		ug_destroy_me(ad->ug);
		ad->ug = NULL;
	}

}

Eina_Bool quit_cb(void *data, Elm_Object_Item *it)
{
    struct ug_data *ad = (struct ug_data*) data;

	if (ad->ug) {
		ug_destroy_me(ad->ug);
		ad->ug = NULL;
	}

	return EINA_TRUE;   
}

void list_clicked_cb(void *data, Evas_Object *obj, void *event_info) {
    Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
    if (it == NULL)
        return;

    elm_list_item_selected_set(it, EINA_FALSE);
}

void genlist_clicked_cb(void *data, Evas_Object *obj, void *event_info) {
    Elm_Object_Item *it = (Elm_Object_Item *) elm_genlist_selected_item_get(obj);
    if (it == NULL)
        return;

    elm_genlist_item_selected_set(it, EINA_FALSE);
}

//create yes/no popup
Evas_Object *create_yes_no_pop(struct ug_data *ad, const char *content) {

    if (NULL == ad){
        return NULL;
    }

    ad->popup = elm_popup_add(ad->navi_bar);
    if (NULL == ad->popup){
        return NULL;
    }

    char *text = NULL;
    int i = 0;
    int j = 0;
    text = malloc ( ( strlen(content) + strlen(align_begin) + strlen(align_end) + 1 ) * sizeof(char));
    for (i=0; i<strlen(align_begin); ++i) {
        text[j] = align_begin[i];
        ++j;
    }
    for (i=0; i<strlen(content); ++i) {
        text[j] = content[i];
        ++j;
    }
    for (i=0; i<strlen(align_end); ++i) {
        text[j] = align_end[i];
        ++j;
    }
    text[j] = 0;

    evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    //yes button
    Evas_Object *btn_yes = elm_button_add(ad->popup);
    elm_object_text_set(btn_yes, dgettext(PACKAGE, "IDS_ST_BUTTON_YES"));
    evas_object_smart_callback_add(btn_yes, "clicked", _info_pop_response_yes_cb, ad);

    //no button
    Evas_Object *btn_no = elm_button_add(ad->popup);
    elm_object_text_set(btn_no, dgettext(PACKAGE, "IDS_ST_BUTTON_NO"));

    if (ad->type_of_screen == PKCS12_SCREEN) {
    	evas_object_smart_callback_add(btn_no, "clicked", _popup_quit_cb, ad);
    	ea_object_event_callback_add(ad->popup, EA_CALLBACK_BACK, _popup_quit_cb, ad);
    }
    else {
    	evas_object_smart_callback_add(btn_no, "clicked", _info_pop_response_no_cb, ad);
    	ea_object_event_callback_add(ad->popup, EA_CALLBACK_BACK, _info_pop_response_no_cb, ad);
    }

    elm_object_text_set(ad->popup, text);
    free(text);

    elm_object_part_content_set(ad->popup, "button1", btn_yes);
    elm_object_part_content_set(ad->popup, "button2", btn_no);

    evas_object_show(ad->popup);

    return ad->popup;
}

void install_button_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("install_button_cb()");
    //struct ug_data *ad = get_ug_data();
    struct ug_data *ad = (struct ug_data *)data;
    create_yes_no_pop(ad, dgettext(PACKAGE, "IDS_ST_BODY_INSTALL_CERTIFICATES_FROM_SD_CARD_Q"));
}

Evas_Object *create_ok_pop(struct ug_data *ad, const char *content) {

    if (NULL == ad) {
        return NULL;
    }

    ad->popup = elm_popup_add(ad->navi_bar);
    if (NULL == ad->popup) {
        return NULL;
    }

    char *text = NULL;
    int i = 0;
    int j = 0;
    text = malloc((strlen(content) + strlen(align_begin) + strlen(align_end) + 1) * sizeof(char));
    for (i = 0; i < strlen(align_begin); ++i) {
        text[j] = align_begin[i];
        ++j;
    }
    for (i = 0; i < strlen(content); ++i) {
        text[j] = content[i];
        ++j;
    }
    for (i = 0; i < strlen(align_end); ++i) {
        text[j] = align_end[i];
        ++j;
    }
    text[j] = 0;

    evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    //ok button
    Evas_Object *btn_ok = elm_button_add(ad->popup);
    elm_object_text_set(btn_ok, dgettext(PACKAGE, "IDS_ST_SK2_OK"));
    evas_object_smart_callback_add(btn_ok, "clicked", _info_pop_response_ok_cb, ad->popup);

    elm_object_text_set(ad->popup, text);
    free(text);

    elm_object_part_content_set(ad->popup, "button1", btn_ok);

    ea_object_event_callback_add(ad->popup, EA_CALLBACK_BACK, _info_pop_response_ok_cb, ad->popup);

    evas_object_show(ad->popup);

    return ad->popup;
}

char *path_cat(const char *str1, char *str2) {
    size_t str1_len = strlen(str1);
    char *result = NULL;
    int as_result;
    if (str1[str1_len - 1] != '/') {
        as_result = asprintf(&result, "%s/%s", str1, str2);
    } else {
        as_result = asprintf(&result, "%s%s", str1, str2);
    }

    if (as_result >= 0)
        return result;
    else
        return NULL;
}

// extract title from certificate ------------------
char *extractDataFromCert(char *path) {

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
    } else {
        certsvc_string_free(buffer);
    }

    if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_ORGANIZATION_NAME, &buffer) && buffer.privateLength > 0) {
        LOGD("certsvc_certificate_get_string_field for the O field has been succeeded");
        goto CATCH;
    } else {
        certsvc_string_free(buffer);
    }

    if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_ORGANIZATION_UNIT_NAME, &buffer) && buffer.privateLength > 0) {
        LOGD("certsvc_certificate_get_string_field for the OU field has been succeeded");
        goto CATCH;
    } else {
        certsvc_string_free(buffer);
    }

    if (CERTSVC_SUCCESS == certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_EMAIL_ADDRESS, &buffer) && buffer.privateLength > 0) {
        LOGD("certsvc_certificate_get_string_field for the emailAddress field has been succeeded");
        goto CATCH;
    } else {
        certsvc_string_free(buffer);
		certsvc_certificate_free(cert);
		return NULL;
    }

CATCH:
    char_buffer = strndup(buffer.privateHandler, buffer.privateLength);
    LOGD("char_buffer : %s", char_buffer);

    certsvc_string_free(buffer);
    certsvc_certificate_free(cert);

    return char_buffer;
}

// List --------------------------------------------------------

struct ListElement* nextElement(struct ListElement *listElement) {
    if (listElement == NULL)
        return NULL;

    return listElement->next;
}

struct ListElement * initList() {

    LOGD("initList()");
    struct ListElement *firstListElement = (struct ListElement*) malloc(sizeof(struct ListElement));
    if (firstListElement == NULL)
        return NULL;

    firstListElement->prev = NULL;
    firstListElement->next = NULL;

    firstListElement->name = NULL;
    firstListElement->title = NULL;
    firstListElement->path = NULL;

    firstListElement->it = NULL;
    firstListElement->isChecked = EINA_FALSE;

    return firstListElement;
}

struct ListElement* addListElement(struct ListElement* lastListElement, const char *name) {

    if (NULL == lastListElement) {
        return NULL;
    }
    struct ListElement *newListElement = (struct ListElement*) malloc(sizeof(struct ListElement));
    if (newListElement == NULL)
        return NULL;

    SECURE_LOGD("name %s", name);
    newListElement->name = malloc((strlen(name) + 1) * sizeof(char));
    if (NULL == newListElement->name) {
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

    newListElement->it = NULL;
    newListElement->isChecked = EINA_FALSE;

    return newListElement;
}

struct ListElement* addListElementWithTitle(struct ListElement* lastListElement, const char *name, const char *title) {

    struct ListElement *newListElement = addListElement(lastListElement, name);
    if(NULL == newListElement){
        return NULL;
    }

    newListElement->title = malloc((strlen(title)+1) * sizeof(char));
    if(NULL == newListElement->title){
        free(newListElement->name);
        free(newListElement);
        return NULL;
    }

    strncpy(newListElement->title, title, strlen(title)+1);

    lastListElement->next = newListElement;
    newListElement->prev  = lastListElement;
    newListElement->next  = NULL;

    newListElement->it        = NULL;
    newListElement->isChecked = EINA_FALSE;

    return newListElement;
}

struct ListElement* addListElementWithPath(struct ListElement* lastListElement, const char *name, const char *path) {

    struct ListElement *newListElement = addListElement(lastListElement, name);
    if (NULL == newListElement) {
        return NULL;
    }

    newListElement->path = malloc((strlen(path)+1) * sizeof(char));
    if (NULL == newListElement->path) {
        free(newListElement->name);
        free(newListElement);
        return NULL;
    }

    strncpy(newListElement->path, path, strlen(path)+1);

    lastListElement->next = newListElement;
    newListElement->prev  = lastListElement;
    newListElement->next  = NULL;

    newListElement->it        = NULL;
    newListElement->isChecked = EINA_FALSE;

    return newListElement;
}

struct ListElement* addListElementWithPathAndTitle(struct ListElement* lastListElement, const char *name, const char *path, const char *title) {

    struct ListElement *newListElement = addListElementWithPath(lastListElement, name, path);
    if (NULL == newListElement) {
        return NULL;
    }

    newListElement->title = malloc((strlen(title)+1) * sizeof(char));
    if (NULL == newListElement->title) {
        free(newListElement->name);
        free(newListElement->path);
        free(newListElement);
        return NULL;
    }

    strncpy(newListElement->title, title, strlen(title)+1);

    lastListElement->next = newListElement;
    newListElement->prev  = lastListElement;
    newListElement->next  = NULL;

    newListElement->it        = NULL;
    newListElement->isChecked = EINA_FALSE;

    return newListElement;
}

void freeListElement(struct ListElement *listElementToRemove){

    if(NULL == listElementToRemove){
        return;
    }

    if (NULL != listElementToRemove->name) {
        free(listElementToRemove->name);
    }

    if (NULL != listElementToRemove->path) {
        free(listElementToRemove->path);
    }

    if (NULL != listElementToRemove->title) {
        free(listElementToRemove->title);
    }
}

/*
 * Function returns position of removed element.
 */
enum PositionOfElement removeListElement(struct ListElement *listElementToRemove) {

    if (listElementToRemove == NULL)
        return NONE;

    struct ListElement* prev;
    struct ListElement* next;

    if (listElementToRemove->prev != NULL && listElementToRemove->next != NULL) {
        prev = listElementToRemove->prev;
        next = listElementToRemove->next;
        prev->next = next;
        next->prev = prev;
        freeListElement(listElementToRemove);
        return IN_THE_MIDDLE;
    } else if (listElementToRemove->prev != NULL && listElementToRemove->next == NULL) {
        prev = listElementToRemove->prev;
        prev->next = NULL;
        freeListElement(listElementToRemove);
        return LAST;
    } else if (listElementToRemove->prev == NULL && listElementToRemove->next != NULL) {
        next = listElementToRemove->next;
        next->prev = NULL;
        freeListElement(listElementToRemove);
        return FIRST;
    } else if (listElementToRemove->prev == NULL && listElementToRemove->next == NULL) {
        freeListElement(listElementToRemove);
        return THE_LAST_ONE;
    }

    return NONE;
}

/*
 * To find last element of the list put as a parameter any element of this list
 */
struct ListElement * findLastElement(struct ListElement* listElement) {
    struct ListElement *lastListElement = listElement;
    while (lastListElement->next != NULL) {
        lastListElement = lastListElement->next;
    }
    return lastListElement;
}

/*
 * To find first element of the list put as a parameter any element of this list
 */
struct ListElement * findFirstElement(struct ListElement* listElement) {
    LOGD("findFirstElement");
    struct ListElement *firsListElement = listElement;
    while (firsListElement->prev != NULL) {
        firsListElement = firsListElement->prev;
    }
    return firsListElement;
}

/*
 * After this is done you have to call initList(...) to use the list again;
 */
void deleteList(struct ListElement* listElement) {
    LOGD("deleteList");
    if (listElement == NULL)
        return;
    listElement = findFirstElement(listElement);
    struct ListElement *current, *tmp;
    current = listElement;
    int count = 0;
    while (current != NULL) {
        tmp = current->next;
        freeListElement(current);
        current = tmp;
        count++;
    }
    LOGD("Removed %d elements", count);
}
// ------------------------------------------------
//@@@test
Eina_Bool back_cb(void *data, Elm_Object_Item *it)
{
    LOGD("back_cb");
    struct ug_data *ad = (struct ug_data *) data;

    deleteList(ad->list_element);
    ad->list_element = NULL;
	   
	return EINA_TRUE;   
}



void back_install_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("back_install_cb");
    struct ug_data *ad = (struct ug_data *) data;

    deleteList(ad->list_element_install);
    ad->list_element_install = NULL;
}

Eina_Bool make_list(struct ug_data *ad, Evas_Object *list, const char *dir_path, struct ListElement *lastListElement) {

    LOGD("make_list()");
    DIR                *dir;
    struct dirent      *dp;
    struct ListElement *current;
    Elm_Object_Item    *it;
    char               *title = NULL;
    Eina_Bool          no_content_bool = EINA_TRUE;

    /* ---------------------------------------------------
     * Checking for files (certificates) in dir_path
     */
    dir = opendir(dir_path);
    if (dir == NULL) {
        LOGE("There's no such directory: %s", dir_path);
        return no_content_bool;
    }
    LOGD("Scanning dir (%s) - looking for certs", dir_path);
    while ((dp = readdir(dir)) != NULL) {
        char *tmp = NULL;
        tmp = path_cat(dir_path, dp->d_name);
        if (strncmp(dp->d_name, "..", 2) != 0 && strncmp(dp->d_name, ".", 1) != 0 && !(dp->d_type == DT_DIR)) {

            title = extractDataFromCert(tmp);
            if (!title) {
                free(tmp);
                tmp = NULL;
                LOGD("Extract data returned NULL pointer");
                continue;
            } else {
                SECURE_LOGD("title = \"%s\"", title);
                SECURE_LOGD("name  = %s", dp->d_name);
                no_content_bool = EINA_FALSE;

                current = addListElementWithPathAndTitle(lastListElement, dp->d_name, dir_path, title);
                lastListElement = current;

				LOGD("strlen(title) = %d", strlen(title));
				if (strlen(title) < 1){
					// if Common name of cert is empty print the name of file instead.
					it = elm_list_item_append(list, dp->d_name, NULL, NULL, _cert_selection_cb, current);
					if (!it){
						LOGE("Error in elm_list_item_append");
					}
				} else {
					it = elm_list_item_append(list, title, NULL, NULL, _cert_selection_cb, current);
					if (!it){
						LOGE("Error in elm_list_item_append");
					}
				}

                SECURE_LOGD("elm list append = %s", current->name);

                free(title);
                title = NULL;
            }
        }
        if (tmp)
            free(tmp);
    }
    closedir(dir);
    dir = NULL;

    return no_content_bool;
}

Evas_Object *create_no_content_layout (struct ug_data *ad) {

    if(NULL == ad){
        return NULL;
    }
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

static void _cert_selection_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("_cert_selection_cb ()");
    if (NULL == data){
        return;
    }
    struct ListElement *current = (struct ListElement *) data;
    char *path = path_cat(current->path, current->name);
    SECURE_LOGD("cert_path = %s", path);

    Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
    if (it == NULL){
        free(path);
        return;
    }
    elm_list_item_selected_set(it, EINA_FALSE);

    struct ug_data *ad = get_ug_data();
    ad->data = (void *) strdup(path);

    get_info_cert_from_file_cb(ad);

    free(path);
}


const char* get_email(CertSvcString alias) {
    LOGD("get_email()");

    struct ug_data *ad = get_ug_data();

    const char *char_buffer;

    CertSvcCertificateList certificateList;
    CertSvcCertificate certificate;
    CertSvcString email_buffer;
    if (CERTSVC_SUCCESS != certsvc_pkcs12_load_certificate_list(
            ad->instance,
            alias,
            &certificateList)) {
        return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");
    }
    if (CERTSVC_SUCCESS != certsvc_certificate_list_get_one(
            certificateList,
            0,
            &certificate)) {
        return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");
    }
    if (CERTSVC_SUCCESS != certsvc_certificate_get_string_field(
            certificate,
            CERTSVC_SUBJECT_EMAIL_ADDRESS,
            &email_buffer)) {
        return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");
    }
    certsvc_string_to_cstring(email_buffer, &char_buffer, NULL);
    LOGD("get_email - return %s", char_buffer);
    if(NULL == char_buffer){
        LOGD("email is NULL; return NO_DATA");
        return dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");
    }
    return char_buffer;
}
