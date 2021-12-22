#ifndef __DEVICE_CONTEXT_HEADER
#define __DEVICE_CONTEXT_HEADER

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

typedef struct {
    DWORD        devices;
    int          device_index;
    FT_STATUS    status;
    FT_DEVICE_LIST_INFO_NODE *dev_info;
    FT_HANDLE    handle;
    DWORD        synchronous; // For 64Drive
    DWORD        bytes_written;
    DWORD        bytes_read;
    DWORD        carttype;
    DWORD        cictype;
    u32          current_dma_bytes_read; // the total amount of bytes read in the current message
} ftdi_context_t;

#endif