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


#include "mgr-app-view-manager.h"
#include "mgr-app-widget.h"
#include "mgr-app-common-debug.h"
#include "mgr-app-common-util.h"

static Evas_Object  *_win_main     = NULL;
static Evas_Object  *_bg           = NULL;
static Evas_Object  *_layout_main  = NULL;
static Evas_Object  *_navi_bar     = NULL;
static Evas_Object  *_popup        = NULL;
static GList        *_view_list    = NULL;

static mgr_app_result_e _mgr_app_view_navigation_push(mgr_app_view_t *view, void *data);

static mgr_app_result_e _mgr_app_view_navigation_push(mgr_app_view_t *view, void *data) {

    MGR_APP_BEGIN();

    Evas_Object *parent_navibar = view->view_common_data->common_navibar;
    Evas_Object *view_layout = view->view_common_data->view_layout;
    const char *title = view->view_common_data->title;
    Evas_Smart_Cb btn1_func = view->view_common_data->btn1_func;
    Evas_Object *cbar = view->view_common_data->view_cbar;
    Evas_Object *l_button = NULL;
    Elm_Object_Item *navi_it = NULL;

    retv_if(view_layout == NULL, MGR_APP_ERROR);
    retv_if(parent_navibar == NULL, MGR_APP_ERROR);

    l_button = elm_button_add(parent_navibar);
    navi_it = elm_naviframe_item_push(parent_navibar, title, l_button, NULL, view_layout, NULL);

    if (btn1_func != NULL) {
        elm_object_style_set(l_button, "naviframe/back_btn/default");
        evas_object_smart_callback_add(l_button, "clicked", btn1_func, data);
    } //else {
      //	/* remove the arrow button. */
      //elm_object_item_part_content_set(navi_it,  "prev_btn", l_button);
    //}

    if (cbar != NULL) {
        elm_object_item_part_content_set(navi_it, "optionheader", cbar);
    }

    MGR_APP_END();
    return MGR_APP_OK;
}


void mgr_app_view_set_win_main(Evas_Object *winmain)
{
	_win_main = winmain;
}

Evas_Object *mgr_app_view_get_win_main(void)
{
	return _win_main;
}

void mgr_app_view_set_bg(Evas_Object *bg)
{
	_bg = bg;
}

Evas_Object *mgr_app_view_get_bg(void)
{
	return _bg;
}

void mgr_app_view_set_layout_main(Evas_Object *layoutmain)
{
	_layout_main = layoutmain;
}

Evas_Object *mgr_app_view_get_layout_main(void)
{
	return _layout_main;
}

void mgr_app_view_set_navibar(Evas_Object *navibar)
{
	_navi_bar = navibar;
}

Evas_Object *mgr_app_view_get_navibar(void)
{
	return _navi_bar;
}

void mgr_app_view_set_popup(Evas_Object *popup)
{
	_popup = popup;
}

Evas_Object *mgr_app_view_get_popup(void)
{
	return _popup;
}

void mgr_app_view_set_viewlist(GList *viewlist)
{
	_view_list = viewlist;
}

GList *mgr_app_view_get_viewlist(void *data)
{
	return _view_list;
}

