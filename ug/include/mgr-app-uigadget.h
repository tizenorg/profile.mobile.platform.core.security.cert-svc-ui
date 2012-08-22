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


#ifndef __UG_SETTING_MANAGE_APPLICATION_EFL_H__
#define __UG_SETTING_MANAGE_APPLICATION_EFL_H__

#include <Elementary.h>
#include <ui-gadget.h>
#include <ui-gadget-module.h>
#include <glib.h>
#include <app.h>
#include <cert-svc/cinstance.h>

#ifndef PACKAGE
#define PACKAGE "ug-setting-manage-certificates-efl"
#endif

#ifndef LOCALEDIR
#define LOCALEDIR PREFIX"/res/locale"
#endif

#define IMAGE_PATH PREFIX"/res/images"

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

	Eina_Bool           uninstall;

	char                *uninstall_path;
	void                *data_to_clear;

	void                *list_to_refresh;
	struct ListElement  *list_element_to_refresh;
	char                dir_to_refresh[512];
	CertSvcInstance     instance;
	Evas_Object         *genlist_pfx;

	Evas_Object         *indicator;
	Ecore_Pipe          *msg_pipe;

};

struct ug_data *get_ug_data();

#endif /* __UG_SETTING_MANAGE_APPLICATION_EFL_H__ */
