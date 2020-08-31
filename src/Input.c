#include "Input.h"
#include <stdlib.h>
#include "Globals.h"

#ifdef _WIN32
#include <Windows.h>

HWND asphalt_window = 0;
 
char is_in_game()
{
    if(!asphalt_window)
        asphalt_window = FindWindow(NULL, WINDOW_TITLE);
    return GetForegroundWindow() == asphalt_window;
}

char is_key_down(unsigned char key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

#endif // _WIN32

#ifdef __linux__

char is_in_game()
{
    return 1;
}

char is_key_down(unsigned char key)
{
    return 0;
}

#endif // __linux__