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
 * @file        detail_info_cert.c
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief
 */

#include <dlog.h>

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cstring.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static char* format_key (char *key);
Eina_Bool _back(void *data, Elm_Object_Item *it);

static char *Cert_Data_Field[12];
static char *Cert_Data[12];

/*
 * This function fill Cert_Data_Field with proper text.
 * These texts are labels to certificate details
 */
static void fillCertDataField() {
    int i = 0;
    while (i < 12) {
        switch (i) {
        case 0:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_COMMON_NAME_C");
            break;

        case 1:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_ORGANISATION_C");
            break;

        case 2:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_COMMON_NAME_C");
            break;

        case 3:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_ORGANISATION_C");
            break;

        case 4:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_VERSION_C");
            break;

        case 5:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_VALID_FROM_C");
            break;

        case 6:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_VALID_TO_C");
            break;

        case 7:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_SERIAL_NUMBER_COLON");
            break;

        case 8:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_SIGNATURE_ALGORITHM_C");
            break;

        case 9:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_KEY_USAGE_C");
            break;

        case 10:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_CERTIFICATION_AUTHORITY_C");
            break;

        case 11:
            Cert_Data_Field[i] = dgettext(PACKAGE, "IDS_ST_BODY_PUBLIC_KEY_C");
            break;
        }
        i++;
    }
}

