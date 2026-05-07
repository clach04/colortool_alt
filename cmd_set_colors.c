/*
 * cmd_set_colors.c - Sets CMD console colors using Windows API
 *
 * Compiles with:
 *
 *      gcc ini.c cmd_set_colors.c -o cmd_set_colors
 *      cl ini.c cmd_set_colors.c
 *
 * Run in CMD to apply colors
 *
 * Basically a poor mans verison of Microsoft ColorTool
 * Using; GetStdHandle(), GetConsoleScreenBufferInfoEx(), and SetConsoleScreenBufferInfoEx() from kernel32.dl1l
 * Current has:
 *    * hard coded colors
 *    * --**not working** ANSI color display TODO use SetConsoleMode() with ENABLE_VIRTUAL_TERMINAL_PROCESSING https://learn.microsoft.com/en-us/windows/console/setconsolemode--
 *    * **partially working** ANSI color display - offset math wrong and looks ugly
 */

/* Version macro */
#define VERSION "0.0.2-dev"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "ini.h"

/* RGB helper macro - converts to uint32 in BGR order for Windows */
/* https://learn.microsoft.com/en-us/windows/win32/gdi/colorref */
#define myRGB(r, g, b) ((uint32_t)((uint8_t)(r) | ((uint8_t)(g) << 8) | ((uint8_t)(b) << 16)))

/* Console buffer info structure - matches Windows SDK definition */
struct CONSOLE_SCREEN_BUFFER_INFO_EX {
    uint32_t cbSize;
    short dwSize_X;
    short dwSize_Y;
    short dwCursorPosition_X;
    short dwCursorPosition_Y;
    uint16_t wAttributes;
    short srWindow_Left;
    short srWindow_Top;
    short srWindow_Right;
    short srWindow_Bottom;
    short dwMaximumWindowSize_X;
    short dwMaximumWindowSize_Y;
    uint16_t wPopupAttributes;
    int bFullscreenSupported;
    uint32_t ColorTable[16];
};

/* Windows API function declarations */
typedef HANDLE(WINAPI *GetStdHandle_t)(int nStdHandle);
typedef BOOL(WINAPI *GetConsoleScreenBufferInfoEx_t)(HANDLE hConsoleOutput, struct CONSOLE_SCREEN_BUFFER_INFO_EX *ConsoleScreenBufferInfo);
typedef BOOL(WINAPI *SetConsoleScreenBufferInfoEx_t)(HANDLE ConsoleOutput, struct CONSOLE_SCREEN_BUFFER_INFO_EX *ConsoleScreenBufferInfoEx);

#ifndef STD_OUTPUT_HANDLE  // hopefully from winbase.h
    /* Standard output handle constant */
    #define STD_OUTPUT_HANDLE (-11)
#endif

/* The 16-color palette (DOS-style) */
// Default to Dracula Classic (Dark) Color Theme/Scheme (as that's what I'm using at the moment)
static uint32_t ColorTable[16] = {
    myRGB( 40,  42,  54),   /* 0: Black */
    myRGB(189, 147, 249),   /* 1: Dark Blue */
    myRGB( 80, 250, 123),   /* 2: Dark Green */
    myRGB(139, 233, 253),   /* 3: Dark Cyan */
    myRGB(255,  85,  85),   /* 4: Dark Red */
    myRGB(255, 121, 198),   /* 5: Dark Magenta */
    myRGB(241, 250, 140),   /* 6: Dark Yellow */
    myRGB(248, 248, 242),   /* 7: Dark White (Gray) */
    myRGB( 98, 114, 164),   /* 8: Bright Black (Gray) */
    myRGB(214, 172, 255),   /* 9: Bright Blue */
    myRGB(105, 255, 148),   /* 10: Bright Green */
    myRGB(164, 255, 255),   /* 11: Bright Cyan */
    myRGB(255, 110, 110),   /* 12: Bright Red */
    myRGB(255, 146, 223),   /* 13: Bright Magenta */
    myRGB(255, 255, 165),   /* 14: Bright Yellow */
    myRGB(255, 255, 255),   /* 15: Bright White */
    // TODO handle FOREGROUND and BACKGROUND
};


