/***************************************************************
                       device_64drive.cpp

Handles 64Drive HW1 and HW2 USB communication. A lot of the code
here is courtesy of MarshallH's 64Drive USB tool:
http://64drive.retroactive.be/support.php
***************************************************************/

#include "device_64drive.h"


/*********************************
        Function Prototypes
*********************************/

void device_sendcmd_64drive(ftdi_context_t* cart, u8 command, bool reply, u32 numparams, ...);


/*==============================
    device_test_64drive1
    Checks whether the device passed as an argument is 64Drive HW1
    @param A pointer to the cart context
    @param The index of the cart
    @returns true if the cart is a 64Drive HW1, or false otherwise
==============================*/

bool device_test_64drive1(ftdi_context_t* cart, int index)
{
    return (strcmp(cart->dev_info[index].Description, "64drive USB device A") == 0 && cart->dev_info[index].ID == 0x4036010);
}


/*==============================
    device_test_64drive2
    Checks whether the device passed as an argument is 64Drive HW2
    @param A pointer to the cart context
    @param The index of the cart
    @returns true if the cart is a 64Drive HW2, or false otherwise
==============================*/

bool device_test_64drive2(ftdi_context_t* cart, int index)
{
    return (strcmp(cart->dev_info[index].Description, "64drive USB device") == 0 && cart->dev_info[index].ID == 0x4036014);
}


/*==============================
    device_open_64drive
    Opens the USB pipe
    @param A pointer to the cart context
==============================*/

void device_open_64drive(ftdi_context_t* cart)
{
    // Open the cart
    cart->status = FT_Open(cart->device_index, &cart->handle);
    if (cart->status != FT_OK || !cart->handle)
        fatal_error("Unable to open flashcart.");

    // Reset the cart and set its timeouts
    testcommand(FT_ResetDevice(cart->handle), "Unable to reset flashcart.");
    testcommand(FT_SetTimeouts(cart->handle, 5000, 5000), "Unable to set flashcart timeouts.");

    // If the cart is in synchronous mode, enable the bits
    if (cart->synchronous)
    {
        testcommand(FT_SetBitMode(cart->handle, 0xff, FT_BITMODE_RESET), "Unable to set bitmode %d.", FT_BITMODE_RESET);
        testcommand(FT_SetBitMode(cart->handle, 0xff, FT_BITMODE_SYNC_FIFO), "Unable to set bitmode %d.", FT_BITMODE_SYNC_FIFO);
    }

    // Purge USB contents
    testcommand(FT_Purge(cart->handle, FT_PURGE_RX | FT_PURGE_TX), "Unable to purge USB contents.");
}


/*==============================
    device_sendcmd_64drive
    Opens the USB pipe
    @param A pointer to the cart context
    @param The command to send
    @param A bool stating whether a reply should be expected
    @param The number of extra params to send
    @param The extra variadic commands to send
==============================*/

void device_sendcmd_64drive(ftdi_context_t* cart, u8 command, bool reply, u32 numparams, ...)
{
    u8  send_buff[32];
    u32 recv_buff[32];
    va_list params;
    va_start(params, numparams);

    // Clear the buffers
    memset(send_buff, 0, 32);
    memset(recv_buff, 0, 32);

    // Setup the command to send
    send_buff[0] = command;
    send_buff[1] = 0x43;    // C
    send_buff[2] = 0x4D;    // M
    send_buff[3] = 0x44;    // D

    // Append extra arguments to the command if needed
    if (numparams > 0)
        *(u32 *)&send_buff[4] = swap_endian(va_arg(params, u32));
    if (numparams > 1)
        *(u32 *)&send_buff[8] = swap_endian(va_arg(params, u32));
    va_end(params);

    // Write to the cart
    testcommand(FT_Write(cart->handle, send_buff, 4+(numparams*4), &cart->bytes_written), "Unable to write to 64Drive.");
    if (cart->bytes_written == 0)
        fatal_error("No bytes were written to 64Drive.");

    // If the command expects a response
    if (reply)
    {
        // These two instructions do not return a success, so ignore them
        if (command == DEV_CMD_PI_WR_BL || command == DEV_CMD_PI_WR_BL_LONG)
            return;

        // Check that we received the signal that the operation completed
        testcommand(FT_Read(cart->handle, recv_buff, 4, &cart->bytes_read), "Unable to read completion signal.");
        recv_buff[1] = command << 24 | 0x504D43;
        if (memcmp(recv_buff, &recv_buff[1], 4) != 0)
            fatal_error("Did not receive completion signal.");
    }
}

