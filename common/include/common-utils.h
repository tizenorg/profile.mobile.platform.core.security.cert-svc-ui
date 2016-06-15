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
 * @file        common-utils.h
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief
 */

#ifndef __CERTSVC_UI_COMMON_UTILS_H_
#define __CERTSVC_UI_COMMON_UTILS_H_

#include <Elementary.h>
#include <dlog.h>

#ifdef BUILD_MGR_APP
#include "mgr-app-uigadget.h"
#else // BUILD_CERT_SELECTION_UG
#include "cert-selection-uigadget.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CERT_SVC_UI"

#ifdef _cplusplus
extern "C" {
#endif

typedef void (*E_CB)(void *, Evas_Object *, void *);

Evas_Object *common_genlist(Evas_Object *parent);
Evas_Object *common_back_btn(struct ug_data *ad);
Evas_Object *add_common_done_btn(struct ug_data *ad, E_CB);
Evas_Object *add_common_cancel_btn(struct ug_data *ad, E_CB);

void common_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
Evas_Object *common_more_ctxpopup(Evas_Object *parent);
void common_move_more_ctxpopup(Evas_Object *popup);

#ifdef _cplusplus
}
#endif

#endif // __CERTSVC_UI_COMMON_UTILS_H_
