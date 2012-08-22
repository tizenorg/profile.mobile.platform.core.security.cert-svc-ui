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


#ifndef __MGR_APP_COMMON_UTIL_H__
#define __MGR_APP_COMMON_UTIL_H__

#include <glib.h>
#include <assert.h>


#define _STRNCPY(DEST, SRC, LEN) 		_strncpy(DEST, SRC, LEN , __FILE__, __LINE__)
#define _STRNCAT(DEST, SRC, DEST_SIZE) 	_strncat(DEST, SRC, DEST_SIZE, __FILE__, __LINE__)
#define _STRNCMP(SRC1, SRC2, LEN)	 	_strncmp(SRC1, SRC2, LEN, __FILE__, __LINE__)

char*	_strncpy(char *dest, const char *src, int len, char *file, int line);
char*	_strncat(char *dest, const char *src, int dest_size, char *file, int line);
int		_strncmp(const char *src1, const char *src2, int len, char *file, int line);


#define MGR_APP_MAX_HEAP_SIZE	5*1024*1024
#define _EDJ(obj) 				elm_layout_edje_get(obj)

#define MGR_APP_MEM_MALLOC(ptr,no_elements,type)	\
		do {\
			if ((gint)(no_elements)<=0) {\
				ptr = NULL;\
			}\
			else if (MGR_APP_MAX_HEAP_SIZE<(gint)(no_elements)*sizeof(type)) {\
				assert(0);\
			}\
			else {\
				ptr=(type*)g_malloc0((gint)(no_elements)*sizeof(type));\
				assert(ptr);\
				MGR_APP_DEBUG("Allocate memory. pointer:%p\n", ptr);\
			}\
		} while (0)

#define MGR_APP_MEM_STRDUP(ptr,str)	\
		do {\
			if ((str) != NULL) {\
				ptr = strdup((const char *)(str));\
				assert(ptr);\
				MGR_APP_DEBUG("Allocate memory. pointer:%p\n", (ptr));\
			}\
			else {\
				(ptr) = NULL;\
			}\
		} while (0)

#define MGR_APP_MEM_STRNDUP(ptr,str,buf_size)\
		do {\
			if ((str) != NULL && (buf_size) >= 0 ) {\
				ptr = strndup((const char *)(str),(buf_size));\
				assert(ptr);\
				MGR_APP_DEBUG("Allocate memory. pointer:%p\n", (ptr));\
			}\
			else {\
				(ptr) = NULL;\
			}\
		} while (0)

#define MGR_APP_MEM_FREE(ptr)	\
		do {\
			if (ptr != NULL) {\
				MGR_APP_DEBUG("Free memory. pointer:%p\n", ptr);\
				g_free(ptr);\
				ptr = NULL;\
			}\
		} while (0)

#endif	/*__MGR_APP_COMMON_UTIL_H__*/

