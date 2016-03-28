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
 * @file        pfx_cert_install.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include <dirent.h>

#include <cert-svc/cpkcs12.h>

#include "common-utils.h"
#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static Elm_Genlist_Item_Class itc;

static void install_pfx_button_cb(void *data, Evas_Object *obj, void *event_info);

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *title = (char *)data;

	if (!strcmp(part, "elm.text"))
		return strdup(title);
	return NULL;
}

static void _clear_list(struct ListElement *listElement)
{
	deleteList(listElement);
}

static Eina_Bool _back_cb(void *data, Elm_Object_Item *it)
{
	struct ug_data *ad = (struct ug_data *)data;

	_clear_list(ad->list_element);
	ad->list_element = NULL;
	ad->more_popup2 = NULL;

	return EINA_TRUE;
}

static struct ListElement *scan_dir(const char *dir_path, Evas_Object *list, struct ListElement *lastListElement)
{
	DIR *dir = NULL;
	struct dirent *dp;
	struct dirent entry;
	Elm_Object_Item *it;
	struct ListElement *current;

	itc.item_style = "default";
	itc.func.text_get = _gl_text_get;
	itc.func.del = NULL;

	dir = opendir(dir_path);
	if (dir == NULL) {
		LOGE("There's no such directory: %s", dir_path);
		return NULL;
	}

	LOGD("Scanning dir (%s) - looking for certs", dir_path);

	while (readdir_r(dir, &entry, &dp) == 0  && dp) {
		char *dname = NULL;
		char *dot = strrchr(dp->d_name, '.');

		if (dp->d_type == DT_DIR)
			continue;

		if (dot == NULL || strlen(dot) < 4 ||
			(strncmp(dot, ".pfx", 4)
				&& strncmp(dot, ".p12", 4)
				&& strncmp(dot, ".P12", 4)
				&& strncmp(dot, ".crt", 4)
				&& strncmp(dot, ".CRT", 4)
				&& strncmp(dot, ".pem", 4)
				&& strncmp(dot, ".PEM", 4)))
			continue;


		current = addListElement(lastListElement, NULL, NULL, dp->d_name, dir_path);
		if (current == NULL) {
			LOGE("Null value return from addListElement");
			goto error;
		}

		lastListElement = current;
		dname = strdup(dp->d_name);
		if (dname == NULL) {
			LOGE("Fail to allocate memory");
			goto error;
		}

		if (strncmp(dot, ".crt", 4) == 0 || strncmp(dot, ".pem", 4) == 0)
			it = elm_genlist_item_append(
					list,
					&itc,
					(void *)dname,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					put_pkcs12_name_cb,
					current);
		else
			it = elm_genlist_item_append(
					list,
					&itc,
					(void *)dname,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					install_pfx_button_cb,
					current);

		if (!it)
			LOGE("Error in elm_list_item_append");

		LOGD("elm list append = %s/%s", current->path, current->name);
	}

error:
	if (dir)
		closedir(dir);

	return lastListElement;
}

Elm_Object_Item *pfx_cert_install(struct ug_data *ad)
{
	Evas_Object *list = NULL;

	struct ListElement *firstListElement = initList();
	struct ListElement *lastListElement = firstListElement;

	ad->list_element = lastListElement;
	if (firstListElement == NULL) {
		LOGE("Failed to allocate memory for element list");
		return NULL;
	}

	list = common_genlist(ad->win_main);
	evas_object_smart_callback_add(list, "selected", genlist_clicked_cb, NULL);

	char *path_media = get_media_path();
	char *path_media_downloads = get_media_downloads_path();
	char *path_sdcard = get_sdcard_path();

	lastListElement = scan_dir(path_media, list, lastListElement);
	lastListElement = scan_dir(path_media_downloads, list, lastListElement);
	scan_dir(path_sdcard, list, lastListElement);

	free(path_media);
	free(path_media_downloads);
	free(path_sdcard);

	Elm_Object_Item *navi_it = NULL;

	if (firstListElement->next == NULL) {
		deleteList(firstListElement);

		list = create_no_content_layout(ad);
		if (list == NULL) {
			LOGE("pfx_cert_install: Cannot create no_content layout");
			return NULL;
		}
	}

	navi_it = elm_naviframe_item_push(
			ad->navi_bar,
			"IDS_ST_HEADER_CERTIFICATE_SEARCH_RESULTS_ABB",
			common_back_btn(ad),
			NULL,
			list,
			NULL);

	elm_object_item_domain_text_translatable_set(navi_it, PACKAGE, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(
		navi_it,
		((ad->type_of_screen == PKCS12_SCREEN) ? quit_cb : _back_cb),
		ad);

	LOGD("pfx_cert_install : EXIT");

	return navi_it;
}

static void install_pfx_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();
	struct ListElement *current = (struct ListElement *)data;
	char *path = path_cat(current->path, current->name);
	int returned_value;
	CertSvcString certSvcString_path;

	if (!path) {
		LOGE("path_cat failed. current->path[%s], current->name[%s]", current->path, current->name);
		return;
	}

	LOGD("install_pfx_button_cb: path [%s] to install", path);

	certsvc_string_new(ad->instance, path, strlen(path), &certSvcString_path);

	if (certsvc_pkcs12_has_password(
			ad->instance,
			certSvcString_path,
			&returned_value) != CERTSVC_SUCCESS) {
		LOGE("install_pfx_button_cb: Wrong PKCS12 or PFX file.");
		elm_naviframe_item_pop(ad->navi_bar);
		_clear_list(current);
		goto error;
	}

	switch (returned_value) {
	case CERTSVC_TRUE:
		SECURE_LOGD("%s/%s is passwod protected", current->path, current->name);
		put_pkcs12_name_and_pass(current, ad);
		break;

	case CERTSVC_FALSE:
		SECURE_LOGD("%s/%s is NOT passwod protected", current->path, current->name);
		put_pkcs12_name_cb(current, NULL, NULL);
		break;

	default:
		LOGE("Invalid status returned[%d] from certsvc_pkcs12_has_password", returned_value);
		break;
	}

error:

	free(path);
}