/* Color indices */
#define IDX_DARK_BLACK    0
#define IDX_DARK_BLUE     1
#define IDX_DARK_GREEN    2
#define IDX_DARK_CYAN     3
#define IDX_DARK_RED      4
#define IDX_DARK_MAGENTA  5
#define IDX_DARK_YELLOW   6
#define IDX_DARK_WHITE    7
#define IDX_BRIGHT_BLACK  8
#define IDX_BRIGHT_BLUE   9
#define IDX_BRIGHT_GREEN  10
#define IDX_BRIGHT_CYAN   11
#define IDX_BRIGHT_RED    12
#define IDX_BRIGHT_MAGENTA 13
#define IDX_BRIGHT_YELLOW 14
#define IDX_BRIGHT_WHITE  15
// TODO handle FOREGROUND and BACKGROUND

/* Load colors from an INI file into the ColorTable */
// Uses ini.c/ini.h functions for parsing
// TODO handle FOREGROUND and BACKGROUND
static int load_ini_colors(const char *filename, uint32_t *table) {
    ini_t *ini = ini_load(filename);
    if (!ini) {
        fprintf(stderr, "Warning: Could not open INI file: %s\n", filename);
        return -1;
    }

    int found_colors[16] = {0};
    int colors_found = 0;

    // Color name mappings (case-insensitive lookup via ini_get), in same order as IDX_DARK_BLACK (etc.) defines
    const char *color_names[] = {
        "DARK_BLACK", "DARK_BLUE", "DARK_GREEN", "DARK_CYAN",
        "DARK_RED", "DARK_MAGENTA", "DARK_YELLOW", "DARK_WHITE",
        "BRIGHT_BLACK", "BRIGHT_BLUE", "BRIGHT_GREEN", "BRIGHT_CYAN",
        "BRIGHT_RED", "BRIGHT_MAGENTA", "BRIGHT_YELLOW", "BRIGHT_WHITE"
    };

    // Process each color entry in the [table] section
    for (int i = 0; i < 16; i++) {
        const char *val = ini_get(ini, "table", color_names[i]);
        if (val) {
            int r, g, b;
            // Parse the RGB value (format: "R,G,B" or "R, G, B")
            if (sscanf(val, "%d, %d, %d", &r, &g, &b) == 3) {
                table[i] = myRGB(r, g, b);
                found_colors[i] = 1;
                colors_found++;
            } else if (sscanf(val, "%d,%d,%d", &r, &g, &b) == 3) {
                table[i] = myRGB(r, g, b);
                found_colors[i] = 1;
                colors_found++;
            }
        }
    }

    ini_free(ini);

    printf("colors_found: %d\n", colors_found);

    /* If we found some colors, return success */
    return (colors_found > 0) ? 0 : -1;
}

/* Function to load colors from an INI file */
int load_colors_from_ini(const char *ini_filename) {
    return load_ini_colors(ini_filename, ColorTable);
}

/* Function to print the color table (for debugging) */
void print_color_table(void) {
    const char *names[] = {
        "DARK_BLACK", "DARK_BLUE", "DARK_GREEN", "DARK_CYAN",
        "DARK_RED", "DARK_MAGENTA", "DARK_YELLOW", "DARK_WHITE",
        "BRIGHT_BLACK", "BRIGHT_BLUE", "BRIGHT_GREEN", "BRIGHT_CYAN",
        "BRIGHT_RED", "BRIGHT_MAGENTA", "BRIGHT_YELLOW", "BRIGHT_WHITE"
    };

    printf("ColorTable (%d colors):\n", 16);
    for (int i = 0; i < 16; i++) {
        uint32_t c = ColorTable[i];
        uint8_t r = (uint8_t)(c);
        uint8_t g = (uint8_t)(c >> 8);
        uint8_t b = (uint8_t)(c >> 16);
        printf("  [%2d] %s: RGB(%3d, %3d, %3d) = 0x%06X\n",
               i, names[i], r, g, b, c);
    }
}

