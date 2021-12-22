#ifndef __DEVICE_HEADER
#define __DEVICE_HEADER


    /*********************************
                  Macros
    *********************************/

    #define CART_NONE      0
    #define CART_64DRIVE1  1
    #define CART_64DRIVE2  2
    #define CART_EVERDRIVE 3
    #define CART_SC64      4


    /*********************************
                 Typedefs
    *********************************/

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
    #ifdef LINUX
        typedef int errno_t;
    #endif


    /*********************************
            Function Prototypes
    *********************************/

    void  device_find(int automode);
    void  device_open();
    void  device_sendrom(FILE* f, int filesize);
    void  device_senddata(int datatype, char* data, u32 size);
    bool  device_isopen();
    DWORD device_getcarttype();
    void  device_close();

    DWORD device_get_pending();
    u32 device_begin_read();
    DWORD device_read(char* buffer, int size);
    void device_end_read();

    ftdi_context_t* device_get_cart();

#endif