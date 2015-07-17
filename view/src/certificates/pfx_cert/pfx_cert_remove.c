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

#include <dirent.h>

#include <dlog.h>
#include <efl_extension.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static CertSvcStringList stringList;
static Eina_Bool *state_pointer = NULL; //check states
static int list_length = 0;
static Evas_Object *genlist = NULL;
static int checked_count = 0;
static Elm_Object_Item *select_all_btn_item = NULL;
Eina_Bool g_state = EINA_FALSE;
Evas_Object *done = NULL;
static CertSvcStoreCertList *CertList = NULL;

static Evas_Object *_create_title_text_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn)
		return NULL;

	elm_object_domain_translatable_text_set(btn, PACKAGE, text);
	evas_object_smart_callback_add(btn, "clicked", func, data);

	return btn;
}

static void _pfx_cert_remove_cleanup()
{
	if (state_pointer) {
		free(state_pointer);
		state_pointer = NULL;
	}
}

static void _navi_text_update()
{
	const char *pText = dgettext(PACKAGE, "IDS_ST_HEADER_PD_SELECTED_ABB");
	if (!pText)
		return;

	size_t size = strlen(pText) + 30; //30 for itoa value of checked_count, should work for 64 bit int
	char *formatedText = (char *)malloc(sizeof(char) * size);
	if (!formatedText) {
		LOGE("Failed to allocate memory");
		return;
	}

	snprintf(formatedText, size, pText, checked_count); //pText must contain %d
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
	Elm_Object_Item *item = ei;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.icon.right");
	if(!ck)
		return;

	elm_check_state_set(ck, !elm_check_state_get(ck));
	_chk_changed_cb(data, ck, NULL);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = (item_data_s *) data;
	int index = id->index;

	if (strcmp(part, "elm.icon.right"))
		return NULL;

	Evas_Object *check = elm_check_add(obj);
	elm_object_style_set(check, "default/genlist");

	//set the State pointer to keep the current UI state of Checkbox.
	elm_check_state_pointer_set(check, &(state_pointer[index]));

	// Repeat events to below object (genlist)
	// So that if check is clicked, genlist can be clicked.
	evas_object_repeat_events_set(check, EINA_FALSE);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(check, "changed", _chk_changed_cb, index);

	return check;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

static char* _gl_get_text_sa(void *data, Evas_Object *obj, const char *part)
{
    return strdup(dgettext(PACKAGE, "IDS_ST_BODY_SELECT_ALL"));
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	item_data_s *id = (item_data_s *)data;

	if (!strcmp(part, "elm.text.main.left.top"))
		return strdup(id->title);

	if (strcmp(part, "elm.text.sub.left.bottom"))
		return NULL;

	if(id->storeType == VPN_STORE)
		return strdup("Store Type: VPN");

	if(id->storeType == WIFI_STORE)
		return strdup("Store Type: WIFI");

	return strdup("Store Type: EMAIL");
}

static void genlist_pfx_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
    (void)obj;
    (void)event_info;
	int i = 0;
	int ret = -1;

    if (!data || !state_pointer)
		return;

    struct ug_data *ad = (struct ug_data *)data;
    if (ad) {
		ad->more_popup2 = NULL;
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	CertSvcString FileName;
	CertSvcInstance instance;
	CertSvcStoreCertList* certListHead = CertList;
	if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
		LOGE("CERTSVC_FAIL to create instance.");
		return;
    }

	while (CertList) {
		if (EINA_TRUE == state_pointer[i]) {
			FileName.privateHandler = strdup(CertList->gname);
            if (!FileName.privateHandler) {
				LOGE("Failed to allocate memory");
				certsvc_instance_free(instance);
				return;
			}

			FileName.privateLength = strlen(FileName.privateHandler);

		    ret = certsvc_pkcs12_delete_certificate_from_store(instance, CertList->storeType, FileName);
		    if (ret != CERTSVC_SUCCESS) {
				LOGE("Fail to delete selected certificate");
				certsvc_instance_free(instance);
				free(FileName.privateHandler);
				return;
			}

		    free(FileName.privateHandler);
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
}

static void _popup_quit_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	struct ug_data *ad = (struct ug_data *)data;
	evas_object_del(ad->popup);
	ad->popup = NULL;

}

static void genlist_pfx_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
    const char *pText;
    char *formatedText;
    size_t size;

    if (!data)
       return;

    struct ug_data *ad = (struct ug_data *)data;

	ad->popup = elm_popup_add(ad->navi_bar);
	if (!ad->popup)
		return;

	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	//no button
	Evas_Object *btn_no = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_no, PACKAGE, "IDS_ST_BUTTON_CANCEL");
	evas_object_smart_callback_add(btn_no, "clicked", _popup_quit_cb, ad);
	elm_object_style_set(btn_no, "popup");

	//yes button
	Evas_Object *btn_yes = elm_button_add(ad->popup);
	elm_object_domain_translatable_text_set(btn_yes, PACKAGE, "IDS_ST_BUTTON_UNINSTALL");
	evas_object_smart_callback_add(btn_yes, "clicked", genlist_pfx_delete_cb, ad);
	elm_object_style_set(btn_yes, "popup");
	eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _popup_quit_cb, ad);

	if (checked_count > 1) {
		pText = dgettext(PACKAGE, "IDS_ST_POP_PD_CERTIFICATES_WILL_BE_UNINSTALLED");
		size = strlen(pText) + 30; //30 for itoa value of checked_count, should work for 64 bit int
		formatedText = malloc(size);
        if (formatedText) {
		    snprintf(formatedText, size, pText, checked_count); //pText must contain %d
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
	(void)obj;
	(void)event_info;

	if (!data)
		return;

	struct ug_data *ad = (struct ug_data *)data;
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
    int i;

	int Length = 0, result = 0;

	CertSvcStoreCertList* certList = NULL;
	CertSvcInstance instance;

	if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
		LOGD("CERTSVC_FAIL to create instance.");
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

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    if (!itc) {
        LOGE("Fail to malloc genlist item class");
        certsvc_instance_free(instance);
        return;
    }

	itc->item_style = "2line.top";
	itc->func.content_get = _gl_content_get;
	itc->func.text_get = _gl_text_get;
	
	if (Length) {
		state_pointer = malloc((Length+1) * sizeof(Eina_Bool));
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
            elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, i);
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

static Evas_Object *create_2_text_with_title_tabbar(Evas_Object *parent)
{
	Evas_Object *toolbar;
	toolbar = elm_toolbar_add(parent);
	elm_object_style_set(toolbar, "tabbar_with_title");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);

	elm_toolbar_item_append(toolbar, NULL, "VPN", VPN_list_cb, parent);
	elm_toolbar_item_append(toolbar, NULL, "WIFI", WIFI_list_cb, parent);
	elm_toolbar_item_append(toolbar, NULL, "EMAIL", EMAIL_list_cb, parent);

	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
	return toolbar;
}

static Evas_Object *_create_genlist(struct ug_data *ad, Evas_Object *parent)
{
	Evas_Object *tabbar;

    Evas_Object *box = NULL;
    Elm_Object_Item *cancel_button;
    checked_count = 0;

    box = elm_box_add(ad->navi_bar);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	genlist = elm_genlist_add(box);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	evas_object_show(box);

    Elm_Object_Item *itm = elm_naviframe_item_push(
            ad->navi_bar,
            "IDS_ST_HEADER_SELECT_ITEMS",
            NULL,
            NULL,
            box,
            NULL);
    select_all_btn_item = itm;

    elm_object_item_domain_text_translatable_set(itm, PACKAGE, EINA_TRUE);
    elm_naviframe_item_pop_cb_set(itm, genlist_pfx_back_cb, ad);

	elm_naviframe_item_style_set(itm, "tabbar");
	tabbar = create_2_text_with_title_tabbar(ad->win_main);
	elm_object_item_part_content_set(itm, "tabbar", tabbar);

	done = elm_button_add(ad->navi_bar);
	elm_object_style_set(done, "naviframe/title_done");
	evas_object_smart_callback_add(done, "clicked", genlist_pfx_popup_cb, ad);
	elm_object_item_part_content_set(itm, "title_right_btn", done);
	elm_object_disabled_set(done, EINA_TRUE);

	/* Title Cancel Button */
	cancel_button = elm_button_add(ad->navi_bar);
	elm_object_style_set(cancel_button, "naviframe/title_cancel");
	evas_object_smart_callback_add(cancel_button, "clicked", genlist_pfx_cancel_cb, ad);
	elm_object_item_part_content_set(itm, "title_left_btn", cancel_button);


	return genlist;
}

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_del(obj,"dismissed", _dismissed_cb);
	evas_object_del(obj);
}

void pfx_cert_remove_cb(void *data, Evas_Object *obj, void *event_info)
{
    struct ug_data *ad = (struct ug_data *) data;

    _dismissed_cb(data, obj, event_info);

	evas_object_hide(ad->more_popup2);
    checked_count = 0;

	_create_genlist(ad,NULL);
}
