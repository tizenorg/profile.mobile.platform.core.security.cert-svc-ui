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
#include <cert-svc/ccert.h>
#include <cert-svc/cinstance.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cstring.h>

#include <vconf.h>
#include <vconf-keys.h>
#include <unicode/udat.h>
#include <unicode/ustring.h>
#include <unicode/uloc.h>
#include <unicode/ucal.h>
#include <unicode/udatpg.h>
#include <unicode/utmscale.h>
#include <system_settings.h>

#include "common-utils.h"
#include "certificates/certificate_util.h"

static char *format_key (const char *key);
Eina_Bool _back(void *data, Elm_Object_Item *it);

#define UG_ICU_ARR_LENGTH  256
#define UG_DATE_FORMAT_12  "yMMMdEhhmms"
#define UG_DATE_FORMAT_24  "yMMMdEHHmms"

/* for language instance */
CertSvcCertificate certInstance;

#define NUMBER_OF_CERT_FIELDS 12
static const CertSvcCertificateField field_name[NUMBER_OF_CERT_FIELDS] = {
	CERTSVC_SUBJECT_COMMON_NAME,
	CERTSVC_SUBJECT_ORGANIZATION_NAME,
	CERTSVC_ISSUER_COMMON_NAME,
	CERTSVC_ISSUER_ORGANIZATION_NAME,
	CERTSVC_VERSION,
	CERTSVC_SUBJECT, /* Filler for CERTSVC_VALID_TO */
	CERTSVC_SUBJECT, /* Filler for CERTSVC_VALID_FROM */
	CERTSVC_SERIAL_NUMBER,
	CERTSVC_SIGNATURE_ALGORITHM,
	CERTSVC_KEY_USAGE,
	CERTSVC_SUBJECT, /* Filler for "is CA certificate?" */
	CERTSVC_KEY
};

typedef enum __cert_info_group {
	OWNER = 0,
	ISSUER,
	DATA
} cert_info_group_e;

static Elm_Genlist_Item_Class itc_group;
static Elm_Genlist_Item_Class itc_2text;

static char *_get_icu_time_string(const char *locale, const char *customSkeleton, const char *timezone, UDate date)
{
	/*
		Copy a byte string encoded in the default codepage to a ustring.
		Copies at most n characters. The result will be null terminated if the length of src is less than n. Performs a host byte to UChar conversion
	*/
	UChar ucustomSkeleton[UG_ICU_ARR_LENGTH] = {0,};

	if (u_uastrncpy(ucustomSkeleton, customSkeleton, UG_ICU_ARR_LENGTH) == NULL) {
		LOGE("u_uastrncpy() error.");
		return NULL;
	}

	UChar utimezone[UG_ICU_ARR_LENGTH] = {0,};

	if (u_uastrncpy(utimezone, timezone, UG_ICU_ARR_LENGTH) == NULL) {
		LOGE("u_uastrncpy() error.");
		return NULL;
	}

	UErrorCode status = U_ZERO_ERROR;
	ucal_setDefaultTimeZone(utimezone, &status);

	if (U_FAILURE(status)) {
		LOGE("ucal_setDefaultTimeZone() is failed");
		return NULL;
	}

	uloc_setDefault(secure_getenv("LC_TIME"), &status);

	if (U_FAILURE(status)) {
		LOGE("ucal_setDefaultTimeZone() is failed");
		return NULL;
	}

	UDateTimePatternGenerator *generator = udatpg_open(locale, &status);
	if (generator == NULL)
		return NULL;

	UChar bestPattern[UG_ICU_ARR_LENGTH] = {0,};
	int32_t bestPatternLength = udatpg_getBestPattern(generator, ucustomSkeleton, u_strlen(ucustomSkeleton), bestPattern, UG_ICU_ARR_LENGTH, &status);
	if (bestPatternLength <= 0)
		return NULL;

	UDateFormat *formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, locale, NULL, -1, bestPattern, -1, &status);
	if (formatter == 0)
		return NULL;

	UChar formatted[UG_ICU_ARR_LENGTH] = {0,};
	int32_t formattedLength = udat_format(formatter, date, formatted, UG_ICU_ARR_LENGTH, NULL, &status);
	if (formattedLength <= 0)
		return NULL;

	char formattedString[UG_ICU_ARR_LENGTH] = {0,};
	u_austrcpy(formattedString, formatted);
	udatpg_close(generator);
	udat_close(formatter);

	if (strlen(formattedString) == 0)
		return NULL;

	return strdup(formattedString);
}

