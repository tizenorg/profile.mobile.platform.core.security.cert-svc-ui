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

#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/ccrl.h>
#include <cert-svc/cocsp.h>

#include "certificates/certificate_util.h"
#include "certificates/certificates.h"

static char* format_key (char *key);
static void _back_cb(void *data, Evas_Object* obj, void* event_info);

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
			Cert_Data_Field[i] = dgettext(PACKAGE, "COMMON_NAME");
			break;

		case 1:
			Cert_Data_Field[i] = dgettext(PACKAGE, "ORGANIZATION");
			break;

		case 2:
			Cert_Data_Field[i] = dgettext(PACKAGE, "COMMON_NAME");
			break;

		case 3:
			Cert_Data_Field[i] = dgettext(PACKAGE, "ORGANIZATION");
			break;

		case 4:
			Cert_Data_Field[i] = dgettext(PACKAGE, "VERSION");
			break;

		case 5:
			Cert_Data_Field[i] = dgettext(PACKAGE, "VALID_FROM");
			break;

		case 6:
			Cert_Data_Field[i] = dgettext(PACKAGE, "VALID_TO");
			break;

		case 7:
			Cert_Data_Field[i] = dgettext(PACKAGE, "SERIAL_NUMBER");
			break;

		case 8:
			Cert_Data_Field[i] = dgettext(PACKAGE, "SIGNATURE_ALGORITHM");
			break;

		case 9:
			Cert_Data_Field[i] = dgettext(PACKAGE, "KEY_USAGE");
			break;

		case 10:
			Cert_Data_Field[i] = dgettext(PACKAGE, "CA");
			break;

		case 11:
			Cert_Data_Field[i] = dgettext(PACKAGE, "PUBLIC_KEY");
			break;
		}
		i++;
	}
}

static char *getInfoFromCert(CertSvcCertificate cert, CertSvcCertificateField field, char **buffer){

    CertSvcString certSvcString_buffer;

    certsvc_certificate_get_string_field(cert, field, &certSvcString_buffer);
    *buffer = strndup(certSvcString_buffer.privateHandler, certSvcString_buffer.privateLength);
    certsvc_string_free(certSvcString_buffer);
    LOGD("Cert string field: %s", *buffer);

    return *buffer;
}