/*==============================
    device_sendrom_64drive
    Sends the ROM to the flashcart
    @param A pointer to the cart context
    @param A pointer to the ROM to send
    @param The size of the ROM
==============================*/

void device_sendrom_64drive(ftdi_context_t* cart, FILE *file, u32 size, device_sendrom_params_t* params)
{
    u32    ram_addr = 0x0;
    int	   bytes_left = size;
    int	   bytes_done = 0;
    int	   bytes_do;
    int	   chunk = 0;
    u8*    rom_buffer = (u8*) malloc(sizeof(u8) * 4*1024*1024);
    DWORD  cmps;

    // Check we managed to malloc
    if (rom_buffer == NULL)
        fatal_error("Unable to allocate memory for buffer.");

    // Handle CIC
    if (params->cictype == -1)
    {
        int cic = -1;
        int j;
        u8* bootcode = (u8*)malloc(4032);
        if (bootcode == NULL)
            fatal_error("Unable to allocate memory for bootcode buffer.");

        // Read the bootcode and store it
        fseek(file, 0x40, SEEK_SET);
        fread(bootcode, 1, 4032, file);
        fseek(file, 0, SEEK_SET);

        // Byteswap if needed
        if (params->z64)
            for (j=0; j<4032; j+=2)
                SWAP(bootcode[j], bootcode[j+1]);

        // Pick the CIC from the bootcode
        cic = cic_from_hash(romhash(bootcode, 4032));
        if (cic != -1)
        {
            // Set the CIC and print it
            cart->cictype = params->cictype;
            device_sendcmd_64drive(cart, DEV_CMD_SETCIC, false, 1, (1 << 31) | cic, 0);
            if (cic == 303)
                fatal_error("The 8303 CIC is not supported through USB");
            const char* cic_str = NULL;
            switch (cic)
            {
                case 0: cic_str = "6101"; break;
                case 1: cic_str = "6102"; break;
                case 2: cic_str = "7101"; break;
                case 3: cic_str = "7102"; break;
                case 4: cic_str = "x103"; break;
                case 5: cic_str = "x105"; break;
                case 6: cic_str = "x106"; break;
                case 7: cic_str = "5101"; break;
            }
            log_message("CIC set to %s automatically.\n", cic_str);
        }
        else
            log_message("Unknown CIC! Game might not boot properly!\n");

        // Free used memory
        free(bootcode);
    }
    else if (cart->cictype == 0)
    {
        int cic = -1;

        // Get the CIC key
        switch(params->cictype)
        {
            case 0:
            case 6101: cic = 0; break;
            case 1:
            case 6102: cic = 1; break;
            case 2:
            case 7101: cic = 2; break;
            case 3:
            case 7102: cic = 3; break;
            case 4:
            case 103:
            case 6103:
            case 7103: cic = 4; break;
            case 5:
            case 105:
            case 6105:
            case 7105: cic = 5; break;
            case 6:
            case 106:
            case 6106:
            case 7106: cic = 6; break;
            case 7:
            case 5101: cic = 7; break;
            case 303: fatal_error("This CIC is not supported through USB");
            default: fatal_error("Unknown CIC type '%d'.", params->cictype);
        }

        // Set the CIC
        cart->cictype = params->cictype;
        device_sendcmd_64drive(cart, DEV_CMD_SETCIC, false, 1, (1 << 31) | cic, 0);
        log_message("CIC set to %d.\n", params->cictype);
    }

    // Set Savetype
    if (params->savetype != 0)
    {
        device_sendcmd_64drive(cart, DEV_CMD_SETSAVE, false, 1, params->savetype, 0);
        log_message("Save type set to %d.\n", params->savetype);
    }

    // Decide a better, more optimized chunk size
    if (size > 16 * 1024 * 1024)
        chunk = 32;
    else if ( size > 2 * 1024 * 1024)
        chunk = 16;
    else
        chunk = 4;
    chunk *= 128 * 1024; // Convert to megabytes

    // Send chunks to the cart
    sendrom_progress(0);
    for ( ; ; )
    {
        int i;

        // Decide how many bytes to send
        if (bytes_left >= chunk)
            bytes_do = chunk;
        else
            bytes_do = bytes_left;

        // If we have an uneven number of bytes, fix that
        if (bytes_do%4 != 0)
            bytes_do -= bytes_do%4;

        // End if we've got nothing else to send
        if (bytes_do <= 0)
            break;

        // Try to send chunks
        for (i=0; i<2; i++)
        {
            int j;

            // If we failed the first time, clear the USB and try again
            if (i == 1)
            {
                FT_ResetPort(cart->handle);
                FT_ResetDevice(cart->handle);
                FT_Purge(cart->handle, FT_PURGE_RX | FT_PURGE_TX);
            }

            // Send the chunk to RAM
            device_sendcmd_64drive(cart, DEV_CMD_LOADRAM, false, 2, ram_addr, (bytes_do & 0xffffff) | 0 << 24);
            fread(rom_buffer, bytes_do, 1, file);
                  if (params->z64)
                      for (j=0; j<bytes_do; j+=2)
                          SWAP(rom_buffer[j], rom_buffer[j+1]);
            FT_Write(cart->handle, rom_buffer, bytes_do, &cart->bytes_written);

            // If we managed to write, don't try again
            if (cart->bytes_written)
                break;
        }

        // Check for a timeout
        if (cart->bytes_written == 0)
        {
            free(rom_buffer);
            fatal_error("64Drive timed out.");
        }

        // Ignore the success response
        cart->status = FT_Read(cart->handle, rom_buffer, 4, &cart->bytes_read);

        // Keep track of how many bytes were uploaded
        bytes_left -= bytes_do;
        bytes_done += bytes_do;
        ram_addr += bytes_do;

        // Draw the progress bar
        sendrom_progress((float)bytes_done/size);
    }

    // Wait for the CMP signal
    #ifndef LINUX
        Sleep(50);
    #else
        usleep(50);
    #endif

    // Read the incoming CMP signals to ensure everything's fine
    FT_GetQueueStatus(cart->handle, &cmps);
    while (cmps > 0)
    {
        // Read the CMP signal and ensure it's correct
        FT_Read(cart->handle, rom_buffer, 4, &cart->bytes_read);
        if (rom_buffer[0] != 'C' || rom_buffer[1] != 'M' || rom_buffer[2] != 'P' || rom_buffer[3] != 0x20)
            fatal_error("Received wrong CMPlete signal: %c %c %c %02x.", rom_buffer[0], rom_buffer[1], rom_buffer[2], rom_buffer[3]);

        // Wait a little bit before reading the next CMP signal
        #ifndef LINUX
            Sleep(50);
        #else
            usleep(50);
        #endif
        FT_GetQueueStatus(cart->handle, &cmps);
    }

    free(rom_buffer);
}