static void put_no_data_text_if_empty(char **text){
    if ( NULL == text )
        return;
    if ( NULL == *text )
        return;
    if ( strlen(*text)<1 ){
        free(*text);
        *text = strdup(dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA"));
    }
}

static char *getInfoFromCert(CertSvcCertificate cert, CertSvcCertificateField field, char **buffer){

    CertSvcString certSvcString_buffer;
    const char* certSvc_buffer;
    int len;

    certsvc_certificate_get_string_field(cert, field, &certSvcString_buffer);
    certsvc_string_to_cstring(certSvcString_buffer, &certSvc_buffer, &len);
    *buffer = strndup(certSvc_buffer, len);
    certsvc_string_free(certSvcString_buffer);
    LOGD("Cert string field: %s", *buffer);

    return *buffer;
}

static int fillCertData(CertSvcCertificate cert) {
    LOGD("fillCertData");

    char          *empty = dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA");
    time_t        time;
    int           status;
    CertSvcString buffer;
    char          *char_buffer;

    //---- SUBJECT COMMON NAME ----
    if (!getInfoFromCert(cert, CERTSVC_SUBJECT_COMMON_NAME, &Cert_Data[0])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[0]);

    //---- SUBJECT ORGANIZATION NAME ----
    if (!getInfoFromCert(cert, CERTSVC_SUBJECT_ORGANIZATION_NAME, &Cert_Data[1])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[1]);

    //---- ISSUER COMMON NAME ----
    if (!getInfoFromCert(cert, CERTSVC_ISSUER_COMMON_NAME, &Cert_Data[2])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[2]);

    //---- ISSUER ORGANIZATION NAME ----
    if (!getInfoFromCert(cert, CERTSVC_ISSUER_ORGANIZATION_NAME, &Cert_Data[3])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[3]);

    //---- VERSION ----
    if (!getInfoFromCert(cert, CERTSVC_VERSION, &Cert_Data[4])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[4]);

    //---- VALID_FROM ----
    certsvc_certificate_get_not_before(cert, &time);
    Cert_Data[5] = strndup(ctime(&time), strlen(ctime(&time)) - 1);
    if (!Cert_Data[5]) {
        certsvc_certificate_free(cert);
        return -1;
    }
    LOGD("Valid from: %s", ctime(&time));

    //---- VALID_TO ----
    certsvc_certificate_get_not_after(cert, &time);
    Cert_Data[6] = strndup(ctime(&time), strlen(ctime(&time)) - 1);
    if (!Cert_Data[6]) {
        certsvc_certificate_free(cert);
        return -1;
    }
    LOGD("Valid to: %s", Cert_Data[6]);

    //---- SERIAL NUMBER ----
    if (!getInfoFromCert(cert, CERTSVC_SERIAL_NUMBER, &Cert_Data[7])) {
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[7]);

    //---- SIGNATURE ALGORITHM ----
    if (!getInfoFromCert(cert, CERTSVC_SIGNATURE_ALGORITHM, &Cert_Data[8])) {
        return -1;
    }

    //---- KEY USAGE ----
    if (!getInfoFromCert(cert, CERTSVC_KEY_USAGE, &Cert_Data[9])) {
        return -1;
    }
    put_no_data_text_if_empty(&Cert_Data[9]);

    //---- CA ----
    certsvc_certificate_is_root_ca(cert, &status);
    if (status == CERTSVC_TRUE) {
        Cert_Data[10] = strdup(dgettext(PACKAGE, "IDS_ST_BODY_TRUE"));
        if (!Cert_Data[10]) {
            certsvc_certificate_free(cert);
            return -1;
        }
    } else if (status == CERTSVC_FALSE) {
        Cert_Data[10] = strdup(dgettext(PACKAGE, "IDS_ST_BODY_FALSE"));
        if (!Cert_Data[10]) {
            certsvc_certificate_free(cert);
            return -1;
        }
    } else {
        Cert_Data[10] = strdup(empty);
        if (!Cert_Data[10]) {
            certsvc_certificate_free(cert);
            return -1;
        }
    }

    //---- PUBLIC KEY ----
    certsvc_certificate_get_string_field(cert, CERTSVC_KEY, &buffer);
    char_buffer = strndup(buffer.privateHandler, buffer.privateLength);
    LOGD("char_buffer : %s", char_buffer);
    Cert_Data[11] = format_key(char_buffer);

    certsvc_string_free(buffer);
    free(char_buffer);
    certsvc_certificate_free(cert);

    if (!Cert_Data[11]) {
        return -1;
    }
    SECURE_LOGD("Public Key: %s", Cert_Data[11]);

    return 0;
}

static int fillCertDataFromFile(char *path_to_cert) {
    LOGD("fillCertDataFromFile");

    struct ug_data *ad = get_ug_data();

    CertSvcCertificate cert;
    if (CERTSVC_SUCCESS != certsvc_certificate_new_from_file(ad->instance, path_to_cert, &cert)) {
        return -1;
    }

    return fillCertData(cert);
}

static void clearCertData() {
    int i = 0;
    while (i < 12) {
        if (Cert_Data[i] != NULL)
            free(Cert_Data[i]);
        i++;
    }
}

static char* format_key(char *key) {

    LOGD("format_key");
    int i = 0;
    int j = 0;
    int len = strlen(key);
    const char *public_key_label = "Public-Key: ";
    char *result;
    char *formated_key = malloc(sizeof(char) * len);

    // remove whitespaces at the beginning
    while ((i < len - 1) && (' ' == key[i])){
        i++;
    }

    // remove the "Public-key: " string at the beginning
    if( 0 == strncmp(public_key_label, &(key[i]), strlen(public_key_label)) ){
        i += strlen(public_key_label);
    }

    while (i < len - 1) {
        if (key[i] == ':' && key[i + 1] == '\n') {
            formated_key[j] = key[i];
            i += 2;
            j += 1;
        } else if (key[i] == ' ' && key[i + 1] == ' ') {
            i += 2;
        } else {
            formated_key[j] = key[i];
            i++;
            j++;
        }
        if (key[i] == 'E' || key[i] == 'M') {
            formated_key[j] = '<';
            j++;
            formated_key[j] = 'b';
            j++;
            formated_key[j] = 'r';
            j++;
            formated_key[j] = '>';
            j++;
        }
    }
    if (i < len && key[i] != '\n') {
        formated_key[j] = key[i];
        i++;
        j++;
    }
    result = strndup(formated_key, j);
    free(formated_key);
    return result;
}

static void _gl_lang_changed(void *data, Evas_Object *obj, void *event_info)

{
   //Update genlist items. The Item texts will be translated in the gl_text_get().
   elm_genlist_realized_items_update(obj);
}

static char* _gl_get_text(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;
    if (index > 11 || index < 0) {
        LOGD("Wrong *index - return NULL");
        return NULL;
    }
    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        LOGD("%s", Cert_Data[index]);
        return strdup(Cert_Data_Field[index]);

    } else if (!strcmp(part, "elm.text.2")) {
        LOGD("%s", Cert_Data_Field[index]);
        return strdup(Cert_Data[index]);
    }
    return NULL;
}

