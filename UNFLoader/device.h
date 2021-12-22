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


    #ifdef LINUX
        typedef int errno_t;
    #endif


    typedef struct {
        int cictype;
        u32 savetype;
        int z64;
    } device_sendrom_params_t;

    typedef void(*device_fatal_error_callback_t)(const char* message);

    /*********************************
            Function Prototypes
    *********************************/

    void  device_sendrom_params_init(device_sendrom_params_t* params);
    void  device_set_fatal_error_callback(device_fatal_error_callback_t callback);
    void  device_find(int automode);
    void  device_open();
    void  device_sendrom(const char* rompath, device_sendrom_params_t* params);
    void  device_senddata(int datatype, char* data, u32 size);
    bool  device_isopen();
    DWORD device_getcarttype();
    void  device_close();

    DWORD device_get_pending();
    u32 device_begin_read();
    DWORD device_read(char* buffer, int size);
    void device_end_read();

#endif