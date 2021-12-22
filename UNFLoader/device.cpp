/***************************************************************
                           device.cpp
                               
Passes flashcart communication to more specific functions
***************************************************************/


#include "main.h"
#include "device_context.h"
#include "helper.h"
#include "device.h"
#include "device_64drive.h"
#include "device_everdrive.h"
#include "device_sc64.h"




/*********************************
        Function Pointers
*********************************/

void (*funcPointer_open)(ftdi_context_t*);
void (*funcPointer_sendrom)(ftdi_context_t*, FILE *file, u32 size, device_sendrom_params_t* params);
void (*funcPointer_senddata)(ftdi_context_t*, int datatype, char *data, u32 size);
void (*funcPointer_close)(ftdi_context_t*);


/*********************************
        Function Prototypes
*********************************/
static void  device_set_64drive1(ftdi_context_t* cart, int index);
static void  device_set_64drive2(ftdi_context_t* cart, int index);
static void  device_set_everdrive(ftdi_context_t* cart, int index);
static void  device_set_sc64(ftdi_context_t* cart, int index);


/*********************************
             Globals
*********************************/

static ftdi_context_t local_usb = {0, };
static device_fatal_error_callback_t fatal_error_callback = NULL;
static device_transfer_progress_callback_t sendrom_progress_callback = NULL;
static device_transfer_progress_callback_t senddata_progress_callback = NULL;

void device_set_fatal_error_callback(device_fatal_error_callback_t callback)
{
    fatal_error_callback = callback;
}

void device_set_sendrom_progress_callback(device_transfer_progress_callback_t callback)
{
    sendrom_progress_callback = callback;
}

void device_set_senddata_progress_callback(device_transfer_progress_callback_t callback)
{
    senddata_progress_callback = callback;
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
void device_sendrom(const char* rompath, device_sendrom_params_t* params) 
{
    FILE*  file = fopen(rompath, "rb");

    if (file == NULL)
    {
        device_close();
        terminate("Unable to open file '%s'.\n", rompath);
    }

    int filesize = 0;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    funcPointer_sendrom(&local_usb, file, filesize, params);

    fclose(file);
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


/*==============================
    device_get_pending
    Returns the amount of data in the device queue
==============================*/
DWORD device_get_pending()
{
    DWORD pending = 0;
    FT_GetQueueStatus(local_usb.handle, &pending);

    return pending > 0;
}

/*==============================
    device_begin_read
    Begins reading from the device and validates the dma header
    Returns the header info value
==============================*/
u32 device_begin_read()
{
    ftdi_context_t* cart = &local_usb;
    char outbuff[4];
    cart->current_dma_bytes_read = 0;

    // Ensure we have valid data by reading the header
    FT_Read(cart->handle, &outbuff[0], 4, &cart->bytes_read);
    cart->current_dma_bytes_read += cart->bytes_read;
    if (outbuff[0] != 'D' || outbuff[1] != 'M' || outbuff[2] != 'A' || outbuff[3] != '@')
        terminate("Unexpected DMA header: %c %c %c %c.", outbuff[0], outbuff[1], outbuff[2], outbuff[3]);

    // Get information about the incoming data
    FT_Read(cart->handle, outbuff, 4, &cart->bytes_read);
    cart->current_dma_bytes_read += cart->bytes_read;
    
    return swap_endian(outbuff[3] << 24 | outbuff[2] << 16 | outbuff[1] << 8 | outbuff[0]);
}

/*==============================
    device_end_read
    Ends a current DMA read by validating completion signal and updating alignment
    Returns the header info value
==============================*/
void device_end_read()
{
    ftdi_context_t* cart = &local_usb;
    char outbuff[16];

    // Decide the alignment based off the cart that's connected
    int alignment;
    switch (cart->carttype)
    {
        case CART_EVERDRIVE: alignment = 16; break;
        case CART_SC64: alignment = 4; break;
        default: alignment = 0;
    }

    // Read the completion signal
    FT_Read(cart->handle, &outbuff[0], 4, &cart->bytes_read);
    cart->current_dma_bytes_read += cart->bytes_read;
    if (outbuff[0] != 'C' || outbuff[1] != 'M' || outbuff[2] != 'P' || outbuff[3] != 'H')
        terminate("Did not receive completion signal: %c %c %c %c.", outbuff[0], outbuff[1], outbuff[2], outbuff[3]);

    // Ensure byte alignment by reading X amount of bytes needed
    if (alignment != 0 && (cart->current_dma_bytes_read % alignment) != 0)
    {
        int left = alignment - (cart->current_dma_bytes_read % alignment);
        FT_Read(cart->handle, &outbuff[0], left, &cart->bytes_read);
    }
}

/*==============================
    device_read
    Begins reading from the device and validates the dma header
    Returns the header info value
==============================*/
DWORD device_read(char* buffer, int size)
{
    ftdi_context_t* cart = &local_usb;

    FT_Read(cart->handle, buffer, size, &cart->bytes_read);
    cart->current_dma_bytes_read += cart->bytes_read;

    return cart->bytes_read;
}

void device_sendrom_params_init(device_sendrom_params_t* params) {
    params->cictype = 1;
    params->savetype = 0;
    params->z64 = 0;
}

/*==============================
    testcommand
    Terminates the program if the command fails
    @param The return value from an FTDI function
    @param Text to print if the command failed
    @param Variadic arguments to print as well
==============================*/

void testcommand(FT_STATUS status, const char* reason, ...)
{
    // Test the command
    if (status == FT_OK || !fatal_error_callback)
        return;

    char buffer[512];

    va_list args;
    va_start(args, reason);

    vsprintf_s(&buffer[0], 512, reason, args);
    fatal_error_callback(&buffer[0]);
        
    va_end(args);
}

void sendrom_progress(float percent)
{
    if (sendrom_progress_callback)
        sendrom_progress_callback(percent);
}

void senddata_progress(float percent) {
    if (senddata_progress_callback) {
        senddata_progress_callback(percent);
    }
}