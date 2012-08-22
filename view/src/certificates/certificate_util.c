/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of the Manage Applications
 * Written by Eunmi Son <eunmi.son@samsung.com>
 *
 * Author of this file:
 * Janusz Kozerski <j.kozerski@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with SAMSUNG ELECTRONICS.
 *
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not limited
 * to the implied warranties of merchantability, fitness for a particular
 * purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or
 * distributing this software or its derivatives.
 *
 */

#define _GNU_SOURCE

#include <Ecore_X.h>
#include <E_Notify.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>

#include "mgr-app-uigadget.h"
#include "dlog.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#define CERT_GENLIST_1TEXT1EDIT_STYLE  "dialogue/1text.1icon.5"
#define CERT_GENLIST_TITLE_STYLE "dialogue/title"

static struct ug_data *appdata;

static void _info_pop_response_no_cb(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *pop = (Evas_Object *) data;

	evas_object_del(pop);
}

static void _info_pop_response_yes_cb(void *data, Evas_Object *obj, void *event_info) {
	Evas_Object *pop = (Evas_Object *) data;

	user_search_cert_cb(appdata, NULL, NULL); //TODO: Using global value(appdata) is not nice.

	evas_object_del(pop);
}

static void _info_pop_response_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    Evas_Object *pop = (Evas_Object *) data;

    evas_object_del(pop);
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

	appdata = ad;
	Evas_Object *pop = NULL;

	pop = elm_popup_add(ad->win_main);
	evas_object_size_hint_weight_set(pop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	//yes button
	Evas_Object *btn_yes = elm_button_add(pop);
	elm_object_text_set(btn_yes, dgettext(PACKAGE, "YES_ANSWER")); // TODO: You shouldn't use magic string. This will make localization very hard.
	evas_object_smart_callback_add(btn_yes, "clicked", _info_pop_response_yes_cb, pop); //TODO: how to pass appdata (other way than get_appdata();)?

	//no button
	Evas_Object *btn_no = elm_button_add(pop);
	elm_object_text_set(btn_no, dgettext(PACKAGE, "NO_ANSWER")); // TODO: You shouldn't use magic string. This will make localization very hard.
	evas_object_smart_callback_add(btn_no, "clicked", _info_pop_response_no_cb, pop);

	Evas_Object *label = elm_label_add(pop);
	elm_object_text_set(label, content);

	elm_object_part_content_set(pop, "default", label);
	elm_object_part_content_set(pop, "button1", btn_yes);
	elm_object_part_content_set(pop, "button2", btn_no);

	evas_object_show(pop);

	return pop;
}

Evas_Object *create_ok_pop(struct ug_data *ad, const char *content) {

    appdata = ad;
    Evas_Object *pop = NULL;

    pop = elm_popup_add(ad->win_main);
    evas_object_size_hint_weight_set(pop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    //yes button
    Evas_Object *btn_ok = elm_button_add(pop);
    elm_object_text_set(btn_ok, dgettext(PACKAGE, "OK_BUTTON"));
    evas_object_smart_callback_add(btn_ok, "clicked", _info_pop_response_ok_cb, pop);

    Evas_Object *label = elm_label_add(pop);
    elm_object_text_set(label, content);

    elm_object_part_content_set(pop, "default", label);
    elm_object_part_content_set(pop, "button1", btn_ok);

    evas_object_show(pop);

    return pop;
}

char *path_cat(const char *str1, char *str2) {
	size_t str1_len = strlen(str1);
	char *result;
	int as_result;
	if (str1[str1_len - 1] != '/') {
	    as_result = asprintf(&result, "%s/%s", str1, str2);
	} else {
	    as_result = asprintf(&result, "%s%s", str1, str2);
	}

	if(as_result != -1)
	    return result;
	else
	    return NULL;
}

// extract title from certificate ------------------
char *extractDataFromCert(char *path){

	CertSvcInstance instance;
	if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
		LOGD("CERTSVC_FAIL");
		return NULL;
	}
	CertSvcCertificate cert;
	CertSvcString buffer;
	char *char_buffer;

	certsvc_certificate_new_from_file(instance, path, &cert);

	certsvc_certificate_get_string_field(cert, CERTSVC_SUBJECT_COMMON_NAME, &buffer);
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
	if(firstListElement == NULL)
		return NULL;

	firstListElement->prev = NULL;
	firstListElement->next = NULL;
	firstListElement->isChecked = EINA_FALSE;
	firstListElement->it = NULL;

	return firstListElement;
}

