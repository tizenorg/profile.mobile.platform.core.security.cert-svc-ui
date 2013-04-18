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

#include "mgr-app-uigadget.h"

#include "certificates/certificates.h"

static struct ug_data *ugd = NULL;
struct ug_data *get_ug_data() {
    LOGD("get_ug_data()");
    return ugd;
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv) {
    LOGD("on_create() BEGIN");

    if (NULL == ug)
        return NULL;
    if (NULL == priv)
        return NULL;

    bindtextdomain(PACKAGE, LOCALEDIR);

    ugd = priv;
    ugd->ug = ug;

    ugd->win_main = ug_get_parent_layout(ug);
    if (NULL == ugd->win_main)
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
    evas_object_show(ugd->navi_bar);	

    certificates_menu_cb((void*) ugd, NULL, NULL);

    LOGD("on_create() END");
    return ugd->layout_main;
}

static void on_start(ui_gadget_h ug, service_h service, void *priv) {

}

static void on_pause(ui_gadget_h ug, service_h service, void *priv) {

}

static void on_resume(ui_gadget_h ug, service_h service, void *priv) {

}

static void on_destroy(ui_gadget_h ug, service_h service, void *priv) {
    LOGD("on_destroy() BEGIN");

    if (NULL == ug) {
        LOGD("NULL == ug; return");
        return;
    }
    if (NULL == priv) {
        LOGD("NULL == priv; return");
        return;
    }

    ugd = priv;

	certsvc_instance_free(ugd->instance);

    evas_object_del(ugd->layout_main);
    ugd->layout_main = NULL;
    LOGD("on_destroy() END");
}

static void on_message(ui_gadget_h ug, service_h msg, service_h service, void *priv) {

}

static void on_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv) {
    LOGD("on_event() BEGIN");
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
    LOGD("on_event() END");
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, service_h service, void *priv) {
    LOGD("on_key_event() BEGIN");
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
    LOGD("on_key_event() END");
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops) {
    LOGD("UG_MODULE_INIT() BEGIN");
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

    LOGD("UG_MODULE_INIT() END");
    return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops) {
    LOGD("UG_MODULE_EXIT() BEGIN");
    if (NULL == ops) {
        LOGD("NULL == ops; return");
        return;
    }

    ugd = ops->priv;
    free(ugd);
    LOGD("UG_MODULE_EXIT() END");
}

UG_MODULE_API int setting_plugin_reset(service_h service, void *priv) {
    LOGD("setting_plugin_reset() BEGIN");
    /* nothing to do for Setting>Reset */
    LOGD("setting_plugin_reset() END");
    return 0;
}