mgr_app_result_e mgr_app_view_create(mgr_app_view_t *view, void *data)
{
	MGR_APP_BEGIN();

	retv_if(view->is_created == TRUE, MGR_APP_ERROR);
	retv_if(data == NULL, MGR_APP_ERROR);
	retv_if(_win_main == NULL, MGR_APP_ERROR);

	Evas_Object *view_base_ly 		= NULL;
	Evas_Object *navibar 			= NULL;
	Evas_Object *base_layout_main = NULL;
	Evas_Object	*bg					= NULL;
	Evas_Object *cbar 				= NULL;
	GList 		*first_view 		= NULL;
	GList 		*last_view 		= NULL;
	GList 		*target_view 		= NULL;
	GList 		*target_view_next = NULL;
	GList 		*target_view_prev = NULL;

	MGR_APP_MEM_MALLOC(view->view_common_data, 1, mgr_app_view_common_data_t);

	if (_bg == NULL) {
		bg = mgr_app_widget_create_bg(_win_main);
		retv_if(bg == NULL, MGR_APP_ERROR);
		_bg = bg;
	}
	if (_layout_main == NULL) {
		base_layout_main = mgr_app_widget_create_main_layout(_win_main);
		retv_if(base_layout_main == NULL, MGR_APP_ERROR);
		_layout_main = base_layout_main;
	}
	if (_navi_bar == NULL) {
		navibar = mgr_app_view_create_base_navigation(_layout_main);
		retv_if(navibar == NULL, MGR_APP_ERROR);
		_navi_bar = navibar;
	}

	view->view_common_data->common_navibar 		= _navi_bar;
	view->view_common_data->common_layout_main 	= _layout_main;
	view->view_common_data->common_bg 			= _bg;

	MGR_APP_DEBUG("view's pointer:%p\n", view);
	target_view = g_list_find(_view_list, view);
	if (target_view == NULL) {
		MGR_APP_DEBUG("there is no same view in the view_list. so we should add the view to view list\n");
		_view_list=first_view = g_list_append(_view_list, view);
		MGR_APP_DEBUG("first_view's pointer:%p\n", first_view);
		MGR_APP_DEBUG("last_view's pointer:%p\n", g_list_last(_view_list));
		MGR_APP_DEBUG("first_view's pointer:%p\n", g_list_first(_view_list));
	} else {
		MGR_APP_DEBUG("there is same view in the view_list. so we should move the view to last of view list\n");
		target_view_prev = g_list_previous(target_view);
		target_view_next = g_list_next(target_view);
		last_view = g_list_last(_view_list);

		if (target_view_prev == NULL) {
			MGR_APP_DEBUG_ERR("target_view_prev is null");
			//return MGR_APP_ERROR;
		} else if (target_view_next == NULL) {
			MGR_APP_DEBUG_ERR("target_view_next is null");
			//return MGR_APP_ERROR;
		} else {
			target_view_prev->next = target_view_next;
			target_view_next->prev = target_view_prev;
		}

		last_view->next 	= target_view;
		target_view->prev 	= last_view;
		target_view->next 	= NULL;
	}

	view_base_ly = view->create(data);
	if (view_base_ly == NULL) {
		return mgr_app_view_cleanup(view, data);
	}
	view->view_common_data->view_layout = view_base_ly;

	cbar = view->setcbar(data);
	view->view_common_data->view_cbar= cbar;

	view->setnavibar(data);

	_mgr_app_view_navigation_push(view, data);

	view->is_created = TRUE;

	MGR_APP_END();
	return MGR_APP_OK;
}

mgr_app_result_e mgr_app_view_destroy(mgr_app_view_t *view, void *data)
{
	MGR_APP_BEGIN();
	retv_if(data == NULL, -1);
	retv_if(view == NULL, -1);
	GList *target_view = NULL;

	view->destroy(data);

	retv_if(view->view_common_data == NULL, -1);
	MGR_APP_MEM_FREE(view->view_common_data->user_view_data);
	MGR_APP_MEM_FREE(view->view_common_data->title);
	MGR_APP_MEM_FREE(view->view_common_data->btn1_label);
	MGR_APP_MEM_FREE(view->view_common_data);

	view->is_created = FALSE;
	target_view = g_list_find(_view_list, view);
	if (target_view != NULL) {
		MGR_APP_DEBUG("find view in view_list\n");
		_view_list = g_list_delete_link(_view_list, target_view);
	} else {
		MGR_APP_DEBUG("Can't find view in view_list\n");
	}

	MGR_APP_END();
	return MGR_APP_OK;
}

mgr_app_result_e mgr_app_view_update(mgr_app_view_t *view, void *data)
{
	MGR_APP_BEGIN();

	view->update(data);

	MGR_APP_END();
	return MGR_APP_OK;
}