static char *_get_icu_date(time_t mtime)
{
	char *skeleton = NULL;
	bool hours_24 = false;
	int ret = -1;

	ret = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &hours_24);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		LOGE("Cannot get 24 hours format");
		/* default 12 */
		hours_24 = false;
	}

	if (hours_24 == true)
		skeleton = UG_DATE_FORMAT_24;
	else
		skeleton = UG_DATE_FORMAT_12;

	/* eg. en_US.UTF-8*/
	char *locale = vconf_get_str(VCONFKEY_REGIONFORMAT);
	if (locale == NULL) {
		LOGE("Cannot get region format.");
		/* default value : en_US */
		locale = strdup("en_US");
	}

	char *timezone = vconf_get_str(VCONFKEY_SETAPPL_TIMEZONE_ID);
	if (timezone == NULL) {
		/* TODO : How to get default time zone? */
		LOGE("Cannot get time zone");
		free(locale);

		return NULL;
	}

	LOGD("Locale:%s TimeZone:%s TimeFormat:%s", locale, skeleton, timezone);

	char *datestr = _get_icu_time_string(locale, skeleton, timezone, (UDate)mtime * 1000);

	free(timezone);
	free(locale);

	if (datestr == NULL) {
		LOGE("Cannot get time string");
		return NULL;
	}

	return datestr;
}

/*
 * This function fill Cert_Data_Field with proper text.
 * These texts are labels to certificate details
 */
static char *_getCertFieldLabel(int index)
{
	const char *cert_data_field = NULL;
	switch (index) {
	case 0:
		cert_data_field = "IDS_ST_BODY_COMMON_NAME_C";
		break;
	case 1:
		cert_data_field = "IDS_ST_BODY_ORGANISATION_C";
		break;
	case 2:
		cert_data_field = "IDS_ST_BODY_COMMON_NAME_C";
		break;
	case 3:
		cert_data_field = "IDS_ST_BODY_ORGANISATION_C";
		break;
	case 4:
		cert_data_field = "IDS_ST_BODY_VERSION_C";
		break;
	case 5:
		cert_data_field = "IDS_ST_BODY_VALID_FROM_C";
		break;
	case 6:
		cert_data_field = "IDS_ST_BODY_VALID_TO_C";
		break;
	case 7:
		cert_data_field = "IDS_ST_BODY_SERIAL_NUMBER_COLON";
		break;
	case 8:
		cert_data_field = "IDS_ST_BODY_SIGNATURE_ALGORITHM_C";
		break;
	case 9:
		cert_data_field = "IDS_ST_BODY_KEY_USAGE_C";
		break;
	case 10:
		cert_data_field = "IDS_ST_BODY_CERTIFICATION_AUTHORITY_C";
		break;
	case 11:
		cert_data_field = "IDS_ST_BODY_PUBLIC_KEY_C";
		break;
	default:
		LOGE("Invalid index of cert field");
		return NULL;
	}

	return strdup(dgettext(PACKAGE, cert_data_field));
}

