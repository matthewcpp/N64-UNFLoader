#ifndef __DEVICE_HEADER
#define __DEVICE_HEADER

#include <stdint.h>


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
        uint32_t savetype;
        int z64;
    } device_sendrom_params_t;

    typedef void(*device_message_callback_t)(const char* message);
    typedef void(*device_transfer_progress_callback_t)(float progress);

    /*********************************
            Function Prototypes
    *********************************/

    void  device_sendrom_params_init(device_sendrom_params_t* params);
    void  device_set_fatal_error_callback(device_message_callback_t callback);
    void  device_set_sendrom_progress_callback(device_transfer_progress_callback_t callback);
    void  device_set_senddata_progress_callback(device_transfer_progress_callback_t callback);
    void  device_set_message_callback(device_message_callback_t callback);
    void  device_find(int automode);
    void  device_open();
    void  device_sendrom(const char* rompath, device_sendrom_params_t* params);
    void  device_senddata(int datatype, char* data, uint32_t size);
    bool  device_isopen();
    uint32_t device_getcarttype();
    void  device_close();

    uint32_t device_get_pending();
    uint32_t device_begin_read();
    uint32_t device_read(char* buffer, int size);
    void device_end_read();

#endif