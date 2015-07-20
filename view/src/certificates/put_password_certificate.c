/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.

 * @file      put_password_certificate.c
 * @author    Janusz Kozerski (j.kozerski@samsung.com)
 *            Kyungwook Tak (k.tak@samsung.com)
 * @version   1.0
 * @brief
 */

#include <dlog.h>
#include <efl_extension.h>

#include <cert-svc/cpkcs12.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static const char *const edj_path = CUSTOM_EDITFIELD_PATH;

static Elm_Genlist_Item_Class itc_group;
static Elm_Genlist_Item_Class itc_entry;
static Elm_Genlist_Item_Class itc_entry_passwd;
static struct ListElement *current_file;
static Evas_Object *_gl_drop_type_content_get(void *data, Evas_Object *obj, const char *part);
static int sel_sub_item_id = 0;
static Elm_Genlist_Item_Class itc_drop;
static Evas_Object *ctxpopup = NULL;

Evas_Object *_entry = NULL;
Evas_Object *_entry_pass = NULL;
Evas_Object *btn = NULL;

static void _get_string_from_entry(Evas_Object *obj, char **out)
{
	if (!out)
		return;

	*out = NULL;

	if (!obj)
		return;

	const char *entry_str = elm_entry_entry_get(obj);
	if (!entry_str)
		return;

	char *out_temp = strdup(entry_str);
	if (out_temp && out_temp[0] == '\0') {
		free(out_temp);
		out_temp = NULL;
	}

	if (out_temp)
		*out = out_temp;
}

static void _cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();

	elm_naviframe_item_pop(ad->navi_bar);
}

static void _install_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	int returned_value;
	int is_unique = CERTSVC_FALSE;

	char *alias = NULL;
	char *password = NULL;
	char *path = NULL;

	CertSvcString Alias;
	CertSvcString Path;
	CertSvcString Password;
	CertSvcString pkcs_alias;

	struct ug_data *ad = get_ug_data();

	if (!ad) {
		LOGE("ug_data can't be null!");
		return;
	}

	_get_string_from_entry(_entry_pass, &password);
	_get_string_from_entry(_entry, &alias);

	if (!alias) {
		create_ok_pop(ad, "IDS_ST_BODY_ALIAS_CANNOT_BE_EMPTY");
		goto exit;
	}

	path = path_cat(current_file->path, current_file->name);
	if (!path) {
		LOGE("patch_cat failed; return");
		goto exit;
	}

	CertStoreType storeType = VPN_STORE;
	if (sel_sub_item_id == 0)
		storeType = VPN_STORE;
	else if (sel_sub_item_id == 1)
		storeType = WIFI_STORE;
	else
		storeType = EMAIL_STORE;

	certsvc_string_new(ad->instance, alias, strlen(alias), &pkcs_alias);
	certsvc_pkcs12_check_alias_exists_in_store(ad->instance, storeType, pkcs_alias, &is_unique);
	if (CERTSVC_FALSE == is_unique) {
		SECURE_LOGD("alias %s already exist", alias);
		create_ok_pop(ad, "IDS_ST_BODY_ALIAS_ALREADY_EXISTS_ENTER_A_UNIQUE_ALIAS");
		goto exit;
	}

	SECURE_LOGD("certsvc_pkcs12_import_from_file(%s, %s)", path, alias);
	certsvc_string_new(ad->instance, alias, strlen(alias), &Alias);
	certsvc_string_new(ad->instance, path, strlen(path), &Path);
	certsvc_string_new(ad->instance, (password) ? password : "", (password) ? strlen(password) : 1, &Password);

	char *dot = strrchr(path, '.');
	if (dot && (!strncasecmp(dot, ".crt", 4) || !strncasecmp(dot, ".pem", 4)))
		Password.privateHandler = password;

	returned_value = certsvc_pkcs12_import_from_file_to_store(ad->instance, storeType, Path, Password, Alias);

	switch (returned_value) {
	case CERTSVC_SUCCESS:
		if (ad->user_cert_list_item)
			elm_naviframe_item_pop_to(ad->user_cert_list_item);
		else if (ad->type_of_screen == PKCS12_SCREEN)
			quit_cb(ad, NULL); //Exit from UG called directly by cert select UG.
		else
			elm_naviframe_item_pop(ad->navi_bar);

		if (ad && ad->refresh_screen_cb)
			ad->refresh_screen_cb(ad, NULL, NULL);

		break;

	case CERTSVC_INVALID_PASSWORD:
		LOGE("Invalid password to %s", current_file->name);
		break;

	case CERTSVC_IO_ERROR:
		LOGE("There's no such file!");
		elm_naviframe_item_pop(ad->navi_bar);
		break;

	case CERTSVC_WRONG_ARGUMENT:
		LOGE("Wrong PKCS12 or PFX file.");
		elm_naviframe_item_pop(ad->navi_bar);
		break;

	case CERTSVC_DUPLICATED_ALIAS:
		// This case should not happened - alias was already checked
		LOGE("Failed. Such alias already exist. This should not happen");
		break;

	case CERTSVC_FAIL:
		LOGE("Failed. Wrong password probably.");
		create_ok_pop(ad, "IDS_ST_POP_FAILED_TO_INSTALL_THE_CERTIFICATE_THE_CERTIFICATE_IS_INVALID_HAS_EXPIRED_OR_YOU_HAVE_ENTERED_AN_INCORRECT_PASSWORD");
		// Do NOT delete list in this case!!!
		goto exit;

	default:
		LOGE("Unknown error code from cert-svc");
		break;
	}

	deleteList(current_file);

