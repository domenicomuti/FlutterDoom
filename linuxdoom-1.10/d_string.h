#ifndef D_STRING_H
#define D_STRING_H

#include <string.h>

#if defined(_WIN32)
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    #define strcmpi _strcmpi
    #define strupr _strupr
#else
    #define strcmpi	strcasecmp
#endif

#endif