/*
 * cmd_set_colors.c - Sets CMD console colors using Windows API
 *
 * Compiles with:
 *
 *      gcc.exe cmd_set_colors.c -o cmd_set_colors
 *      cl cmd_set_colors.c
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

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

/* RGB helper macro - converts to uint32 in BGR order for Windows */
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
static const uint32_t ColorTable[16] = {
    myRGB(0, 0, 0),         /* 0: Black */
    myRGB(0, 0, 128),       /* 1: Dark Blue */
    myRGB(0, 128, 0),       /* 2: Dark Green */
    myRGB(0, 128, 128),     /* 3: Dark Cyan */
    myRGB(128, 0, 0),       /* 4: Dark Red */
    myRGB(128, 0, 128),     /* 5: Dark Magenta */
    myRGB(128, 128, 0),     /* 6: Dark Yellow */
    myRGB(192, 192, 192),   /* 7: Dark White (Gray) */
    myRGB(128, 128, 128),   /* 8: Bright Black (Gray) */
    myRGB(0, 0, 255),       /* 9: Bright Blue */
    myRGB(0, 255, 0),       /* 10: Bright Green */
    myRGB(0, 255, 255),     /* 11: Bright Cyan */
    myRGB(255, 0, 0),       /* 12: Bright Red */
    myRGB(255, 0, 255),     /* 13: Bright Magenta */
    myRGB(255, 255, 0),     /* 14: Bright Yellow */
    myRGB(255, 255, 255)    /* 15: Bright White */
};


int main(void)
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
    success = pGetConsoleScreenBufferInfoEx(hOut, &csbiex);
    if (!success) {
        fprintf(stderr, "Error: Could not obtain console screen buffer\n");
        FreeLibrary(kernel32);
        return 1;
    }

    /* Copy the static color table into the buffer */
    for (i = 0; i < 16; i++) {
        csbiex.ColorTable[i] = ColorTable[i];
    }

    /* Adjust window size (matches original ColorTool behavior) */
    csbiex.srWindow_Right++;

    /* Apply the new color table */
    success = pSetConsoleScreenBufferInfoEx(hOut, &csbiex);
    if (!success) {
        fprintf(stderr, "Error: Could not set console screen buffer info\n");
        FreeLibrary(kernel32);
        return 1;
    }

    FreeLibrary(kernel32);
    return process_exit_code;
}
