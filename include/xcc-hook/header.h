﻿#ifndef			_XCC_HOOK_HEADER_H_
#define			_XCC_HOOK_HEADER_H_

#include <xcc-posix/xcc.h>


// X-Hook export
#if defined(XCC_HOOK_BUILD_STATIC)
#define			_XHOOKAPI_
#else
#if defined(XCC_HOOK_BUILD_LOCAL)
#define			_XHOOKAPI_					XCC_COMPILER_API_EXP
#else
#define			_XHOOKAPI_					XCC_COMPILER_API_IMP
#endif
#endif



// X-Hook macro
#define 		X_HOOK_LOG_TAG					"X-Hook"
#define 		X_HOOK_MAX_CONTEXT				(16)
#define 		X_HOOK_EVENT_IGNORE				(true)
#define 		X_HOOK_EVENT_ACCEPT				(false)


// X-Hook types
typedef			void*						x_hook_context_t;


#endif
