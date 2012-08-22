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


#include "mgr-app-common-util.h"
#include "mgr-app-common-debug.h"
#include <string.h>

char *_strncpy(char *dest, const char *src, int len, char *file, int line)
{
	if (dest == NULL || src == NULL || len <= 0) {
		MGR_APP_DEBUG_ERR("STRNCPY ERROR at %s(%d)", file, line);
		return dest;
	}

	strncpy(dest, src, len);
	dest[len] = '\0';

	return dest;
}

char *_strncat(char *dest, const char *src, int dest_size, char *file, int line)
{
	if (dest == NULL || src == NULL || dest_size <= 0) {
		MGR_APP_DEBUG_ERR("STRNCAT ERROR at %s(%d)", file, line);
		return dest;
	}

	return strncat(dest, src, dest_size);
}

int _strncmp(const char *src1, const char *src2, int len, char *file, int line)
{
	if (src1 == NULL || src2 == NULL || len <= 0) {
		MGR_APP_DEBUG_ERR("STRNCMP ERROR at %s(%d)", file, line);
		return -1;
	}

	return strncmp(src1, src2, len);
}


