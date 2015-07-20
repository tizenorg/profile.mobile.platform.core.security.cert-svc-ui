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
 * @file        certificates.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef __DEF_certificates_H_
#define __DEF_certificates_H_

#include <Elementary.h>
#include <cert-svc/ccert.h>
#include <tzplatform_config.h>

#include "mgr-app-uigadget.h"

//sd-card
#define PATH_SDCARD          tzplatform_mkpath(TZ_SYS_STORAGE, "sdcard/")
#define PATH_MEDIA           tzplatform_getenv(TZ_USER_CONTENT)
#define PATH_MEDIA_DOWNLOADS tzplatform_mkpath(TZ_USER_CONTENT, "Downloads/")

// certificates menu
void direct_pfx_install_screen_cb(void *data, Evas_Object *obj, void *event_info);
void certificates_menu_cb (void *data, Evas_Object *obj, void *event_info);
void trusted_root_cert_cb (void *data, Evas_Object *obj, void *event_info);
void pfx_cert_cb          (void *data, Evas_Object *obj, void *event_info);

// trusted root certificates
void trusted_root_cert_selection_cb     (void *data, Evas_Object *obj, void *event_info);
Eina_Bool trusted_root_cert_create_list (struct ug_data *ad);

// PFX certificate
Elm_Object_Item* pfx_cert_install_cb (void *data, Evas_Object *obj, void *event_info);
void put_pkcs12_name_cb          (void *data, Evas_Object *obj, void *event_info);
void put_pkcs12_name_and_pass_cb (void *data, Evas_Object *obj, void *event_info);
void pfx_cert_remove_cb          (void *data, Evas_Object *obj, void *event_info);

// cert general
void get_info_cert_from_file_cb        (struct ug_data *ad, void *list);
void get_info_cert_from_certificate_cb (CertSvcCertificate cert);

// common cb
Evas_Object *create_2_text_with_title_tabbar(Evas_Object *parent);

//---------------------------------------------

#endif /* __DEF_certificates_H__ */
