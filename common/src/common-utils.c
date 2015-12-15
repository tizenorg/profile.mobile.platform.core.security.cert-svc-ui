/**
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        common-utils.c
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief
 */

#include <efl_extension.h>

#include "common-utils.h"

static void _back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = (struct ug_data *)data;
	elm_naviframe_item_pop(ad->navi_bar);
}

static void _lang_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_realized_items_update(obj);
}

Evas_Object *common_genlist(Evas_Object *parent)
{
	Evas_Object *genlist = elm_genlist_add(parent);

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "language,changed", _lang_changed_cb, NULL);

	return genlist;
}

Evas_Object *common_back_btn(struct ug_data *ad)
{
	Evas_Object *btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(btn, "clicked", _back_btn_cb, ad);

	return btn;
}

Evas_Object *add_common_done_btn(struct ug_data *ad, E_CB done_cb)
{
	Evas_Object *btn = elm_button_add(ad->navi_bar);
	Elm_Object_Item *nf_it = elm_naviframe_top_item_get(ad->navi_bar);

	elm_object_style_set(btn, "naviframe/title_done");
	evas_object_smart_callback_add(btn, "clicked", done_cb, ad);
	elm_object_item_part_content_set(nf_it, "title_right_btn", btn);

	return btn;
}

Evas_Object *add_common_cancel_btn(struct ug_data *ad, E_CB cancel_cb)
{
	Evas_Object *btn = elm_button_add(ad->navi_bar);
	Elm_Object_Item *nf_it = elm_naviframe_top_item_get(ad->navi_bar);

	elm_object_style_set(btn, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn, "clicked", cancel_cb, ad);
	elm_object_item_part_content_set(nf_it, "title_left_btn", btn);

	return btn;
}

void common_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ad = get_ug_data();

	evas_object_smart_callback_del((Evas_Object *)data, "rotation,changed", common_dismissed_cb);
	evas_object_smart_callback_del(obj, "dismissed", common_dismissed_cb);
	evas_object_del(obj);

	ad->more_popup2 = NULL;
}

void common_rotate_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	int pos = -1;
	Evas_Coord w;
	Evas_Coord h;
	Evas_Object *popup = (Evas_Object *)data;
	Evas_Object *win = (Evas_Object *)obj;

	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);

	switch (pos) {
	case 0:
	case 180:
		evas_object_move(popup, (w / 2), h);
		return;
	case 90:
	case 270:
		evas_object_move(popup, (h / 2), w);
		return;
	default:
		LOGE("Invalid pos value[%d]", pos);
		return;
	}
}

Evas_Object *common_more_ctxpopup(Evas_Object *parent)
{
	Evas_Object *popup = elm_ctxpopup_add(parent);
	Evas_Object *win = elm_object_top_widget_get(popup);

	elm_ctxpopup_auto_hide_disabled_set(popup, EINA_TRUE);
	elm_object_style_set(popup, "more/default");

	evas_object_smart_callback_add(win, "rotation,changed", common_rotate_popup_cb, popup);
	evas_object_smart_callback_add(popup, "dismissed", common_dismissed_cb, win);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

	elm_ctxpopup_direction_priority_set(
		popup,
		ELM_CTXPOPUP_DIRECTION_UP,
		ELM_CTXPOPUP_DIRECTION_UNKNOWN,
		ELM_CTXPOPUP_DIRECTION_UNKNOWN,
		ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	common_rotate_popup_cb(popup, win, NULL);

	return popup;
}
