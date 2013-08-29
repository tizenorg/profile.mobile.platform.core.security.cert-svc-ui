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
 * @version     1.0
 * @brief
 */

#include <dlog.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

#include <dirent.h>

static CertSvcStringList stringList;
static char **alias_list = NULL;
static char **email_list = NULL;
static int  max_length   = 0;

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);

void clear_pfx_genlist_data(){

    LOGD("clear_pfx_genlist_data()");
    int i;
    if(1 > max_length){
        alias_list = NULL;
        email_list = NULL;
        return;
    }
    if (NULL != alias_list) {
        for (i = 0; i < max_length; ++i) {
            free (alias_list[i]);
        }
        free(alias_list);
        alias_list = NULL;
    }

    if (NULL != email_list) {
        for (i = 0; i < max_length; ++i) {
            free (email_list[i]);
        }
        free(email_list);
        email_list = NULL;
    }
    max_length = 0;

    certsvc_string_list_free(stringList);
}

static Elm_Genlist_Item_Class itc_2text = {
        .item_style = "2text.2",
        .func.text_get = _gl_text_get,
        .func.del = NULL
};

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;
    if(max_length <= index || 0 > index){
        return NULL;
    }
    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        return strdup(alias_list[index]);
    } else if (!strcmp(part, "elm.text.2")) {
        return strdup(email_list[index]);
    }
    return NULL;
}

static Eina_Bool _back_pfx_cb(void *data, Elm_Object_Item *it) {
    clear_pfx_genlist_data();
	return EINA_TRUE;
}

static void pfx_selection_cb(void *data, Evas_Object *obj, void *event_info) {

    LOGD("pfx_selection_cb()");

    int index = (int) data;
    struct ug_data *ad = get_ug_data();

    Elm_Object_Item *it = (Elm_Object_Item *) elm_genlist_selected_item_get(obj);
    if (NULL == it){
        LOGD("it is NULL; return;");
        return;
    }
    elm_genlist_item_selected_set(it, EINA_FALSE);

    int                    certListlength = 0;
    int                    returned;
    CertSvcCertificateList certList;
    CertSvcCertificate     cert;
    CertSvcString          buffer;

    if (CERTSVC_SUCCESS != certsvc_string_list_get_one(stringList, index, &buffer)) {
        LOGD("certsvc_string_list_get_one returned not CERTSVC_SUCCESS");
        return;
    }
    SECURE_LOGD("alias == %s", buffer);

    returned = certsvc_pkcs12_load_certificate_list(ad->instance, buffer, &certList);
    if(CERTSVC_SUCCESS == returned) {
        LOGD("certsvc_pkcs12_load_certificate_list returned CERTSVC_SUCCESS");
    }
    if (CERTSVC_BAD_ALLOC == returned) {
        LOGD("certsvc_pkcs12_load_certificate_list returned CERTSVC_BAD_ALLOC");
        return;
    }
    if (CERTSVC_FAIL == returned) {
        LOGD("certsvc_pkcs12_load_certificate_list returned CERTSVC_FAIL");
        return;
    }
    if (CERTSVC_IO_ERROR == returned) {
        LOGD("certsvc_pkcs12_load_certificate_list returned CERTSVC_IO_ERROR");
        return;
    }

    returned = certsvc_certificate_list_get_length(certList, &certListlength);
    if(CERTSVC_SUCCESS == returned) {
        LOGD("certsvc_certificate_list_get_length returned CERTSVC_SUCCESS");
    }
    if(CERTSVC_WRONG_ARGUMENT == returned) {
        LOGD("certsvc_certificate_list_get_length returned CERTSVC_WRONG_ARGUMENT");
        return;
    }

    LOGD("Certificate List Length = %d", certListlength);
    if(certListlength<1){
        return;
    }

    if(CERTSVC_SUCCESS != certsvc_certificate_list_get_one(certList, 0, &cert)) {
        LOGD("certsvc_certificate_list_get_one returned not CERTSVC_SUCCESS");
        return;
    }

    get_info_cert_from_certificate_cb(cert);
}