mgr_app_result_e mgr_app_view_cleanup(mgr_app_view_t *view, void *data)
{
	MGR_APP_BEGIN();
	retv_if(data == NULL, -1);

	GList 			*last_list 	= NULL;
	mgr_app_view_t 	*target_view 	= NULL;

	last_list = g_list_last(_view_list);
	MGR_APP_DEBUG("last_list pointer:%p\n", last_list);
	while (last_list != NULL) {
		MGR_APP_DEBUG("last_list pointer:%p\n", last_list);
		target_view = (mgr_app_view_t *)last_list->data;
		mgr_app_view_destroy(target_view, data);
		MGR_APP_BEGIN();
		last_list = g_list_last(_view_list);
	}

	g_list_free(_view_list);
	_view_list = NULL;

	MGR_APP_END();
	return MGR_APP_OK;
}

mgr_app_result_e mgr_app_view_manager_set_navibar(mgr_app_view_common_data_t *view_common_data, char *title, char *btn1_label, Evas_Smart_Cb btn1_func, const char *btn2_label, 	Evas_Smart_Cb btn2_func)
{
	MGR_APP_BEGIN();

	MGR_APP_MEM_STRDUP(view_common_data->title, title);
	MGR_APP_MEM_STRDUP(view_common_data->btn1_label, btn1_label);
	view_common_data->btn1_func = btn1_func;

	MGR_APP_END();
	return MGR_APP_OK;
}

mgr_app_result_e mgr_app_view_manager_navibar_title_update(const char *title, mgr_app_view_t *view, void *data)
{
	MGR_APP_BEGIN();

	retv_if(view == NULL, MGR_APP_ERROR);

	mgr_app_view_common_data_t *view_common_data = NULL;
	Evas_Object *parent_navibar = NULL;
	Elm_Object_Item	*navi_it = NULL;

	view_common_data = view->view_common_data;
	retv_if(view_common_data == NULL, MGR_APP_ERROR);

	parent_navibar = view_common_data->common_navibar;
	retv_if(parent_navibar == NULL, MGR_APP_ERROR);

	MGR_APP_MEM_FREE(view_common_data->title);
	MGR_APP_MEM_STRDUP(view_common_data->title, title);

	navi_it = elm_naviframe_top_item_get(_navi_bar);
	elm_object_item_text_set(navi_it, view_common_data->title);

	MGR_APP_END();
	return MGR_APP_OK;
}

void mgr_app_view_common_back_cb(void *data, Eina_Bool is_update)
{
	MGR_APP_BEGIN();

	ret_if(data == NULL);

	GList *last 	= NULL;
	GList *update 	= NULL;

	Elm_Object_Item *top_it = NULL;
	Elm_Object_Item *bottom_it = NULL;

	top_it = elm_naviframe_top_item_get(_navi_bar);
	bottom_it = elm_naviframe_bottom_item_get(_navi_bar);
	
	if (elm_object_item_content_get(top_it) == elm_object_item_content_get(bottom_it)) {
		MGR_APP_END();
		mgr_app_view_cleanup(NULL, data);
	} else {
		elm_naviframe_item_pop(_navi_bar);
		if (_view_list != NULL) {
			last = g_list_last(_view_list);
			if (last != NULL) {
				mgr_app_view_t *destroy_view = NULL;
				mgr_app_view_t *update_view = NULL;

				update = g_list_previous(last);
				if (update != NULL && is_update) {
					update_view = update->data;
					mgr_app_view_update(update_view, data);
				} else {
					MGR_APP_DEBUG("previous view is null or don't need to update");
				}

				destroy_view = last->data;
				mgr_app_view_destroy((mgr_app_view_t*)destroy_view, data);
			}
			MGR_APP_DEBUG("last_view pointer:%p\n", last);
		}
	}

	MGR_APP_END();
}
