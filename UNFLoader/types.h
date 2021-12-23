#ifndef __TYPES_HEADER
#define __TYPES_HEADER

#pragma warning(push, 0)
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
    #include <ctype.h>
    #ifndef LINUX
        #include <windows.h> // Needed to prevent a macro redefinition due to curses.h
    #else
        #include <unistd.h>
    #endif

    #include "Include/ftd2xx.h"
#pragma warning(pop)

/*********************************
             Typedefs
*********************************/

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef char           s8;
typedef short          s16;
typedef int            s32;

#endif

