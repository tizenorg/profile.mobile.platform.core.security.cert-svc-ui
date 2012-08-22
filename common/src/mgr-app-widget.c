/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved 
 * 
 * This file is part of the Manage Applications
 * Written by Eunmi Son <eunmi.son@samsung.com>
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


#include "mgr-app-widget.h"
#include "mgr-app-common-debug.h"
#include "mgr-app-common-util.h"
#include "mgr-app-view-manager.h"

typedef struct _Custom_Effect Custom_Effect;

struct _size {
	Evas_Coord w;
	Evas_Coord h;
};

struct _Custom_Effect {
	struct _size from;
	struct _size to;
};

static Elm_Transit_Effect *__custom_context_new(Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
	MGR_APP_BEGIN();

	Custom_Effect *custom_effect = NULL;

	custom_effect = calloc(1, sizeof(Custom_Effect));
	if (!custom_effect) {
   		return NULL;
   	}

   	custom_effect->from.w = from_w;
   	custom_effect->from.h = from_h;
   	custom_effect->to.w = to_w - from_w;
   	custom_effect->to.h = to_h - from_h;

	MGR_APP_END();
   	return custom_effect;
}

static void _custom_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
	MGR_APP_BEGIN();

   	if (!effect) {
   		return;
   	}
   	
   	Evas_Coord w = 0;
   	Evas_Coord h = 0;
   	Evas_Object *obj = NULL;
   	const Eina_List *elist = NULL;

   	Custom_Effect *custom_effect = (Custom_Effect *)effect;
   	const Eina_List *objs = elm_transit_objects_get(transit);

	if (progress < 0.5) {
		h = custom_effect->from.h + (custom_effect->to.h * progress * 2);
		w = custom_effect->from.w;
    } else {
        h = custom_effect->from.h + custom_effect->to.h;
        w = custom_effect->from.w + (custom_effect->to.w * (progress - 0.5) * 2);
	}

   	EINA_LIST_FOREACH(objs, elist, obj)
	evas_object_resize(obj, w, h);

	MGR_APP_END();
}

static void _custom_context_free(Elm_Transit_Effect *effect)
{
	MGR_APP_BEGIN();

   	Custom_Effect *custom_effect = (Custom_Effect *)effect;
   	MGR_APP_MEM_FREE(custom_effect);

   	MGR_APP_END();
}

Evas_Object *mgr_app_view_create_base_navigation(Evas_Object *parent)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *navibar = NULL;

	navibar = elm_naviframe_add(parent);
	elm_object_part_content_set(parent, "elm.swallow.content", navibar);

	evas_object_show(navibar);

	MGR_APP_END();
	return navibar;
}

Evas_Object *mgr_app_widget_create_bg(Evas_Object *parent)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *bg = NULL;

	bg = elm_bg_add(parent);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(parent, bg);
    evas_object_show(bg);
	
	MGR_APP_END();
	return bg;
}

Evas_Object *mgr_app_widget_create_main_layout(Evas_Object *parent)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, layout);
	evas_object_show(layout);

	MGR_APP_END();
	return layout;
}

Evas_Object *mgr_app_create_layout(Evas_Object *parent, const char *clas, const char *group, const char *style)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *ly = NULL;

	ly = elm_layout_add(parent);
	elm_layout_theme_set(ly, clas, group, style);
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ly);
	MGR_APP_END();

	return ly;
}

Evas_Object *mgr_app_create_layout_file(Evas_Object *parent, const char *filename, const char *group)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *ly = NULL;

	ly = elm_layout_add(parent);
	elm_layout_file_set(ly, filename, group);
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ly);
	MGR_APP_END();

	return ly;
}

Evas_Object *mgr_app_create_scroller(Evas_Object *parent)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *scroller = NULL;

	scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
	elm_object_part_content_set(parent, "elm.swallow.content", scroller);
	MGR_APP_END();
	return scroller;
}

Evas_Object *mgr_app_create_box(Evas_Object *parent, Eina_Bool is_hori)
{
	MGR_APP_BEGIN();

	retv_if(parent == NULL, NULL);

	Evas_Object *box = NULL;

	box = elm_box_add(parent);
	evas_object_size_hint_weight_set( box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set( box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);
	MGR_APP_END();
	return box;
}

Evas_Object *mgr_app_create_button(Evas_Object *parent, void (*func) (void *data, Evas_Object *obj, void *event_info), const char *label, const char *swallow, const char *style, void *data)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *button = NULL;

	button = elm_button_add(parent);
	elm_object_part_content_set(parent, swallow, button);
	elm_object_style_set(button, style);
	elm_object_text_set(button, label);
	evas_object_show(button);
	evas_object_smart_callback_add(button, "clicked", func, (void*)data);
	MGR_APP_END();
	return button;
}

void mgr_app_box_pack_end_separator(Evas_Object *box, const char *style)
{
	MGR_APP_BEGIN();
	ret_if(box == NULL);

	Evas_Object *sp = NULL;
	sp = elm_separator_add(box);
	elm_separator_horizontal_set(sp, EINA_TRUE);
	elm_object_style_set(sp, style);
	evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0);
	evas_object_show(sp);
	elm_box_pack_end(box, sp);
	MGR_APP_END();
}

Evas_Object *mgr_app_create_label(Evas_Object *parent, const char *text, const char *swallow, void *data)
{
	MGR_APP_BEGIN();
	retv_if(parent == NULL, NULL);

	Evas_Object *label = NULL;

	label = elm_label_add(parent);
	elm_object_part_content_set(parent, swallow, label);
	elm_object_text_set(label, text);
	MGR_APP_END();

	return label;
}

void mgr_app_set_transit_effect(Evas_Object *parent, int effect_type)
{
	MGR_APP_BEGIN();

	Elm_Transit *transit = NULL;
	Elm_Transit_Effect *effect = NULL;

	transit = elm_transit_add();
	effect = __custom_context_new(100, 100, 250, 250);

	/* open */
	if (effect_type == 0) {
		elm_transit_effect_translation_add(transit, 480, 0, 0, 0);

		elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);
		elm_transit_effect_add(transit, _custom_op, effect, (Elm_Transit_Effect_End_Cb)_custom_context_free);
		elm_transit_go(transit);
	/* close */
	} else if (effect_type == 1) {
		elm_transit_effect_translation_add(transit,  -480, 0, 0, 0);
		elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);
		elm_transit_effect_add(transit, _custom_op, effect, (Elm_Transit_Effect_End_Cb)_custom_context_free);
		elm_transit_go(transit);
	} else {
		elm_transit_effect_translation_add(transit,  720, 0, 0, 0);
		elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);

		elm_transit_effect_add(transit, _custom_op, effect, (Elm_Transit_Effect_End_Cb)_custom_context_free);
		elm_transit_go(transit);
	}

	elm_transit_del(transit);

	MGR_APP_END();
}

