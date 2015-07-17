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
 * @file        mgr-app-uigadget.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef __UG_SETTING_MANAGE_CERTIFICATES_EFL_H__
#define __UG_SETTING_MANAGE_CERTIFICATES_EFL_H__

#include <Elementary.h>
#include <ui-gadget.h>
#include <ui-gadget-module.h>
#include <glib.h>
#include <app.h>
#include <cert-svc/cinstance.h>

#ifndef PACKAGE
#define PACKAGE "setting-manage-certificates-efl"
#endif

#ifndef LOCALEDIR
#define LOCALEDIR PREFIX "/res/locale"
#endif

// Type of screen with certificates that is displayed - it is used to refreshing list
enum Screen {
    NONE_SCREEN,         // Screen is unknown
    TRUSTED_ROOT_SCREEN, // Trusted root
    USER_SCREEN,         // User certificates
    PKCS12_SCREEN        // PKCS12/PFX
};

struct ug_data {
    Evas_Object         *win_main;
    Evas_Object         *bg;
    Evas_Object         *layout_main;
    Evas_Object         *navi_bar;
    GList               *view_list;
    ui_gadget_h         ug;
    ui_gadget_h         sub_ug;
    void                *data;
    Evas_Object         *popup;
    Evas_Object         *more_popup2;
    Elm_Theme 			*theme;

    Evas_Object         *list_to_refresh;
    enum Screen         type_of_screen;
    struct ListElement  *list_element;
    struct ListElement  *list_element_install;

    CertSvcInstance     instance;
    Evas_Object         *indicator;
    Ecore_Pipe          *msg_pipe;
    Elm_Object_Item		*uninstall_button;
    Elm_Object_Item		*user_cert_list_item;
    void (*refresh_screen_cb)(void *data, Evas_Object *obj, void *event_info);

};

struct ug_data *get_ug_data();

#ifdef LOG_TAG
    #undef LOG_TAG
#endif

#ifndef LOG_TAG
    #define LOG_TAG "CERT_SVC_UI"
#endif

#endif /* __UG_SETTING_MANAGE_CERTIFICATES_EFL_H__ */