static void put_no_data_text_if_empty(char **text)
{
	if (text == NULL) {
		return;
	} else if (*text == NULL) {
		*text = strdup(dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA"));
	} else if (strlen(*text) == 0) {
		free(*text);
		*text = strdup(dgettext(PACKAGE, "IDS_ST_BODY_NO_DATA"));
	}
}

static char *getInfoFromCert(CertSvcCertificate cert, CertSvcCertificateField field, char **buffer)
{
	CertSvcString certSvcString_buffer;
	const char *certSvc_buffer;
	size_t len;

	certsvc_certificate_get_string_field(cert, field, &certSvcString_buffer);
	certsvc_string_to_cstring(certSvcString_buffer, &certSvc_buffer, &len);
	*buffer = strndup(certSvc_buffer, len);
	certsvc_string_free(certSvcString_buffer);
	LOGD("Cert string field: %s", *buffer);

	return *buffer;
}

static char *_getCertFieldData(int index)
{
	LOGD("fillCertData");
	time_t time;
	int status = -1;
	CertSvcString buffer;
	const char *char_buffer = NULL;
	size_t char_buffer_len = 0;
	char *cert_Data = NULL;

	switch (index) {
	case 5:
		certsvc_certificate_get_not_before(certInstance, &time);
		cert_Data = _get_icu_date(time);
		LOGD("Valid from: %s", cert_Data);
		break;

	case 6:
		certsvc_certificate_get_not_after(certInstance, &time);
		cert_Data = _get_icu_date(time);
		LOGD("Valid to: %s", cert_Data);
		break;

	case 10:
		certsvc_certificate_is_root_ca(certInstance, &status);
		if (status == CERTSVC_TRUE)
			cert_Data = strdup(dgettext(PACKAGE, "IDS_ST_BODY_TRUE"));
		else if (status == CERTSVC_FALSE)
			cert_Data = strdup(dgettext(PACKAGE, "IDS_ST_BODY_FALSE"));

		break;

	case 11:
		certsvc_certificate_get_string_field(certInstance, CERTSVC_KEY, &buffer);
		certsvc_string_to_cstring(buffer, &char_buffer, &char_buffer_len);
		if (char_buffer != NULL) {
			LOGD("char_buffer : %s", char_buffer);
			cert_Data = format_key(char_buffer);

			certsvc_string_free(buffer);
			if (cert_Data == NULL)
				SECURE_LOGD("Fail to get cert_Data");
			else
				SECURE_LOGD("Public Key: %s", cert_Data);
		}

		break;

	default:
		/*
		 * ---- SUBJECT COMMON NAME ----
		 * ---- SUBJECT ORGANIZATION NAME ----
		 * ---- ISSUER COMMON NAME ----
		 * ---- ISSUER ORGANIZATION NAME ----
		 * ---- VERSION ----
		 * ---- SERIAL NUMBER ----
		 * ---- SIGNATURE ALGORITHM ----
		 * ---- KEY USAGE ----
		 */
		getInfoFromCert(certInstance, field_name[index], &cert_Data);
		break;
	}

	put_no_data_text_if_empty(&cert_Data);

	return cert_Data;
}

static char *format_key(const char *key)
{
	LOGD("format_key");
	int i = 0;
	int j = 0;
	int len = strlen(key);
	const char *public_key_label = "Public-Key: ";
	char *result;
	char *formated_key = malloc(sizeof(char) * len);

	if (formated_key == NULL)
		return NULL;

	/* remove whitespaces at the beginning */
	while ((i < len - 1) && (' ' == key[i]))
		i++;

	/* remove the "Public-key: " string at the beginning */
	if (strncmp(public_key_label, &(key[i]), strlen(public_key_label)) == 0)
		i += strlen(public_key_label);

	while (i < len - 1) {
		if (key[i] == ':' && key[i + 1] == '\n') {
			formated_key[j] = key[i];
			i += 2;
			j += 1;
		} else if (key[i] == ' ' && key[i + 1] == ' ') {
			i += 2;
		} else {
			formated_key[j++] = key[i++];
		}

		if (key[i] == 'E' || key[i] == 'M') {
			formated_key[j++] = '<';
			formated_key[j++] = 'b';
			formated_key[j++] = 'r';
			formated_key[j++] = '>';
		}
	}

	if (i < len && key[i] != '\n') {
		formated_key[j++] = key[i++];
	}

	result = strndup(formated_key, j);
	free(formated_key);
	return result;
}

static void _gl_realized(void *data, Evas_Object *obj, void *ei)
{
	int id = (int)elm_object_item_data_get(ei);

	if (ei == NULL)
		return;
	else if ((id == 0) || (id == 2) || (id == 4))
		elm_object_item_signal_emit(ei, "elm,state,top", "");
	else if ((id == 1) || (id == 3) || (id == 11))
		elm_object_item_signal_emit(ei, "elm,state,bottom", "");
	else
		elm_object_item_signal_emit(ei, "elm,state,center", "");
}

static char *_gl_get_text(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	char *cert_data = NULL;

	if (strcmp(part, "elm.text") == 0)
		cert_data = _getCertFieldLabel(index);
	else if (strcmp(part, "elm.text.multiline") == 0)
		cert_data = _getCertFieldData(index);

	LOGD("%s", cert_data);
	return cert_data;
}

static char *_gl_get_text_group(void *data, Evas_Object *obj, const char *part)
{
	cert_info_group_e group = (cert_info_group_e)data;
	const char *text = NULL;

	if (safeStrCmp(part, "elm.text") != 0)
		return NULL;

	switch (group) {
	case OWNER:
		text = "IDS_SCP_BODY_OWNER_ABB";
		break;

	case ISSUER:
		text = "IDS_ST_BODY_ISSUER";
		break;

	case DATA:
		text = "IDS_ST_BODY_CERTIFICATE_INFORMATION";
		break;

	default:
		LOGE("Wrong *index - return NULL");
		return NULL;
	}

	return strdup(dgettext(PACKAGE, text));
}

void show_detail_info(void)
{
	int i = 0;
	Elm_Object_Item *it = NULL;
	Evas_Object *genlist;
	struct ug_data *ad = get_ug_data();

	itc_group.item_style       = "group_index";
	itc_group.func.text_get    = _gl_get_text_group;
	itc_group.func.content_get = NULL;
	itc_group.func.state_get   = NULL;
	itc_group.func.del         = NULL;

	itc_2text.item_style       = "multiline";
	itc_2text.func.text_get    = _gl_get_text;
	itc_2text.func.content_get = NULL;
	itc_2text.func.state_get   = NULL;
	itc_2text.func.del         = NULL;

	if (!ad)
		return;

	if (!ad->more_popup2)
		ad->more_popup2 = (Evas_Object *)(-1);

	genlist = common_genlist(ad->win_main);
	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);
	evas_object_smart_callback_add(genlist, "selected", genlist_clicked_cb, NULL);

	for (i = 0; i < NUMBER_OF_CERT_FIELDS; i++) {
		if (field_name[i] == CERTSVC_SUBJECT_COMMON_NAME)
			it = elm_genlist_item_append(genlist, &itc_group, (void *)OWNER, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		else if (field_name[i] == CERTSVC_ISSUER_COMMON_NAME)
			it = elm_genlist_item_append(genlist, &itc_group, (void *)ISSUER, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		else if (field_name[i] == CERTSVC_VERSION)
			it = elm_genlist_item_append(genlist, &itc_group, (void *)DATA, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		it = elm_genlist_item_append(genlist, &itc_2text, (void *)i, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		LOGD("Succesful append %d genlist", i);
	}

	LOGD("Push genlist");
	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->navi_bar, "IDS_ST_BODY_CERTIFICATE_DETAILS", common_back_btn(ad), NULL, genlist, NULL);
	elm_object_item_domain_text_translatable_set(nf_it, PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, _back, (struct Evas_Object *)ad);
}

void get_info_cert_from_file_cb(struct ug_data *ad, void *list)
{
	LOGD("get_info_cert_from_file_cb()");
	struct ListElement *current = (struct ListElement *)list;
	if (certsvc_pkcs12_get_certificate_from_store(
			ad->instance,
			current->storeType,
			current->gname,
			&certInstance) != CERTSVC_SUCCESS) {
		LOGE("Unable to load certificate information into the certInstance.");
		return;
	}
	show_detail_info();

	free(ad->data);
	ad->data = NULL;
}

void get_info_cert_from_certificate_cb(CertSvcCertificate cert)
{
	LOGD("get_info_cert_from_certificate_cb()");

	certInstance = cert;

	show_detail_info();
}

Eina_Bool _back(void *data, Elm_Object_Item *it)
{
	struct ug_data *ad = (struct ug_data *) data;
	ad->more_popup2 = NULL;
	certsvc_certificate_free(certInstance);
	memset(&certInstance, 0, sizeof(certInstance));
	return EINA_TRUE;
}