struct ListElement* addListElement(struct ListElement* lastListElement, char name[256]) {
	struct ListElement *newListElement = (struct ListElement*) malloc(sizeof(struct ListElement));
	if (newListElement == NULL)
		return NULL;
	strcpy(newListElement->name, name);
	lastListElement->next = newListElement;
	newListElement->prev = lastListElement;
	newListElement->next = NULL;
	newListElement->isChecked = EINA_FALSE;
	newListElement->it = NULL;
	return newListElement;
}

struct ListElement* addListElementWithTitle(struct ListElement* lastListElement, char name[256], char title[256]) {
	struct ListElement *newListElement = (struct ListElement*) malloc(sizeof(struct ListElement));
	if (newListElement == NULL)
		return NULL;
	strcpy(newListElement->name, name);
	strcpy(newListElement->title, title);
	lastListElement->next = newListElement;
	newListElement->prev = lastListElement;
	newListElement->next = NULL;
	newListElement->isChecked = EINA_FALSE;
	newListElement->it = NULL;
	return newListElement;
}

struct ListElement* addListElementSetUninstallPath(struct ListElement* lastListElement, char uninstall_path[256]) {
	struct ListElement *newListElement = (struct ListElement*) malloc(sizeof(struct ListElement));
	if (newListElement == NULL)
		return NULL;
	strcpy(newListElement->uninstall_path, uninstall_path);
	lastListElement->next = newListElement;
	newListElement->prev = lastListElement;
	newListElement->next = NULL;
	newListElement->it = NULL;
	return newListElement;
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
		free(listElementToRemove);
		return IN_THE_MIDDLE;
	} else if (listElementToRemove->prev != NULL && listElementToRemove->next == NULL) {
		prev = listElementToRemove->prev;
		prev->next = NULL;
		free(listElementToRemove);
		return LAST;
	} else if (listElementToRemove->prev == NULL && listElementToRemove->next != NULL) {
		next = listElementToRemove->next;
		next->prev = NULL;
		free(listElementToRemove);
		return FIRST;
	} else if (listElementToRemove->prev == NULL && listElementToRemove->next == NULL) {
		free(listElementToRemove);
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
	if(listElement == NULL)
		return;
	listElement = findFirstElement(listElement);
	struct ListElement *current, *tmp;
	current = listElement;
	int count = 0;
	while (current != NULL) {
		tmp = current->next;
		free(current);
		current = tmp;
		count++;
	}
	LOGD("Removed %d elements", count);
}
// ------------------------------------------------

void back_cb(void *data, Evas_Object *obj, void *event_info) {
	LOGD("back_cb");
	struct ug_data *ad = (struct ug_data *) data;

	// TODO: Cleanup lists after screen is closed - this code cause some issues
	/*
	if(ad->data_to_clear){
		LOGD("Deleting listElement...");
		deleteList(ad->data_to_clear);
		ad->data_to_clear = NULL;
		LOGD("OK");
	}
	*/
	//free_path(ad);

	ad->list_to_refresh = NULL;
	ad->list_element_to_refresh = NULL;
}