int main(int argc, char *argv[])
{
    HMODULE kernel32;
    GetStdHandle_t pGetStdHandle;
    GetConsoleScreenBufferInfoEx_t pGetConsoleScreenBufferInfoEx;
    SetConsoleScreenBufferInfoEx_t pSetConsoleScreenBufferInfoEx;
    HANDLE hOut;
    struct CONSOLE_SCREEN_BUFFER_INFO_EX csbiex;
    BOOL success;
    int i;
    int process_exit_code=0;
    char *ini_filename="colors.ini";

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [ini_filename]\n", argv[0]);
            printf("Default ini_filename is \"colors.ini\"\n");
            return 0;
        }
        if (strcmp(argv[1], "--version") == 0) {
            printf("%s\n", VERSION);
            return 0;
        }
        ini_filename = argv[1];
    }

    /* Load kernel32.dll and get function pointers */
    kernel32 = LoadLibraryA("kernel32.dll");
    if (!kernel32) {
        fprintf(stderr, "Error: Could not load kernel32.dll\n");
        return 1;
    }

    pGetStdHandle = (GetStdHandle_t)GetProcAddress(kernel32, "GetStdHandle");
    pGetConsoleScreenBufferInfoEx = (GetConsoleScreenBufferInfoEx_t)GetProcAddress(kernel32, "GetConsoleScreenBufferInfoEx");
    pSetConsoleScreenBufferInfoEx = (SetConsoleScreenBufferInfoEx_t)GetProcAddress(kernel32, "SetConsoleScreenBufferInfoEx");

    if (!pGetStdHandle || !pGetConsoleScreenBufferInfoEx || !pSetConsoleScreenBufferInfoEx) {
        fprintf(stderr, "Error: Could not get API functions from kernel32.dll\n");
        FreeLibrary(kernel32);
        return 1;
    }

    /* Get console output handle */
    hOut = pGetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not get console handle\n");
        FreeLibrary(kernel32);
        return 1;
    }

    /* Get current console screen buffer info */
    csbiex.cbSize = sizeof(struct CONSOLE_SCREEN_BUFFER_INFO_EX);
    success = pGetConsoleScreenBufferInfoEx(hOut, &csbiex);  // https://learn.microsoft.com/en-us/windows/console/getconsolescreenbufferinfoex
    if (!success) {
        fprintf(stderr, "Error: Could not obtain console screen buffer\n");
        FreeLibrary(kernel32);
        return 1;
    }

    {
        printf("Loading \"%s\"\n", ini_filename);
        process_exit_code = load_colors_from_ini(ini_filename);
        if (process_exit_code)
        {
            printf("Error loading \"%s\"\n", ini_filename);
            printf("Result %d\n", process_exit_code);

            //goto exit;
            // use built in default colors
            printf("Using default built in colors");
        }
        process_exit_code = 0;
        print_color_table();
    }

    // TODO Text/Forground, Background, and Cursor color using SetConsoleTextAttribute()
    // https://learn.microsoft.com/en-us/windows/console/console-screen-buffers
    // https://learn.microsoft.com/en-us/windows/console/setconsolecursorinfo

    /* Copy the static color table into the buffer */
    for (i = 0; i < 16; i++) {
        csbiex.ColorTable[i] = ColorTable[i];
    }

    /* Adjust window size (matches original ColorTool behavior) */
    csbiex.srWindow_Right++;

    /* Apply the new color table */
    success = pSetConsoleScreenBufferInfoEx(hOut, &csbiex);  // https://learn.microsoft.com/en-us/windows/console/setconsolescreenbufferinfoex
    if (!success) {
        fprintf(stderr, "Error: Could not set console screen buffer info\n");
        FreeLibrary(kernel32);
        return 1;
    }

exit:

    FreeLibrary(kernel32);
    return process_exit_code;
}
