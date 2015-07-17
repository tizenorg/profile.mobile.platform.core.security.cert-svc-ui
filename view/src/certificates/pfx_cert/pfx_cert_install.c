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
#include <dlog.h>

#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static void install_pfx_button_cb (void *data, Evas_Object *obj, void *event_info);
static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);

static Elm_Genlist_Item_Class itc_2text = {
        .item_style = "1line",
        .func.text_get = _gl_text_get,
        .func.del = NULL
};

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

    char * title = (char *) data;
    if (!strcmp(part, "elm.text.main.left")) {
        return strdup(title);
    }
    return NULL;
}

static void _clear_list (struct ListElement *listElement){
    deleteList(listElement);
}

static Eina_Bool _back_cb(void *data, Elm_Object_Item *it) {
	LOGD("back_cb");

	struct ug_data *ad = (struct ug_data *) data;
	if (ad!=NULL)
	{
		_clear_list(ad->list_element);
		ad->list_element = NULL;
		ad->more_popup2 = NULL;
		data = NULL;
	}
	return EINA_TRUE;   
}

static struct ListElement* scan_dir(const char * dir_path, Evas_Object *list, struct ListElement *lastListElement){

    DIR                *dir = NULL;
    struct dirent      *dp;
    struct dirent      entry;
    Elm_Object_Item    *it;
    struct ListElement *current;

    dir = opendir(dir_path);
    if (dir == NULL) {
        LOGE("There's no such directory: %s", dir_path);
        goto error;
    }
    LOGD("Scanning dir (%s) - looking for certs", dir_path);
    while ( readdir_r(dir, &entry, &dp) == 0  && dp != NULL) {
        char *tmp = NULL;
	    char * dname = NULL;

        tmp = path_cat(dir_path, dp->d_name);
        char *dot = strrchr(dp->d_name, '.');

        if (dot != NULL && strlen(dot) > 3
                && (strncmp(dot, ".pfx", 4) == 0 || strncmp(dot, ".PFX", 4) == 0 || strncmp(dot, ".p12", 4) == 0
                        || strncmp(dot, ".P12", 4) == 0 || strncmp(dot, ".crt", 4) == 0 || strncmp(dot, ".CRT", 4) == 0 || strncmp(dot, ".pem", 4) == 0 || strncmp(dot, ".PEM", 4) == 0)) {
            if (!(dp->d_type == DT_DIR)) {
                current = addListElementWithPath(lastListElement, dp->d_name, dir_path);
                if(current == NULL) {
                    LOGE("Null value return from addListElementWithPath");
                    goto error;
                }
                lastListElement = current;
                dname = (char *) malloc(strlen(dp->d_name)+1);
                if(dname == NULL) {
                    LOGE("Fail to allocate memory");
                    goto error;
                }
                strncpy(dname, dp->d_name, strlen(dp->d_name)+1);

                if(strncmp(dot, ".crt", 4) == 0 || strncmp(dot, ".pem", 4) == 0)
                    it = elm_genlist_item_append(list, &itc_2text, (void *)dname, NULL, ELM_GENLIST_ITEM_NONE, put_pkcs12_name_cb, current);
                else
                    it = elm_genlist_item_append(list, &itc_2text, (void *)dname, NULL, ELM_GENLIST_ITEM_NONE, install_pfx_button_cb, current);

                if (!it){
                    LOGE("Error in elm_list_item_append");
                }
                SECURE_LOGD("elm list append     = %s", current->name);
                SECURE_LOGD("elm list append dir = %s", current->path);
            }

        }
        if (tmp) {
            free(tmp);
            tmp = NULL;
        }
    }

error:
    if(dir != NULL) {
        closedir(dir);
        dir = NULL;
    }

    return lastListElement;
}

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info) {

	evas_object_smart_callback_del(obj,"dismissed", _dismissed_cb);
	evas_object_del(obj);
}

Elm_Object_Item* pfx_cert_install_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_install_cb : IN");

    if(NULL == data){
        return NULL;
    }
    struct ug_data *ad   = (struct ug_data *) data;

    if (ad!=NULL)
    	_dismissed_cb(data, obj, event_info);

    Evas_Object    *list = NULL;

    struct ListElement *firstListElement = initList();
    struct ListElement *lastListElement  = firstListElement;
    ad->list_element = lastListElement;

    list = elm_genlist_add(ad->win_main);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);
    elm_genlist_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", genlist_clicked_cb, NULL);

    lastListElement = scan_dir(PATH_SDCARD, list, lastListElement);
    lastListElement = scan_dir(PATH_MEDIA, list, lastListElement);
    scan_dir(PATH_MEDIA_DOWNLOADS, list, lastListElement);

    Elm_Object_Item *navi_it = NULL;

    if(firstListElement != NULL) {
        if(firstListElement->next) {
            navi_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_HEADER_CERTIFICATE_SEARCH_RESULTS_ABB", NULL, NULL, list, NULL);
        }
        else { //No content
            Evas_Object *no_content = create_no_content_layout(ad);

            if(!no_content){
                LOGD("pfx_cert_install_cb: Cannot create no_content layout (NULL); return");
                return NULL;
            }
            navi_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_HEADER_CERTIFICATE_SEARCH_RESULTS_ABB", NULL, NULL, no_content, NULL);
       }
    }

    elm_object_item_domain_text_translatable_set(navi_it, PACKAGE, EINA_TRUE);

    if (ad->type_of_screen == PKCS12_SCREEN) {
    	elm_naviframe_item_pop_cb_set(navi_it, quit_cb, ad);
    }
    else {
    	elm_naviframe_item_pop_cb_set(navi_it, _back_cb, ad);
    }

    LOGD("pfx_cert_install_cb : EXIT");
	return navi_it;
}

static void install_pfx_button_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("install_pfx_button_cb() :IN ");

    struct ListElement *current = (struct ListElement *) data;
    struct ug_data *ad = get_ug_data();
    char *path = NULL;
    CertSvcString certSvcString_path;

    Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
    if (it){
        elm_list_item_selected_set(it, EINA_FALSE);
    }

    path = path_cat(current->path, current->name);
    if(!path){
        LOGD("path_cat returned NULL; return");
        return;
    }

    LOGD("install_pfx_button_cb: path [%s] to install", path);

    certsvc_string_new(ad->instance, path, strlen(path), &certSvcString_path);

    int returned_value;
    if(certsvc_pkcs12_has_password(ad->instance, certSvcString_path, &returned_value)
            != CERTSVC_SUCCESS){
        LOGD("install_pfx_button_cb: Wrong PKCS12 or PFX file.");
        elm_naviframe_item_pop(ad->navi_bar);
        _clear_list(current);
        free(path);
        return;
    }

    switch (returned_value){
    case CERTSVC_TRUE:
        SECURE_LOGD("%s/%s is passwod protected", current->path, current->name);
        put_pkcs12_name_and_pass_cb(current, NULL, NULL);
        free(path);
        return;

    case CERTSVC_FALSE:
        SECURE_LOGD("%s/%s is NOT passwod protected", current->path, current->name);
        put_pkcs12_name_cb(current, NULL, NULL);
        free(path);
        return;
    }
    free(path);
}