Eina_Bool make_list(struct ug_data *ad, Evas_Object *list, char *dir_path, struct ListElement *lastListElement, enum TypeOfCert type) {

	Eina_Bool no_content_bool = EINA_TRUE;

	/* ---------------------------------------------------
	 * Checking for files (certificates) in PATH
	 */
	DIR *dir;
	struct dirent *dp;

	dir = opendir(dir_path);
	if (dir == NULL) {
		LOGE("There's no such directory: %s", dir_path);
		return no_content_bool;
	}

	if(type != TO_INSTALL && type != TO_UNINSTALL){
		LOGD("Setting params to refresh");
		set_refresh_params(ad, list, lastListElement, dir_path);
	}

	LOGD("Scanning dir (%s) - looking for certs", dir_path);
	while ((dp = readdir(dir)) != NULL) {
		char *tmp;
		tmp = path_cat(dir_path, dp->d_name);
		if (strncmp(dp->d_name, "..", 2) != 0 && strncmp(dp->d_name, ".", 1) != 0) {

			if (!(dp->d_type == DT_DIR)) {

				char *title = NULL;
				title = extractDataFromCert(tmp);
				if (!title) {
					free(tmp);
					tmp = NULL;
					LOGD("Extract data returned NULL pointer");
					continue;
				} else {
					LOGD("title = %s", title);
					no_content_bool = EINA_FALSE;

					Elm_Object_Item * it;
					struct ListElement *current;
					if(type == TO_INSTALL || type == TO_UNINSTALL){
						current = addListElementWithTitle(lastListElement, dp->d_name, title);
					}
					else{
						current = addListElementWithTitle(lastListElement, tmp, title);
					}
					lastListElement = current;

					switch (type) {

					case TO_INSTALL:
						it = elm_list_item_append(list, title, NULL, NULL, install_cb, current);
						break;

					case TO_UNINSTALL: // in this case item isn't append to the list, because "uninstall" use-case uses more complex genlist
						break;

					default:
						it = elm_list_item_append(list, title, NULL, NULL, cert_selection_cb, strdup(tmp));
						break;
					}
					LOGD("elm list append = %s", current->name);
					if (title) {
						free(title);
						title = NULL;
					}
				}
			}

			if (tmp) {
				free(tmp);
				tmp = NULL;
			}
		}
	}
	closedir(dir);

	return no_content_bool;
}

void cert_selection_cb(void *data, Evas_Object *obj, void *event_info) {

	LOGD("cert_selection_cb ----------------------------------------");
	char *path = (char *) data;
	LOGD("cert_path = %s", path);

	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if (it == NULL)
		return;

	struct ug_data *ad = get_ug_data();
	ad->data = (void *) strdup(path);
	LOGD("ad->data  = %s", ad->data);

	elm_list_item_selected_set(it, EINA_FALSE);

	get_info_cert_cb(ad, NULL, NULL);
}

int set_path(struct ug_data *ad, char const *path){
	LOGD("uninstall path = %s", path);
	if (ad->uninstall_path)
		free(ad->uninstall_path);
	ad->uninstall_path = (char *) malloc(sizeof(char) * (strlen(path) + 1));
	strcpy(ad->uninstall_path, path);
	LOGD("uninstall path = %s", ad->uninstall_path);
	return 0;
}

void free_path(struct ug_data *ad){
	if (ad->uninstall_path)
		free(ad->uninstall_path);
	ad->uninstall_path = NULL;
}

void set_refresh_params(struct ug_data *ad, Evas_Object *list, struct ListElement *lastListElement, char dir[MAX_PATH_LENGHT]){
	LOGD("set_refresh_params");
	ad->list_to_refresh = (void *) list;
	ad->list_element_to_refresh = lastListElement;
	strcpy(ad->dir_to_refresh, dir);
}

void refresh_list(struct ug_data *ad) { //TODO: Implement refresh list after uninstall

	LOGD("refresh_list");
	Evas_Object *list = ad->list_to_refresh;
	struct ListElement *lastListElement = ad->list_element_to_refresh;

	if(list == NULL || lastListElement == NULL){
		LOGD("Nothing to refresh");
		if(!list)
			LOGD("list is NULL");
		if(!lastListElement)
			LOGD("lastListElement is NULL");
		return;
	}

	char dir_path[MAX_PATH_LENGHT];
	strcpy(dir_path, ad->dir_to_refresh);
	LOGD("dir path to refresh: %s", ad->dir_to_refresh);

	elm_list_clear(list);
	deleteList(lastListElement);

	lastListElement = initList();
	ad->list_element_to_refresh = lastListElement;

	make_list(ad, list, dir_path, lastListElement, TRUSTED_ROOT);
	LOGD("List refreshed");
}
