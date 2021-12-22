/***************************************************************
                           device.cpp
                               
Passes flashcart communication to more specific functions
***************************************************************/


#include "main.h"
#include "helper.h"
#include "device.h"
#include "device_64drive.h"
#include "device_everdrive.h"
#include "device_sc64.h"


/*********************************
        Function Pointers
*********************************/

void (*funcPointer_open)(ftdi_context_t*);
void (*funcPointer_sendrom)(ftdi_context_t*, FILE *file, u32 size);
void (*funcPointer_senddata)(ftdi_context_t*, int datatype, char *data, u32 size);
void (*funcPointer_close)(ftdi_context_t*);


/*********************************
             Globals
*********************************/

static ftdi_context_t local_usb = {0, };

/*==============================
    device_find
    Returns the cart context
==============================*/
ftdi_context_t* device_get_cart() {
    return &local_usb;
}


/*==============================
    device_find
    Finds the flashcart plugged in to USB
    @param The cart to check for (CART_NONE for automatic checking)
==============================*/

void device_find(int automode)
{
    int i;
    ftdi_context_t *cart = &local_usb;

    // Initialize FTD
    if (automode == CART_NONE)
        pdprint("Attempting flashcart autodetection.\n", CRDEF_PROGRAM);
    testcommand(FT_CreateDeviceInfoList(&cart->devices), "USB Device not ready.");

    // Check if the device exists
    if (cart->devices == 0)
        terminate("No devices found.");

    // Allocate storage and get device info list
    cart->dev_info = (FT_DEVICE_LIST_INFO_NODE*) malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*cart->devices);
    FT_GetDeviceInfoList(cart->dev_info, &cart->devices);

    // Search the devices
    for(i=0; (unsigned)i < cart->devices; i++)
    {
        // Look for 64drive HW1 (FT2232H Asynchronous FIFO mode)
        if ((automode == CART_NONE || automode == CART_64DRIVE1) && device_test_64drive1(cart, i))
        {
            device_set_64drive1(cart, i);
            if (automode == CART_NONE) 
                pdprint_replace("64Drive HW1 autodetected!\n", CRDEF_PROGRAM);
            break;
        }

        // Look for 64drive HW2 (FT232H Synchronous FIFO mode)
        if ((automode == CART_NONE || automode == CART_64DRIVE2) && device_test_64drive2(cart, i))
        {
            device_set_64drive2(cart, i);
            if (automode == CART_NONE) 
                pdprint_replace("64Drive HW2 autodetected!\n", CRDEF_PROGRAM);
            break;
        }

        // Look for an EverDrive
        if ((automode == CART_NONE || automode == CART_EVERDRIVE) && device_test_everdrive(cart, i))
        {
            device_set_everdrive(cart, i);
            if (automode == CART_NONE)
                pdprint_replace("EverDrive autodetected!\n", CRDEF_PROGRAM);
            break;
        }

        // Look for SummerCart64
        if ((automode == CART_NONE || automode == CART_SC64) && device_test_sc64(cart, i))
        {
            device_set_sc64(cart, i);
            if (automode == CART_NONE)
                pdprint_replace("SummerCart64 autodetected!\n", CRDEF_PROGRAM);
            break;
        }
    }

    // Finish
    free(cart->dev_info);
    if (cart->carttype == CART_NONE)
    {
        if (automode == CART_NONE)
        {
            #ifdef LINUX
                terminate("No flashcart detected. Are you running sudo?");
            #else
                terminate("No flashcart detected.");
            #endif
        }
        else
            terminate("Requested flashcart not found.");
    }
}


/*==============================
    device_set_64drive1
    Marks the cart as being 64Drive HW1
    @param A pointer to the cart context
    @param The index of the cart
==============================*/

void device_set_64drive1(ftdi_context_t* cart, int index)
{
    // Set cart settings
    cart->device_index = index;
    cart->synchronous = 0;
    cart->carttype = CART_64DRIVE1;

    // Set function pointers
    funcPointer_open = &device_open_64drive;
    funcPointer_sendrom = &device_sendrom_64drive;
    funcPointer_senddata = &device_senddata_64drive;
    funcPointer_close = &device_close_64drive;
}


/*==============================
    device_set_64drive2
    Marks the cart as being 64Drive HW2
    @param A pointer to the cart context
    @param The index of the cart
==============================*/

void device_set_64drive2(ftdi_context_t* cart, int index)
{
    // Do exactly the same as device_set_64drive1
    device_set_64drive1(cart, index);

    // But modify the important cart settings
    cart->synchronous = 1;
    cart->carttype = CART_64DRIVE2;
}


/*==============================
    device_set_everdrive
    Marks the cart as being EverDrive
    @param A pointer to the cart context
    @param The index of the cart
==============================*/

void device_set_everdrive(ftdi_context_t* cart, int index)
{
    // Set cart settings
    cart->device_index = index;
    cart->carttype = CART_EVERDRIVE;

    // Set function pointers
    funcPointer_open = &device_open_everdrive;
    funcPointer_sendrom = &device_sendrom_everdrive;
    funcPointer_senddata = &device_senddata_everdrive;
    funcPointer_close = &device_close_everdrive;
}


/*==============================
    device_set_sc64
    Marks the cart as being SummerCart64
    @param A pointer to the cart context
    @param The index of the cart
==============================*/

void device_set_sc64(ftdi_context_t* cart, int index)
{
    // Set cart settings
    cart->device_index = index;
    cart->carttype = CART_SC64;

    // Set function pointers
    funcPointer_open = &device_open_sc64;
    funcPointer_sendrom = &device_sendrom_sc64;
    funcPointer_senddata = &device_senddata_sc64;
    funcPointer_close = &device_close_sc64;
}


/*==============================
    device_open
    Calls the function to open the flashcart
==============================*/

void device_open()
{
    funcPointer_open(&local_usb);
    pdprint("USB connection opened.\n", CRDEF_PROGRAM);
}


/*==============================
    device_sendrom
    Transfers a rom to the flashcart
    @param file handle to ROM
    @param size of rom
==============================*/
void  device_sendrom(FILE* f, int filesize) {
    funcPointer_sendrom(&local_usb, f, filesize);
}


/*==============================
    device_senddata
    Sends data to the flashcart via USB
    @param The data to send
    @param The number of bytes in the data
    @returns 1 if success, 0 if failure
==============================*/

void device_senddata(int datatype, char* data, u32 size)
{
    funcPointer_senddata(&local_usb, datatype, data, size);
}


/*==============================
    device_isopen
    Checks if the device is open
    @returns Whether the device is open
==============================*/

bool device_isopen()
{
    ftdi_context_t* cart = &local_usb;
    return (cart->handle != NULL);
}


/*==============================
    device_getcarttype
    Returns the type of the connected cart
    @returns The cart type
==============================*/

DWORD device_getcarttype()
{
    ftdi_context_t* cart = &local_usb;
    return cart->carttype;
}


/*==============================
    device_close
    Calls the function to close the flashcart
==============================*/

void device_close()
{
    // Should never happen, but just in case...
    if (local_usb.handle == NULL)
        return;

    // Close the device
    funcPointer_close(&local_usb);
    pdprint("USB connection closed.\n", CRDEF_PROGRAM);
}

