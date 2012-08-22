/*
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of the Manage Applications
 * Written by Eunmi Son <eunmi.son@samsung.com>
 *
 * Author of this file:
 * Janusz Kozerski <j.kozerski@samsung.com>
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

#ifndef __DEF_certificates_H_
#define __DEF_certificates_H_

#include <Elementary.h>
#include "mgr-app-uigadget.h"

// Path to the certs
//root path
#define PATH_CERT_ROOT "/opt/share/cert-svc/certs"

//mdm
#define PATH_CERT_MDM "/opt/share/cert-svc/certs/mdm"
#define PATH_CERT_MDM_SECURITY "/opt/share/cert-svc/certs/security"
#define PATH_CERT_MDM_SECURITY_CERT "/opt/share/cert-svc/certs/security/cert"

//ssl
#define PATH_CERT_SSL "/opt/share/cert-svc/certs/ssl"

//sim
#define PATH_CERT_SIM_OPERATOR "/opt/share/cert-svc/certs/sim/operator"
#define PATH_CERT_SIM_THIRDPARTY "/opt/share/cert-svc/certs/sim/thirdparty"

//user
#define PATH_CERT_USER "/opt/share/cert-svc/certs/user"
#define PATH_CERT_TRUSTEDUSER "/opt/share/cert-svc/certs/trusteduser"

//code-signing
#define PATH_CERT_WAC "/opt/share/cert-svc/certs/code-signing/wac"

//sd-card
#define PATH_SDCARD "/opt/storage/sdcard"

// certificates menu
void certificates_menu_cb(void *data, Evas_Object *obj, void *event_info);
void trusted_root_cert_cb(void *data, Evas_Object *obj, void *event_info);
void user_cert_cb(void *data, Evas_Object *obj, void *event_info);
void pfx_cert_cb(void *data, Evas_Object *obj, void *event_info);


// trusted root certificates
void trusted_root_cert_selection_cb(void *data, Evas_Object *obj, void *event_info);

// user certificate
void user_search_cert_cb(void *data, Evas_Object *obj, void *event_info);
void user_cert_selection_cb(void *data, Evas_Object *obj, void *event_info);

// PFX certificate
void pfx_cert_install_cb(void *data, Evas_Object *obj, void *event_info);
void put_pkcs12_name_cb(void *data, Evas_Object *obj, void *event_info);
void put_pkcs12_name_and_pass_cb(void *data, Evas_Object *obj, void *event_info);
void pfx_cert_remove_cb(void *data, Evas_Object *obj, void *event_info);
void pfx_cert_create_list(struct ug_data *ad);

// cert general
void get_info_cert_cb(void *data, Evas_Object *obj, void *event_info);
void delete_cert_cb(void *data, Evas_Object *obj, void *event_info);
void install_cb(void *data, Evas_Object *obj, void *event_info);

//---------------------------------------------

#endif /* __DEF_certificates_H__ */
