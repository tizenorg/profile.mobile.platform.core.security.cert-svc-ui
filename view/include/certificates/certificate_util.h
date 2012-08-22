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

#ifndef __CERT_UTIL_H__
#define __CERT_UTIL_H_

#include <Elementary.h>
#include <assert.h>
#include <dlog.h>

#include "certificates/certificates.h"

#ifdef _cplusplus
extern "C" {
#endif

#define CERT_MAX_DATA_SIZE  256
#define MAX_PATH_LENGHT     512
#define NO_INFORMATION      "no information"

enum TypeOfCert{
	USER,
	TRUSTED_ROOT,
	TO_INSTALL,
	TO_UNINSTALL,
};

struct ListElement {
	struct ListElement *prev, *next;
	char name[CERT_MAX_DATA_SIZE];
	char title[CERT_MAX_DATA_SIZE];
	char uninstall_path[MAX_PATH_LENGHT];
	Eina_Bool isChecked;
	Elm_Object_Item * it;
};

enum PositionOfElement {
	NONE,			// This is no element of the list
	FIRST, 			// First element of list - there is nothing before, but is something after
	LAST, 			// Last element of the list - there is nothing after, but is something before
	IN_THE_MIDDLE, 	// There are element before and after this element
	THE_LAST_ONE, 	// It means that this is the only one element on the list
};

void list_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_clicked_cb(void *data, Evas_Object *obj, void *event_info);
Evas_Object *create_info_pop(const char *content);

struct ListElement* nextElement(struct ListElement *listElement);
struct ListElement* initList();
struct ListElement* addListElement(struct ListElement* lastListElement, char name[256]);
struct ListElement* addListElementWithTitle(struct ListElement* lastListElement, char name[256], char title[256]);
struct ListElement* addListElementSetUninstallPath(struct ListElement* lastListElement, char uninstall_path[256]);
struct ListElement* findLastElement(struct ListElement* listElement);
struct ListElement* findFirstElement(struct ListElement* listElement);
enum PositionOfElement removeListElement(struct ListElement *listElementToRemove);
void deleteList(struct ListElement* listElement);

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
char *extractDataFromCert(char *path);

struct ListElement *initList();
struct ListElement *addListElement(struct ListElement* lastListElement, char name[256]);
struct ListElement *addListElementWithTitle(struct ListElement* lastListElement, char name[256], char title[256]);
enum PositionOfElement removeListElement(struct ListElement *listElementToRemove);
struct ListElement *findLastElement(struct ListElement* listElement);
struct ListElement *findFirstElement(struct ListElement* listElement);
void cert_selection_cb(void *data, Evas_Object *obj, void *event_info);
void deleteList(struct ListElement* listElement);
void back_cb(void *data, Evas_Object *obj, void *event_info);
Eina_Bool make_list(struct ug_data *ad, Evas_Object *list, char *dir_path, struct ListElement *lastListElement, enum TypeOfCert type);
int set_path(struct ug_data *ad, char const *path);
void free_path(struct ug_data *ad);
void set_refresh_params(struct ug_data *ad, Evas_Object *list, struct ListElement *lastListElement, char dir[MAX_PATH_LENGHT]);
void refresh_list(struct ug_data *ad);
Evas_Object *create_yes_no_pop(struct ug_data *ad, const char *content);
Evas_Object *create_ok_pop(struct ug_data *ad, const char *content);

#ifdef _cplusplus
}
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif

#ifndef LOG_TAG
    #define LOG_TAG "CERT-SVC-UI"
#endif

//temporary enum definition
#define ELM_TOOLBAR_SHRINK_EXPAND 4

#define BUF256 256
#define BUF1024 1024

#endif // end of CERT_UTIL_H