exit:
	free(password);
	free(alias);
	free(path);
	return;
}


/* for editfield from here */
static void editfield_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = (Evas_Object *)data;
	elm_object_signal_emit(editfield, "elm,state,focused", "");

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
}

static void editfield_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = (Evas_Object *)data;
	elm_object_signal_emit(editfield, "elm,state,unfocused", "");
	elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

static void editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
	else
		elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

static void editfield_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

static Evas_Object *create_singleline_editfield_layout(Evas_Object *parent)
{
	Evas_Object *editfield;
	Evas_Object *button;

	editfield = elm_layout_add(parent);
	elm_layout_theme_set(editfield, "layout", "editfield", "singleline");
	evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

	_entry = elm_entry_add(editfield);
	elm_entry_single_line_set(_entry, EINA_TRUE);
	elm_entry_scrollable_set(_entry, EINA_TRUE);
	elm_object_part_text_set(_entry, "elm.guide", "Alias");
	evas_object_smart_callback_add(_entry, "focused", editfield_focused_cb, editfield);
	evas_object_smart_callback_add(_entry, "unfocused", editfield_unfocused_cb, editfield);
	evas_object_smart_callback_add(_entry, "changed", editfield_changed_cb, editfield);
	evas_object_smart_callback_add(_entry, "preedit,changed", editfield_changed_cb, editfield);
	elm_object_part_content_set(editfield, "elm.swallow.content", _entry);

	button = elm_button_add(editfield);
	elm_object_style_set(button, "editfield_clear");
	evas_object_smart_callback_add(button, "clicked", editfield_clear_button_clicked_cb, _entry);
	elm_object_part_content_set(editfield, "elm.swallow.button", button);

	return editfield;
}

static Evas_Object *create_password_editfield_layout(Evas_Object *parent)
{
	Evas_Object *editfield;
	Evas_Object *button;

	editfield = elm_layout_add(parent);
	elm_layout_theme_set(editfield, "layout", "editfield", "singleline");
	evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

	_entry_pass = elm_entry_add(editfield);
	elm_entry_single_line_set(_entry_pass, EINA_TRUE);
	elm_entry_scrollable_set(_entry_pass, EINA_TRUE);
	elm_entry_password_set(_entry_pass, EINA_TRUE);
	elm_object_part_text_set(_entry_pass, "elm.guide", "Password");
	evas_object_smart_callback_add(_entry_pass, "focused", editfield_focused_cb, editfield);
	evas_object_smart_callback_add(_entry_pass, "unfocused", editfield_unfocused_cb, editfield);
	evas_object_smart_callback_add(_entry_pass, "changed", editfield_changed_cb, editfield);
	evas_object_smart_callback_add(_entry_pass, "preedit,changed", editfield_changed_cb, editfield);
	elm_object_part_content_set(editfield, "elm.swallow.content", _entry_pass);

	button = elm_button_add(editfield);
	elm_object_style_set(button, "editfield_clear");
	evas_object_smart_callback_add(button, "clicked", editfield_clear_button_clicked_cb, _entry_pass);
	elm_object_part_content_set(editfield, "elm.swallow.button", button);

	return editfield;
}
/* for editfield done */


