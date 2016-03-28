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
 * @file        certificate_util.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 *              Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef __CERT_UTIL_H__
#define __CERT_UTIL_H_

#include <assert.h>
#include <Elementary.h>
#include <Eina.h>
#include <cert-svc/cstring.h>
#include <cert-svc/ccert.h>

#include "certificates/certificates.h"

#ifdef _cplusplus
extern "C" {
#endif

enum TypeOfCert {
    USER,
    TRUSTED_ROOT,
    PKCS12,
    TO_INSTALL,
    TO_UNINSTALL,
};

typedef struct item_data
{
	CertStatus status;
	char *gname;
	CertStoreType storeType;
	size_t index;
	char *title;
} item_data_s;

enum PositionOfElement {
	NONE,           // This is no element of the list
	FIRST,          // First element of list - there is nothing before, but is something after
	LAST,           // Last element of the list - there is nothing after, but is something before
	IN_THE_MIDDLE,  // There are element before and after this element
	THE_LAST_ONE,   // It means that this is the only one element on the list
};

enum CountOfSelected {
	SELECTED_NONE,   // NONE element is selected
	SELECTED_FEW,    // At least one, but not all element are selected
	SELECTED_ALL,    // ALL element are selected
};

Eina_Bool quit_cb(void *data, Elm_Object_Item *it);
void list_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_clicked_cb(void *data, Evas_Object *obj, void *event_info);

struct ListElement* initList();
struct ListElement* nextElement(struct ListElement *listElement);
struct ListElement* addListElement(
		struct ListElement* lastListElement,
		const char *title,
		const char *gname,
		const char *name,
		const char *path);
struct ListElement* findLastElement(struct ListElement* listElement);
struct ListElement* findFirstElement(struct ListElement* listElement);
enum PositionOfElement removeListElement(struct ListElement *listElementToRemove);
void deleteList(struct ListElement* listElement);
void clear_pfx_genlist_data();

/**
 * Path concatenation
 * Merge two string in a correct path, e.g.:
 *
 * char *c1 = "/opt/shared"
 * char *c2 = "file_1.pdf"
 * result -> "/opt/shared/file_1.pdf"
 *
 * or:
 *
 * char *c1 = "/opt/shared/"
 * char *c2 = "file_1.pdf"
 * result -> "/opt/shared/file_1.pdf"
 *
 * (function put '/' char in the end of first string if it's needed)
 *
 * Result is char pointer allocated by malloc()
 * Should be deallocated by free()
 **/
char *path_cat(const char *str1, char *str2);

Eina_Bool back_cb(void *data, Elm_Object_Item *it);
void back_install_cb(void *data, Evas_Object *obj, void *event_info);
void install_button_cb(void *data, Evas_Object *obj, void *event_info);

Evas_Object *create_yes_no_pop(struct ug_data *ad, const char *content);
Evas_Object *create_ok_pop(struct ug_data *ad, const char *content);

Evas_Object *create_no_content_layout (struct ug_data *ad);

Eina_Bool make_list(
		struct ug_data *ad,
		Evas_Object *list,
		const char *dir_path,
		struct ListElement *lastListElement);

int safeStrCmp(const char* s1, const char* s2);

item_data_s *item_data_create(
		const char *gname,
		const char *title,
		CertStatus status,
		CertStoreType storeType,
		size_t index);

void item_data_free(item_data_s *id);

Eina_Bool certStatusToEina(CertStatus status);

#ifdef _cplusplus
}
#endif

//temporary enum definition
#define ELM_TOOLBAR_SHRINK_EXPAND 4

#define BUF256 256

#endif // end of CERT_UTIL_H
