/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/




#ifndef __DEF_MF_UG_DLOG_H_
#define __DEF_MF_UG_DLOG_H_

#include <stdio.h>
#include <string.h>

#define DLOG_ON 1

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG			"MYFILE-UG"
#include <dlog.h>


#if DLOG_ON
#define LOG_COLOR_RED      "\033[31m"
#define LOG_COLOR_RESET    "\033[0m"

#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define SECURE_DEBUG(fmt, args...)	dlog_print(DLOG_DEBUG, "[%s][%d]Secure debug message from ug-myfile-efl is : "fmt"\n", __func__, __LINE__, ##args)//SECURE_LOGD(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##args)
#define SECURE_INFO(fmt, args...)	dlog_print(DLOG_INFO, "[%s][%d]Secure info message from ug-myfile-efl is : "fmt"\n", __func__, __LINE__, ##args)//SECURE_LOGI(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##args)
#define SECURE_ERROR(fmt, args...)	dlog_print(DLOG_ERROR, "[%s][%d]Secure Error message from ug-myfile-efl is : "fmt"\n", __func__, __LINE__, ##args)//SECURE_LOGE(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##args)

#define ug_debug(fmt , args...)        dlog_print(DLOG_DEBUG, "[%s][%d]debug message from ug-myfile-efl is : "fmt"\n", __func__, __LINE__, ##args)
#define ug_myfile_dlog(fmt , args...)        dlog_print(DLOG_DEFAULT, "[%s][%d]debug message from myfile is : "fmt"\n", __func__, __LINE__, ##args)
#define UG_TRACE_BEGIN do {\
					{\
						dlog_print(DLOG_INFO, "\n\033[0;35mENTER FUNCTION: %s. \033[0m\t%s:%d\n", \
						__FUNCTION__, (char *)(strrchr(__FILE__, '/') + 1), __LINE__);\
					} \
				} while (0);

#define UG_TRACE_END  do {\
					{\
						dlog_print(DLOG_INFO, "\n\033[0;35mEXIT FUNCTION: %s. \033[0m\t%s:%d\n", \
						__FUNCTION__, (char *)(strrchr(__FILE__, '/') + 1), __LINE__);\
					} \
				} while (0) ;
#define ug_error(fmt, arg...)	dlog_print(DLOG_ERROR, LOG_COLOR_RED"[ %s : %d]   "fmt""LOG_COLOR_RESET, __FUNCTION__, __LINE__,##arg)


#else
#define ug_myfile_dlog(fmt , args...)        printf("[MYFILE][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args)
#endif
#define ug_mf_debug(fmt , args...)			do { (void)0; } while (0)
#define ug_mf_warnig(fmt , args...)			do { (void)0; } while (0)
#define ug_mf_error(fmt , args...)			do { (void)0; } while (0)
#define UG_MYFILE_TRACE_ERROR(fmt, arg...)		do { dlog_print(DLOG_ERROR, "[%s][%d] "fmt"\n", strrchr(__FILE__, '/') + 1, __LINE__, ##arg); } while (0)

#define ug_mf_retvm_if(expr, val, fmt, arg...) do { \
			if (expr) { \
				UG_MYFILE_TRACE_ERROR(fmt, ##arg); \
				return (val); \
			} \
		} while (0)

#define ug_mf_retv_if(expr, val) do { \
			if (expr) { \
				return (val); \
			} \
		} while (0)


#define ug_mf_retm_if(expr, fmt, arg...) do { \
			if (expr) { \
				UG_MYFILE_TRACE_ERROR(fmt, ##arg); \
				return; \
			} \
		} while (0)

#define MF_CHECK(expr) 				ug_mf_retm_if(!(expr),"INVALID PARAM RETURN")
#define MF_CHECK_FALSE(expr)			ug_mf_retvm_if(!(expr), false, "INVALID PARAM RETURN FALSE")
#define MF_CHECK_NULL(expr)			ug_mf_retvm_if(!(expr), NULL, "INVALID PARAM RETURN NULL")
#define MF_CHECK_VAL(expr, val) 		ug_mf_retvm_if(!(expr),val,"INVALID PARM RETURN val:0x%x", val)

#endif
