#ifndef D_ALLOCA_H
#define D_ALLOCA_H

#if defined(_WIN32)
    #include <malloc.h>
    #define alloca _alloca
#else
    #include  <alloca.h>
#endif

#endif