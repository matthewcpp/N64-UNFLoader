#ifndef __DEVICE_HEADER
#define __DEVICE_HEADER

#include <stdint.h>


    /*********************************
                  Macros
    *********************************/

    #define CART_NONE      0
    #define CART_ANY       0
    #define CART_64DRIVE1  1
    #define CART_64DRIVE2  2
    #define CART_EVERDRIVE 3
    #define CART_SC64      4


    // Data types defintions
    #define DATATYPE_TEXT       0x01
    #define DATATYPE_RAWBINARY  0x02
    #define DATATYPE_HEADER     0x03
    #define DATATYPE_SCREENSHOT 0x04


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

#ifdef SHARED_DEVICE_LIB
    #ifdef BUILD_SHARED_DEVICE_LIB
        #define N64_DEVICE_API __declspec(dllexport)
        #else
        #define N64_DEVICE_API __declspec(dllimport)
    #endif
#else
    #define N64_DEVICE_API
#endif


   
#ifdef __cplusplus
extern "C" {
#endif

    void N64_DEVICE_API device_sendrom_params_init(device_sendrom_params_t* params);
    void N64_DEVICE_API device_set_fatal_error_callback(device_message_callback_t callback);
    void N64_DEVICE_API device_set_sendrom_progress_callback(device_transfer_progress_callback_t callback);
    void N64_DEVICE_API device_set_senddata_progress_callback(device_transfer_progress_callback_t callback);
    void N64_DEVICE_API device_set_message_callback(device_message_callback_t callback);
    void N64_DEVICE_API device_find(int automode);
    void N64_DEVICE_API device_open();
    void N64_DEVICE_API device_sendrom(const char* rompath, device_sendrom_params_t* params);
    void N64_DEVICE_API device_senddata(int datatype, char* data, uint32_t size);
    bool N64_DEVICE_API device_isopen();
    uint32_t N64_DEVICE_API device_getcarttype();
    void N64_DEVICE_API device_close();

    uint32_t N64_DEVICE_API device_get_pending();
    uint32_t N64_DEVICE_API device_begin_read();
    uint32_t N64_DEVICE_API device_read(char* buffer, int size);
    void N64_DEVICE_API device_end_read();

    
#ifdef __cplusplus
}
#endif

#endif