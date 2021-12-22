#ifndef __DEVICE_EVERDRIVE_HEADER
#define __DEVICE_EVERDRIVE_HEADER

    #include "device.h"
    #include "device_context.h"


    /*********************************
            Function Prototypes
    *********************************/

    bool device_test_everdrive(ftdi_context_t* cart, int index);
    void device_open_everdrive(ftdi_context_t* cart);
    void device_sendrom_everdrive(ftdi_context_t* cart, FILE *file, u32 size, device_sendrom_params_t* params);
    void device_senddata_everdrive(ftdi_context_t* cart, int datatype, char *data, u32 size);
    void device_close_everdrive(ftdi_context_t* cart);

#endif