/*==============================
    device_senddata_64drive
    Sends data to the flashcart
    @param A pointer to the cart context
    @param A pointer to the data to send
    @param The size of the data
==============================*/

void device_senddata_64drive(ftdi_context_t* cart, int datatype, char* data, u32 size)
{
    u8 buf[4];
    u32 cmp_magic;
    int newsize = 0;
    char* datacopy = NULL;

    // Pad the data to be 512 byte aligned if it is large, if not then to 4 bytes
    if (size > 512 && (size%512) != 0)
        newsize = (size-(size%512))+512;
    else if (size % 4 != 0)
        newsize = (size & ~3) + 4;
    else
        newsize = size;

    // Copy the data onto a temp variable
    datacopy = (char*) calloc(newsize, 1);
    memcpy(datacopy, data, size);
    senddata_progress(0.0);

    // Send this block of data
    device_sendcmd_64drive(cart, DEV_CMD_USBRECV, false, 1, (newsize & 0x00FFFFFF) | datatype << 24, 0);
    cart->status = FT_Write(cart->handle, datacopy, newsize, &cart->bytes_written);

    // Read the CMP signal
    cart->status = FT_Read(cart->handle, buf, 4, &cart->bytes_read);
    cmp_magic = swap_endian(buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0]);
    if (cmp_magic != 0x434D5040)
        fatal_error("Received wrong CMPlete signal.");

    // Draw the progress bar
    senddata_progress(1.0);

    // Free used up resources
    free(datacopy);
}


/*==============================
    device_close_64drive
    Closes the USB pipe
    @param A pointer to the cart context
==============================*/

void device_close_64drive(ftdi_context_t* cart)
{
    testcommand(FT_Close(cart->handle), "Unable to close flashcart.");
    cart->handle = 0;
}