/* for Elm_Genlist_Item_Class from here */
static Evas_Object *_gl_content_edit_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.entry"))
		return create_singleline_editfield_layout(obj);

	return NULL;
}

static Evas_Object *_gl_content_pass_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.entry"))
		return create_password_editfield_layout(obj);

	return NULL;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info)
		elm_genlist_item_selected_set(event_info, EINA_FALSE);

	struct ug_data *ad = get_ug_data();
	if (ad->popup)
		evas_object_del(ad->popup);

	ad->popup = NULL;
}

static char *_gl_get_text_group(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text.main"))
		return NULL;

	/* text group set only elm.text.main */
	switch ((int)data) {
	case 0:
		LOGD("IDS_ST_BODY_ENTER_PASSWORD_C");
		return strdup(dgettext(PACKAGE, "IDS_ST_BODY_ENTER_PASSWORD_C"));
	case 1:
		LOGD("IDS_ST_BODY_ENTER_KEY_ALIAS_C");
		return strdup(dgettext(PACKAGE, "IDS_ST_BODY_ENTER_KEY_ALIAS_C"));
	case 2:
		LOGD("IDS_ST_HEADER_USED_FOR");
		return strdup(dgettext(PACKAGE, "IDS_ST_HEADER_USED_FOR"));
	default:
		LOGE("Wrong index - return NULL");
		return NULL;
	}

}

static void move_dropdown(Evas_Object *ctxpopup, Evas_Object *btn)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(btn, &x, &y, &w, &h);
	evas_object_move(ctxpopup, x + (w / 2), y + h);
}

static void ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(ctxpopup);
	ctxpopup = NULL;
}

static void ctxpopup_item_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buf[100];
	sel_sub_item_id = data;

	if (sel_sub_item_id == 0)
		snprintf(buf, sizeof(buf), "<align=left>%s</align>", "VPN");
	else if (sel_sub_item_id == 1)
		snprintf(buf, sizeof(buf), "<align=left>%s</align>", "WIFI");
	else
		snprintf(buf, sizeof(buf), "<align=left>%s</align>", "EMAIL");

	elm_object_text_set(btn, buf);
	ctxpopup_dismissed_cb(obj, NULL, NULL);
}

static void btn_dropdown_label_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	if (ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(elm_object_top_widget_get(obj));
	elm_object_style_set(ctxpopup, "dropdown/list");
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup,"dismissed", ctxpopup_dismissed_cb, NULL);

	elm_ctxpopup_item_append(ctxpopup, "VPN", NULL, ctxpopup_item_select_cb, (void *)0);
	elm_ctxpopup_item_append(ctxpopup, "WIFI", NULL, ctxpopup_item_select_cb, (void *)1);
	elm_ctxpopup_item_append(ctxpopup, "EMAIL", NULL, ctxpopup_item_select_cb, (void *)2);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	move_dropdown(ctxpopup, obj);
	evas_object_show(ctxpopup);
}

static Evas_Object *_gl_drop_type_content_get(void *data, Evas_Object *obj, const char *part)
{
	struct ug_data *ad = get_ug_data();
	char buf[100];
	sel_sub_item_id = 0;

	if (strcmp(part, "elm.icon.entry"))
		return NULL;

	Evas_Object *ly = elm_layout_add(obj);
	elm_layout_file_set(ly, edj_path, "eap_dropdown_button");
	btn = elm_button_add(ly);

	snprintf(buf, sizeof(buf), "<align=left>%s</align>", "VPN");

	elm_object_text_set(btn, buf);
	elm_object_style_set(btn, "dropdown/list");
	evas_object_propagate_events_set(btn, EINA_FALSE);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(btn, "clicked", btn_dropdown_label_style_cb, ad->navi_bar);
	elm_layout_content_set(ly, "btn", btn);
	evas_object_show(btn);

	return ly;
}


static void _set_itc_classes()
{
	itc_group.item_style = "groupindex";
	itc_group.func.text_get = _gl_get_text_group;
	itc_entry.func.state_get = NULL;
	itc_entry.func.del = NULL;

	itc_entry.item_style = "entry";
	itc_entry.func.text_get = NULL;
	itc_entry.func.content_get = _gl_content_edit_get;
	itc_entry.func.state_get = NULL;
	itc_entry.func.del = NULL;

	itc_entry_passwd.item_style = "entry";
	itc_entry_passwd.func.text_get = NULL;
	itc_entry_passwd.func.content_get = _gl_content_pass_get;
	itc_entry_passwd.func.state_get = NULL;
	itc_entry_passwd.func.del = NULL;

	itc_drop.item_style = "entry";
	itc_drop.func.text_get = NULL;
	itc_drop.func.content_get = _gl_drop_type_content_get;
	itc_drop.func.state_get = NULL;
	itc_drop.func.del = NULL;
}
/* for Elm_Genlist_Item_Class done */


