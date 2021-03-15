#ifndef _EZLOG_PLATFORM_COMPATIBILITY_H_
#define _EZLOG_PLATFORM_COMPATIBILITY_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if _MSC_VER
#    include <Windows.h>
#    define sleep(sec) ::Sleep((sec)*1000)
#    define snprintf   _snprintf
#else
#    include <unistd.h>
#    define stricmp(str1, str2) strcasecmp(str1, str2)
#endif // _MSC_VER

#endif // !_EZLOG_PLATFORM_COMPATIBILITY_H_