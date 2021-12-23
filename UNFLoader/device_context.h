#ifndef __DEVICE_CONTEXT_HEADER
#define __DEVICE_CONTEXT_HEADER

#include "types.h"

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
    const char*        current_rompath;
} ftdi_context_t;

void testcommand(FT_STATUS status, const char* reason, ...);
void log_message(const char* message, ...);
void fatal_error(const char* message, ...);
void sendrom_progress(float progress);
void senddata_progress(float progress);

u32   swap_endian(u32 val);
u32   calc_padsize(u32 size);
#define SWAP(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) // From https://graphics.stanford.edu/~seander/bithacks.html#SwappingValuesXOR
u32 romhash(u8 *buff, u32 len);
s16 cic_from_hash(u32 hash);

#endif