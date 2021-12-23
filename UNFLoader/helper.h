#ifndef __HELPER_HEADER
#define __HELPER_HEADER

    #include "helper_internal.h"


    /*********************************
                  Macros
    *********************************/

    #define PRINT_HISTORY_SIZE 512

    // Color macros
    #define TOTAL_COLORS 4
    #define CR_NONE   0
    #define CR_RED    1
    #define CR_GREEN  2
    #define CR_BLUE   3
    #define CR_YELLOW 4

    // Program colors
    #define CRDEF_PROGRAM CR_NONE
    #define CRDEF_ERROR   CR_RED
    #define CRDEF_INPUT   CR_GREEN
    #define CRDEF_PRINT   CR_YELLOW
    #define CRDEF_INFO    CR_BLUE


    /*********************************
            Function Prototypes
    *********************************/

    // Printing
    #define pdprint(string, color, ...) __pdprint(color, string, ##__VA_ARGS__)
    #define pdprintw(window, string, color, ...) __pdprintw(window, color, 1, string, ##__VA_ARGS__)
    #define pdprintw_nolog(window, string, color, ...) __pdprintw(window, color, 0, string, ##__VA_ARGS__)
    #define pdprint_replace(string, color, ...) __pdprint_replace(color, string, ##__VA_ARGS__)
    void terminate(const char* reason, ...);
    void on_device_error(const char* error);
    void on_device_message(const char* message);
    void on_sendrom_progress(float percent);
    void on_senddata_progress(float percent);
    void progressbar_draw(const char* text, short color, float percent);

    // Useful functions
    char* gen_filename();
    void handle_timeout();

#endif