static int fillCertData(char *path_to_cert) {
	LOGD("fillCertData");

	char *empty = dgettext(PACKAGE, "NO_DATA");
	time_t time;
	int status;

	CertSvcInstance instance;
	if (certsvc_instance_new(&instance) == CERTSVC_FAIL) {
		LOGD("CERTSVC_FAIL");
		return -1;
	}
	CertSvcCertificate cert;
	CertSvcString buffer;
	char *char_buffer;

	certsvc_certificate_new_from_file(instance, path_to_cert, &cert);

	//---- SUBJECT COMMON_NAME ----
	if(! getInfoFromCert(cert, CERTSVC_SUBJECT_COMMON_NAME, &Cert_Data[0])){
	    certsvc_certificate_free(cert);
	    return -1;
    }
    if (!getInfoFromCert(cert, CERTSVC_SUBJECT_ORGANIZATION_NAME, &Cert_Data[1])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    if (!getInfoFromCert(cert, CERTSVC_ISSUER_COMMON_NAME, &Cert_Data[2])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    if (!getInfoFromCert(cert, CERTSVC_ISSUER_ORGANIZATION_NAME, &Cert_Data[3])) {
        certsvc_certificate_free(cert);
        return -1;
    }
    if (!getInfoFromCert(cert, CERTSVC_VERSION, &Cert_Data[4])) {
        certsvc_certificate_free(cert);
        return -1;
    }

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

    if (!getInfoFromCert(cert, CERTSVC_SERIAL_NUMBER, &Cert_Data[7])) {
        return -1;
    }
    if (!getInfoFromCert(cert, CERTSVC_SIGNATURE_ALGORITHM, &Cert_Data[8])) {
        return -1;
    }
    if (!getInfoFromCert(cert, CERTSVC_KEY_USAGE, &Cert_Data[9])) {
        return -1;
    }

    //---- CA ----
    certsvc_certificate_is_root_ca(cert, &status);
    if (status == CERTSVC_TRUE) {
        Cert_Data[10] = strdup(dgettext(PACKAGE, "CA_TRUE"));
        if (!Cert_Data[10]) {
            certsvc_certificate_free(cert);
            return -1;
        }
    } else if (status == CERTSVC_FALSE) {
        Cert_Data[10] = strdup(dgettext(PACKAGE, "CA_FALSE"));
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

    //---- PUBLIC_KEY ----
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
    LOGD("Public Key: %s", Cert_Data[11]);

    return 0;
}

static void clearCertData() {
	int i = 0;
	while (i < 12) {
		if (Cert_Data[i] != NULL)
			free(Cert_Data[i]);
		i++;
	}
}

static char* format_key (char *key){

	LOGD("format_key");
	int i = 0;
	int j = 0;
	int len = strlen(key);
	char *result;
	char *formated_key = malloc(sizeof(char) * len);
	while (i<len-1){
		if(key[i] == ':' && key[i+1]=='\n'){
			formated_key[j] = key[i];
			i += 2;
			j += 1;
		}
		else if(key[i] == ' ' && key[i+1]==' '){
			i += 2;
		}
		else{
			LOGD("copy char - %c - next (%c)", key[i], key[i+1]);
			formated_key[j] = key[i];
			i++;
			j++;
		}
		if(key[i] == 'E' || key[i] == 'M'){
			LOGD("-----------------------print <br>");
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
	if(i<len && key[i]!= '\n'){
		formated_key[j] = key[i];
		i++;
		j++;
	}
	result = strndup(formated_key, j);
	free(formated_key);
	return result;
}

static char* _gl_get_text(void *data, Evas_Object *obj, const char *part) {

	int index = (int) data;
	LOGD("_gl_label_get ( %d )", index);
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
	LOGD("return NULL");
	return NULL;
}

static char* _gl_get_text_multiline(void *data, Evas_Object *obj, const char *part) {

	int index = (int) data;
	LOGD("_gl_get_text_multiline ( %d )", index);
	if (index > 11 || index < 0) {
		LOGD("Wrong *index - return NULL");
		return NULL;
	}
	LOGD("%s", Cert_Data[index]);
	return strdup(Cert_Data[index]);
}

static char* _gl_get_text_group(void *data, Evas_Object *obj, const char *part) {

	int index = (int) data;
	LOGD("_gl_get_text_gropu ( %d )", index);
	if(index == 0){
		LOGD("OWNER");
		return strdup(dgettext(PACKAGE, "CERT_OWNER"));
	}
	if(index == 1){
		LOGD("ISSUER");
		return strdup(dgettext(PACKAGE, "CERT_ISSUER"));
	}
	if(index == 2){
		LOGD("DATA");
		return strdup(dgettext(PACKAGE, "CERT_DATA"));
	}

	else
		LOGD("Wrong *index - return NULL");

	return NULL;
}

static Elm_Genlist_Item_Class itc_group = {
		.item_style = "grouptitle",
		.func.text_get = _gl_get_text_group
};

static Elm_Genlist_Item_Class itc_2text = {
		.item_style = "2text.2",
		.func.text_get = _gl_get_text
};

static Elm_Genlist_Item_Class itc_1text_multiline = {
		.item_style = "multiline/1text",
        .func.text_get = _gl_get_text_multiline,
        .func.content_get = NULL,
        .func.state_get = NULL,
        .func.del = NULL
};

void get_info_cert_cb(void *data, Evas_Object *obj, void *event_info) {
	LOGD("get_info_cert_cb");
	struct ug_data *ad = (struct ug_data *) data;
	Evas_Object *genlist = NULL;
	Evas_Object *back = NULL;

	fillCertDataField();

	if (ad->data == NULL)
		return;
	char *cert_path = (char *) ad->data;
	LOGD("cert_path = %s", cert_path);
	LOGD("ad->data  = %s", ad->data);

	genlist = elm_genlist_add(ad->win_main);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "selected", genlist_clicked_cb, NULL);

	LOGD("filling cert data...");
	if (fillCertData(cert_path)) {
		LOGD("Error in fillCertData( %s )", cert_path);
		LOGD("EXIT");
		return;
	}

	Elm_Object_Item * it;
	int i = 0;
	while (i < 12) {
		if(i==0) // Adding label "Owner"
			elm_genlist_item_append(genlist, &itc_group, (void *) 0, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if(i==2) // Adding label "Issuer"
			elm_genlist_item_append(genlist, &itc_group, (void *) 1, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if(i==4) // Adding label "Certificate data"
			elm_genlist_item_append(genlist, &itc_group, (void *) 2, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

		LOGD("Try to append %d genlist item...", i);

		if(i == 11){
			it = elm_genlist_item_append(genlist, &itc_1text_multiline, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
		else{
			it = elm_genlist_item_append(genlist, &itc_2text, (void*) i, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
		LOGD("Succesful append %d genlist", i);
		i += 1;
	}

	LOGD("Push genlist");
	Elm_Object_Item *navi_it = NULL;
	navi_it = elm_naviframe_item_push(ad->navi_bar, dgettext(PACKAGE, "CERTIFICATE_DETAILS"), NULL, NULL, genlist,
			NULL);

	LOGD("Setup callback for back button");
	back = elm_object_item_part_content_get(navi_it, "prev_btn");
	evas_object_smart_callback_add(back, "clicked", _back_cb, ad);
}

static void _back_cb(void *data, Evas_Object* obj, void* event_info) {

	clearCertData();
}