static Evas_Object *_create_title_text_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn)
		return NULL;

	elm_object_domain_translatable_text_set(btn, PACKAGE, text);
	evas_object_smart_callback_add(btn, "clicked", func, data);

	return btn;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

/* CALLBACK */
void put_pkcs12_name_and_pass_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();
	ad->popup = NULL;
	current_file = (struct ListElement *) data;

	Evas_Object *genlist = elm_genlist_add(ad->navi_bar);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	_set_itc_classes();

	Elm_Object_Item *passwd_entry_label = elm_genlist_item_append(
			genlist,
			&itc_group,
			(void *) 0,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );
	elm_genlist_item_select_mode_set(passwd_entry_label, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	elm_genlist_item_append(
			genlist,
			&itc_entry_passwd,
			NULL,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );

	Elm_Object_Item *name_entry_label = elm_genlist_item_append(
			genlist,
			&itc_group,
			(void *) 1,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );
	elm_genlist_item_select_mode_set(name_entry_label, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	elm_genlist_item_append(
			genlist,
			&itc_entry,
			(void *) 0,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );

	Elm_Object_Item *ctxpopup_entry_label = elm_genlist_item_append(
			genlist,
			&itc_group,
			(void *) 2,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );
	elm_genlist_item_select_mode_set(ctxpopup_entry_label, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	elm_genlist_item_append(
			genlist,
			&itc_drop,
			NULL,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );

	evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(
			ad->navi_bar,
			"IDS_ST_HEADER_INSTALL_CERTIFICATE_ABB2",
			NULL,
			NULL,
			genlist,
			NULL);

	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);

	if (!nf_it)
		LOGE("Error in elm_naviframe_item_push");

	/* Title Cancel Button */
	Evas_Object *right_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(right_btn, "naviframe/title_done");
	evas_object_smart_callback_add(right_btn, "clicked", _install_button_cb, ad->navi_bar);
	elm_object_item_part_content_set(nf_it, "title_right_btn", right_btn);

	/* Title Done Button */
	Evas_Object *left_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(left_btn, "naviframe/title_cancel");
	evas_object_smart_callback_add(left_btn, "clicked", _cancel_button_cb, ad->navi_bar);
	elm_object_item_part_content_set(nf_it, "title_left_btn", left_btn);
}

/* CALLBACK */
void put_pkcs12_name_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();
	ad->popup = NULL;
	current_file = (struct ListElement *) data;

	Evas_Object *genlist = elm_genlist_add(ad->navi_bar);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	_set_itc_classes();

	Elm_Object_Item *name_entry_label = elm_genlist_item_append(
				genlist,
				&itc_group,
				(void *) 1,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL );
	elm_genlist_item_select_mode_set(name_entry_label, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	elm_genlist_item_append(
				genlist,
				&itc_entry,
				(void *) 1,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL );
	Elm_Object_Item *ctxpopup_entry_label = elm_genlist_item_append (
			genlist,
			&itc_group,
			(void *) 2,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );
	elm_genlist_item_select_mode_set(ctxpopup_entry_label, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	elm_genlist_item_append(
			genlist,
			&itc_drop,
			NULL,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			_gl_sel,
			NULL );

	evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(
			ad->navi_bar,
			"IDS_ST_HEADER_INSTALL_CERTIFICATE_ABB2",
			NULL,
			NULL,
			genlist,
			NULL);

	if (!nf_it)
		LOGE("Error in elm_naviframe_item_push");

	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);

	Evas_Object *right_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(right_btn, "naviframe/title_done");
	evas_object_smart_callback_add(right_btn, "clicked", _install_button_cb, NULL);
	elm_object_item_part_content_set(nf_it, "title_right_btn", right_btn);

	Evas_Object *left_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(left_btn, "naviframe/title_cancel");
	evas_object_smart_callback_add(left_btn, "clicked", _cancel_button_cb, NULL);
	elm_object_item_part_content_set(nf_it, "title_left_btn", left_btn);
}
