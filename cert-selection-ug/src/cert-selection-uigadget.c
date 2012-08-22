/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        cert-ui-api.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <stdio.h>
#include <ui-gadget.h>
#include <ui-gadget-module.h>
#include <app.h>

#include "cert-selection-uigadget.h"
#include "mgr-app-common-debug.h"
#include "mgr-app-common-error.h"
#include "mgr-app-view-manager.h"
#include "mgr-app-widget.h"
#include "mgr-app-common-util.h"

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#define STR_VIEWTYPE                "viewtype"
#define STR_MANAGE_APPLICATIONS     "manage-applications"

static struct ug_data *ugd = NULL;
struct ug_data *get_ug_data() {
    LOGD("get_ug_data()");
    return ugd;
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)mode;

    retv_if(ug == NULL, NULL);
    retv_if(priv == NULL, NULL);

    ugd = priv;
    ugd->ug = ug;

    ugd->service = &service;

    bindtextdomain(PACKAGE, LOCALEDIR);

    ugd->win_main = ug_get_parent_layout(ug);
    retv_if(ugd->win_main == NULL, NULL);

    ugd->bg = mgr_app_widget_create_bg(ugd->win_main);
    if (!ugd->bg) {
        MGR_APP_DEBUG_ERR("ugd->bg is null");
        MGR_APP_MEM_FREE(ugd->win_main);
        return NULL;
    }

    ugd->layout_main = mgr_app_widget_create_main_layout(ugd->win_main);
    if (!ugd->layout_main) {
        MGR_APP_DEBUG_ERR("ugd->layout_main is null");
        MGR_APP_MEM_FREE(ugd->win_main);
        MGR_APP_MEM_FREE(ugd->bg);
        MGR_APP_END();
        return NULL;
    }

    elm_object_part_content_set(ugd->layout_main, "elm.swallow.bg", ugd->bg);

    ugd->navi_bar = mgr_app_view_create_base_navigation(ugd->layout_main);
    if (!ugd->navi_bar) {
        MGR_APP_DEBUG_ERR("ugd->navi_bar is null");
        MGR_APP_MEM_FREE(ugd->win_main);
        MGR_APP_MEM_FREE(ugd->bg);
        MGR_APP_MEM_FREE(ugd->layout_main);
        MGR_APP_END();
        return NULL;
    }

    mgr_app_view_set_win_main(ugd->win_main);
    mgr_app_view_set_bg(ugd->bg);
    mgr_app_view_set_layout_main(ugd->layout_main);
    mgr_app_view_set_navibar(ugd->navi_bar);
    mgr_app_view_set_viewlist(ugd->view_list);

    cert_selection_cb((void*) ugd, NULL, NULL);

    MGR_APP_END();
    return ugd->layout_main;
}

static void on_start(ui_gadget_h ug, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)ug;
    (void)service;
    (void)priv;
    MGR_APP_END();
}

static void on_pause(ui_gadget_h ug, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)ug;
    (void)service;
    (void)priv;
    MGR_APP_END();
}

static void on_resume(ui_gadget_h ug, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)ug;
    (void)service;
    (void)priv;
    MGR_APP_END();
}

static void on_destroy(ui_gadget_h ug, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)service;

    ret_if(ug == NULL);
    ret_if(priv == NULL);

    ugd = priv;

    evas_object_del(ugd->layout_main);
    ugd->layout_main = NULL;
    MGR_APP_END();
}

static void on_message(ui_gadget_h ug, service_h msg, service_h service, void *priv) {
    MGR_APP_BEGIN();

    (void)ug;
    (void)msg;
    (void)service;
    (void)priv;
    MGR_APP_END();
}

static void on_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)ug;
    (void)service;
    (void)priv;

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
    MGR_APP_END();
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)service;
    (void)priv;

    ret_if(ug == NULL);

    switch (event) {
    case UG_KEY_EVENT_END:
        ug_destroy_me(ug);
        break;
    default:
        break;
    }
    MGR_APP_END();
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops) {
    MGR_APP_BEGIN();

    retv_if(ops == NULL, -1);

    MGR_APP_MEM_MALLOC(ugd, (1), struct ug_data);

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

    MGR_APP_END();
    return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops) {
    MGR_APP_BEGIN();

    ret_if(ops == NULL);

    ugd = ops->priv;
    MGR_APP_MEM_FREE(ugd);

    MGR_APP_END();
}

UG_MODULE_API int setting_plugin_reset(service_h service, void *priv) {
    MGR_APP_BEGIN();
    (void)service;
    (void)priv;
    /* nothing to do for Setting>Reset */
    MGR_APP_END();
    return 0;
}
