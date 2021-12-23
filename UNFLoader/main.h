#ifndef __MAIN_HEADER
#define __MAIN_HEADER

    #pragma warning(push, 0)

        #ifndef LINUX
            #include "Include/curses.h"
            #include "Include/curspriv.h"
            #include "Include/panel.h"
        #else
            #include <locale.h>
            #include <curses.h>
        #endif
        #include "Include/ftd2xx.h"
    #pragma warning(pop)

    #include "types.h"


    /*********************************
                  Macros
    *********************************/

    #define false 0
    #define true  1

    #define CH_ESCAPE    27
    #define CH_ENTER     '\n'
    #define CH_BACKSPACE '\b'


    /*********************************
                 Globals
    *********************************/

    extern bool    global_usecolors;
    extern bool    global_listenmode;
    extern bool    global_debugmode;
    extern char*   global_debugout;
    extern FILE*   global_debugoutptr;
    extern char*   global_exportpath;
    extern time_t  global_timeout;
    extern time_t  global_timeouttime;
    extern bool    global_closefail;
    extern char*   global_filename;
    extern WINDOW* global_window;

#endif