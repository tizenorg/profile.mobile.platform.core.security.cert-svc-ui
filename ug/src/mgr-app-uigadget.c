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
 * @file        mgr-app-uigadget.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <dlog.h>
#include <efl_extension.h>

#include "mgr-app-uigadget.h"
#include "certificates/certificates.h"

static struct ug_data *ugd = NULL;
struct ug_data *get_ug_data()
{
    return ugd;
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode, app_control_h service, void *priv)
{
	if (!ug || !priv)
        return NULL;

    bindtextdomain(PACKAGE, LOCALEDIR);

    ugd = priv;
    ugd->ug = ug;

    ugd->win_main = ug_get_parent_layout(ug);
    if (!ugd->win_main)
        return NULL;

	ugd->bg = elm_bg_add(ugd->win_main);
    if (!ugd->bg) {
        LOGD("ugd->bg is null");
        free(ugd->win_main);
        return NULL;
    }
    evas_object_size_hint_weight_set(ugd->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(ugd->bg);

    ugd->layout_main = elm_layout_add(ugd->win_main);
    if (!ugd->layout_main) {
        LOGD("ugd->layout_main is null");
        free(ugd->win_main);
        free(ugd->bg);
        return NULL;
    }

    elm_layout_theme_set(ugd->layout_main, "layout", "application", "default");
    evas_object_size_hint_weight_set(ugd->layout_main, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(ugd->layout_main);

    elm_object_part_content_set(ugd->layout_main, "elm.swallow.bg", ugd->bg);

    ugd->navi_bar = elm_naviframe_add(ugd->layout_main);
    if (!ugd->navi_bar) {
        LOGD("ugd->navi_bar is null");
        free(ugd->win_main);
        free(ugd->bg);
        free(ugd->layout_main);
        return NULL;
    }
    elm_object_part_content_set(ugd->layout_main, "elm.swallow.content", ugd->navi_bar);
	elm_naviframe_prev_btn_auto_pushed_set(ugd->navi_bar, EINA_FALSE);
	eext_object_event_callback_add(ugd->navi_bar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, ugd);
    evas_object_show(ugd->navi_bar);


    if (service)
    	direct_pfx_install_screen_cb((void*) ugd, NULL, NULL);
    else
		certificates_menu_cb((void*) ugd, NULL, NULL);

    return ugd->layout_main;
}

static void on_start(ui_gadget_h ug, app_control_h service, void *priv) {

}

static void on_pause(ui_gadget_h ug, app_control_h service, void *priv) {

}

static void on_resume(ui_gadget_h ug, app_control_h service, void *priv) {

}

static void on_destroy(ui_gadget_h ug, app_control_h service, void *priv) {
	LOGD("on_destroy");

    if (NULL == ug) {
        LOGD("NULL == ug; return");
        return;
    }
    if (NULL == priv) {
        LOGD("NULL == priv; return");
        return;
    }

    ugd = priv;

    if (ugd->theme) {
    	elm_theme_free(ugd->theme);
    	ugd->theme = NULL;
    }

	certsvc_instance_free(ugd->instance);

	evas_object_hide(ugd->layout_main);
    evas_object_del(ugd->layout_main);	
    ugd->layout_main = NULL;
}

static void on_message(ui_gadget_h ug, app_control_h msg, app_control_h service, void *priv) {

}

static void on_event(ui_gadget_h ug, enum ug_event event, app_control_h service, void *priv) {
    switch (event) {
    case UG_EVENT_LOW_MEMORY:
        break;
    case UG_EVENT_LOW_BATTERY:
        break;
    case UG_EVENT_LANG_CHANGE:
        break;
    case UG_EVENT_ROTATE_PORTRAIT:
        break;
    case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
        break;
    case UG_EVENT_ROTATE_LANDSCAPE:
        break;
    case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
        break;
    default:
        break;
    }
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h service, void *priv) {
    if (NULL == ug) {
        LOGD("NULL == ug; return");
        return;
    }

    switch (event) {
    case UG_KEY_EVENT_END:
        ug_destroy_me(ug);
        break;
    default:
        break;
    }
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops) {

    if (NULL == ops) {
        LOGD("NULL == ops; return");
        return -1;
    }

    ugd = calloc(1, sizeof(struct ug_data));

    ops->create = on_create;
    ops->start = on_start;
    ops->pause = on_pause;
    ops->resume = on_resume;
    ops->destroy = on_destroy;
    ops->message = on_message;
    ops->event = on_event;
    ops->key_event = on_key_event;
    ops->priv = ugd;
    ops->opt = UG_OPT_INDICATOR_ENABLE;

    return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops) {

    if (NULL == ops) {
        LOGD("NULL == ops; return");
        return;
    }

    ugd = ops->priv;
    free(ugd);
}

UG_MODULE_API int setting_plugin_reset(app_control_h service, void *priv) {
    /* nothing to do for Setting>Reset */
    return 0;
}
