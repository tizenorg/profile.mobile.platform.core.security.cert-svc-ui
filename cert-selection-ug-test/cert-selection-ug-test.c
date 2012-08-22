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
 * @file        cert-selection-ug-test.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

/**
 * To use this test app:
 * 1) Install cert-svc-test and cert-svc-ui packages on target
 * 2) In first console run "dlogutil" on target
 * 3) In second console run /usr/bin/cert-selection-ug-test
 * 4) In app that will show on taget click "Selecttion screen" button
 * 5) test "Selection screen" and check in logs (on first terminal) if
 *    it works correctly
 * 6) Push "Exit" button to exit test app.
 */

#include <Elementary.h>
#include <Ecore_X.h>
#include "dlog.h"
#include <Evas.h>
#include <app.h>

#include <stdio.h>
#include <ui-gadget.h>
#include <ui-gadget-module.h>
#include <SLP_UI_Gadget_PG.h>
#include <ui-gadget-engine.h>

typedef struct appdata_t {
    ui_gadget_h ug;
    Evas_Object *win;
    Evas_Object *box;
    Evas_Object *lab;
    Evas_Object *btn_show_selection_screen;
    Evas_Object *btn_exit;
} appdata;

static appdata *ad;

static service_h service;

static void layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv) {
    Evas_Object *base, *win;
    appdata *mydata;

    if (!ug || !priv)
        return;

    mydata = priv;

    base = ug_get_layout(ug);
    if (!base)
        return;

    win = ug_get_window();

    switch (mode) {
    case UG_MODE_FULLVIEW:
        evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(win, base);
        ug_disable_effect(ug);
        evas_object_show(base);
        break;
    default:
        break;
    }
}

static void result_cb(ui_gadget_h ug, service_h service, void *priv) {

    LOGD("result_cb");

    if (!ug || !priv)
        return;

    char *value;
    service_get_extra_data(service, "selected-cert", &value);

    LOGD("#################################################");
    LOGD("Value catch in return");
    LOGD("seleted-cert = %s", value);
    LOGD("#################################################");

    free(value);
}

static void destroy_cb(ui_gadget_h ug, void *priv) {

    if (!ug || !priv)
        return;

    ug_destroy(ug);
    ((appdata *) priv)->ug = NULL;
}

ui_gadget_h create_ug(appdata *data) {

    struct ug_cbs cbs = { 0, };
    cbs.layout_cb = layout_cb;
    cbs.result_cb = result_cb;
    cbs.destroy_cb = destroy_cb;
    cbs.priv = (void *) data;

    data->ug = ug_create(NULL, "cert-selection-ug-efl", UG_MODE_FULLVIEW, service, &cbs);

    return data->ug;
}

static void on_done(void *data, Evas_Object *obj, void *event_info) {
    (void) data;
    (void) obj;
    (void) event_info;

    elm_exit();
}

static void ok_button(void *data, Evas_Object *obj, void *event_info) {

    (void) obj;
    (void) event_info;

    appdata *mydata = (appdata *) data;
    mydata->ug = create_ug(ad);
}

EAPI_MAIN int elm_main(int argc, char **argv) {

    appdata appd;

    ad = &appd;

    service_create(&service);

    // new window - do the usual and give it a name (hello) and title (Hello)
    ad->win = elm_win_util_standard_add("Test app", "Test app");
    // when the user clicks "close" on a window there is a request to delete
    evas_object_smart_callback_add(ad->win, "delete,request", on_done, NULL);

    // add a box object - default is vertical. a box holds children in a row,
    // either horizontally or vertically. nothing more.
    ad->box = elm_box_add(ad->win);
    // make the box horizontal
    elm_box_horizontal_set(ad->box, EINA_TRUE);
    // add object as a resize object for the window (controls window minimum
    // size as well as gets resized if window is resized)
    elm_win_resize_object_add(ad->win, ad->box);
    evas_object_show(ad->box);

    // add a label widget, set the text and put it in the pad frame
    //lab = elm_label_add(win);
    // set default text of the label
    //elm_object_text_set(lab, "");
    // pack the label at the end of the box
    //elm_box_pack_end(box, lab);
    //evas_object_show(lab);

    ad->btn_show_selection_screen = elm_button_add(ad->win);
    elm_object_text_set(ad->btn_show_selection_screen, "Selection screen");
    elm_box_pack_end(ad->box, ad->btn_show_selection_screen);
    evas_object_show(ad->btn_show_selection_screen);
    evas_object_smart_callback_add(ad->btn_show_selection_screen, "clicked", ok_button, ad);

    ad->btn_exit = elm_button_add(ad->win);
    elm_object_text_set(ad->btn_exit, "Exit App");
    elm_box_pack_end(ad->box, ad->btn_exit);
    evas_object_show(ad->btn_exit);
    evas_object_smart_callback_add(ad->btn_exit, "clicked", on_done, NULL);

    UG_INIT_EFL(ad->win, UG_OPT_INDICATOR_ENABLE);

    // now we are done, show the window
    evas_object_show(ad->win);

    // run the mainloop and process events and callbacks
    elm_run();
    elm_shutdown();

    service_destroy(service);

    return 0;
}
ELM_MAIN();