void pfx_cert_create_list(struct ug_data *ad){
    LOGD("pfx_cert_create_list()");

    int i;
    int list_length = 0;
    Evas_Object *no_content = NULL;
    Evas_Object *genlist_pfx = NULL;

    clear_pfx_genlist_data();

    certsvc_pkcs12_get_id_list(ad->instance, &stringList);
    certsvc_string_list_get_length(stringList, &list_length);

    //With Refresh logic: 1. Make new layouts on Main screen or set part content
    if (1 > list_length) { // No content
		no_content = create_no_content_layout(ad);
		if(!no_content){
			LOGD("Cannot create no_content layout (NULL); return");
			return;
		}

    	if (ad->user_cert_list_item) {
    		elm_object_item_part_content_set(ad->user_cert_list_item, NULL , no_content); //deletes the previous object set.
    	}
    	else {
    		ad->user_cert_list_item = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_USER_CERTIFICATES", NULL, NULL, no_content, NULL);
    	}

    	elm_object_item_disabled_set(ad->uninstall_button, EINA_TRUE);

    }
    else {

    	CertSvcString buffer;
    	const char *char_buffer;

    	genlist_pfx = elm_genlist_add(ad->win_main);
        if (!genlist_pfx)
        	return;

        evas_object_smart_callback_add(genlist_pfx, "language,changed", _gl_lang_changed, NULL);

		max_length = list_length;
		if(list_length > 0 ){
			alias_list = malloc(list_length * sizeof(char *));
			email_list = malloc(list_length * sizeof(char *));
		}

		for(i = 0; i < list_length; i++) {
			if (CERTSVC_SUCCESS != certsvc_string_list_get_one(stringList, i, &buffer)) {
				alias_list[i] = strdup("ERROR WHILE LOADING STRING");
				email_list[i] = strdup("");
				continue;
			}
			certsvc_string_to_cstring(buffer, &char_buffer, NULL);
			alias_list[i] = strdup(char_buffer);
			email_list[i] = strdup(get_email(buffer));
			LOGD("%s, %s", alias_list[i], email_list[i]);
		}

		for (i = 0; i < list_length; i++) {
			LOGD("elm_genlist_item_append: %d: %s, %s", i, alias_list[i], email_list[i]);
			elm_genlist_item_append(genlist_pfx, &itc_2text, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, pfx_selection_cb, (void*) i);
		}

        if (ad->user_cert_list_item) {
        	elm_object_item_part_content_set(ad->user_cert_list_item, NULL , genlist_pfx); //deletes the previous object set.
        }
        else {
        	ad->user_cert_list_item = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_USER_CERTIFICATES", NULL, NULL, genlist_pfx, NULL);
        }

    	elm_object_item_disabled_set(ad->uninstall_button, EINA_FALSE);
    }

    if (ad->user_cert_list_item != NULL) {
     	elm_object_item_domain_text_translatable_set(ad->user_cert_list_item, PACKAGE, EINA_TRUE);
    }
}

void refresh_pfx_cert_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *) data;
	if (!ad)
		return;

	pfx_cert_create_list(ad);
}

void pfx_cert_cb(void *data, Evas_Object *obj, void *event_info) {
    LOGD("pfx_cert_cb()");

    Evas_Object* toolbar;
    Elm_Object_Item* install_button;
    struct ug_data *ad = (struct ug_data *) data;
    if (!ad)
    	return;

    ad->user_cert_list_item = NULL;

    toolbar = elm_toolbar_add(ad->navi_bar);
    if (!toolbar)
        return;
    elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
    elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
    elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

    install_button = elm_toolbar_item_append(toolbar, NULL, "IDS_ST_BUTTON_INSTALL", install_button_cb, ad);
    if (!install_button)
        return;
    elm_object_item_domain_text_translatable_set(install_button, PACKAGE, EINA_TRUE);
    
    ad->uninstall_button = elm_toolbar_item_append(toolbar, NULL, "IDS_ST_BUTTON_UNINSTALL", pfx_cert_remove_cb, ad);
    if (!ad->uninstall_button)
    	return;
    elm_object_item_domain_text_translatable_set(ad->uninstall_button, PACKAGE, EINA_TRUE);

    pfx_cert_create_list(ad);

    if (!ad->user_cert_list_item)
    	return;

    elm_object_item_part_content_set(ad->user_cert_list_item, "toolbar", toolbar);

    elm_naviframe_item_pop_cb_set(ad->user_cert_list_item, _back_pfx_cb, NULL);

    ad->refresh_screen_cb = refresh_pfx_cert_cb;

}