static char* _gl_get_text_multiline(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;
    if (index > 11 || index < 0) {
        LOGD("Wrong *index - return NULL");
        return NULL;
    }

    if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
        return strdup(Cert_Data_Field[index]);
    }
    else if (!strcmp(part, "elm.text.2")){
        LOGD("%s", Cert_Data[index]);
        return strdup(Cert_Data[index]);
    }
    return NULL;
}

static char* _gl_get_text_group(void *data, Evas_Object *obj, const char *part) {

    int index = (int) data;
    if (index == 0) {
        LOGD("OWNER");
        return strdup(dgettext(PACKAGE, "IDS_ST_BODY_OWNER_C"));
    }
    if (index == 1) {
        LOGD("ISSUER");
        return strdup(dgettext(PACKAGE, "IDS_ST_BODY_ISSUER_COLON"));
    }
    if (index == 2) {
        LOGD("DATA");
        return strdup(dgettext(PACKAGE, "IDS_ST_BODY_CERTIFICATE_INFORMATION"));
    }

    else
        LOGD("Wrong *index - return NULL");

    return NULL;
}

static Elm_Genlist_Item_Class itc_group = {
        .item_style       = "dialogue/grouptitle",
        .func.text_get    = _gl_get_text_group,
        .func.content_get = NULL,
        .func.state_get   = NULL,
        .func.del         = NULL
};

static Elm_Genlist_Item_Class itc_2text = {
        .item_style       = "multiline/2text",
        .func.text_get    = _gl_get_text,
        .func.content_get = NULL,
        .func.state_get   = NULL,
        .func.del         = NULL
};

static Elm_Genlist_Item_Class itc_2text_multiline = {
        .item_style       = "multiline/2text",
        .func.text_get    = _gl_get_text_multiline,
        .func.content_get = NULL,
        .func.state_get   = NULL,
        .func.del         = NULL
};

void show_detail_info() {

    LOGD("show_detail_info()");
    struct ug_data *ad = get_ug_data();

    Evas_Object *genlist = NULL;

    genlist = elm_genlist_add(ad->win_main);
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    evas_object_smart_callback_add(genlist, "language,changed", _gl_lang_changed, NULL);
    evas_object_smart_callback_add(genlist, "selected", genlist_clicked_cb, NULL);

    Elm_Object_Item * it;
    int i = 0;
    while (i < 12) {
        if (i == 0) { // Adding label "Owner"
            it = elm_genlist_item_append(genlist, &itc_group, (void *) 0, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
            elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
        }
        if (i == 2) { // Adding label "Issuer"
            it = elm_genlist_item_append(genlist, &itc_group, (void *) 1, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
            elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
        }
        if (i == 4) { // Adding label "Certificate data"
            it = elm_genlist_item_append(genlist, &itc_group, (void *) 2, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
            elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
        }

        LOGD("Try to append %d genlist item...", i);

        if (i == 11) {
            it = elm_genlist_item_append(genlist, &itc_2text_multiline, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, NULL,
                    NULL);
        } else {
            it = elm_genlist_item_append(genlist, &itc_2text, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        }
        elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
        LOGD("Succesful append %d genlist", i);
        i += 1;
    }

    LOGD("Push genlist");
	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_CERTIFICATE_DETAILS", NULL, NULL, genlist, NULL);
	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, _back, (struct Evas_Object *)ad);

}

void get_info_cert_from_file_cb(struct ug_data *ad) {

    LOGD("get_info_cert_from_file_cb()");

    if (ad->data == NULL) {
        LOGD("ad->data is NULL; return");
        return;
    }
    fillCertDataField();
    if(NULL == ad->data){
        return;
    }
    char *cert_path = (char *) ad->data;
    SECURE_LOGD("cert_path = %s", cert_path);

    LOGD("filling cert data...");
    if (fillCertDataFromFile(cert_path)) {
        SECURE_LOGD("Error in fillCertDataFromFile( %s )", cert_path);
        LOGD("EXIT");
        return;
    }

    show_detail_info();

    free(ad->data);
    ad->data = NULL;
}

void get_info_cert_from_certificate_cb(CertSvcCertificate cert) {

    LOGD("get_info_cert_from_certificate_cb()");

    fillCertDataField();

    LOGD("filling cert data...");
    if (fillCertData(cert)) {
        LOGD("Error in fillCertData");
        LOGD("EXIT");
        return;
    }

    show_detail_info();
}

Eina_Bool _back(void *data, Elm_Object_Item *it)
{
	   
    clearCertData();
	return EINA_TRUE;   
}

