#include "Input.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>

#include "Globals.h"

char key_states[322]; // 322 is the number of SDLK_DOWN events
char frame_key_states[322];
char mouse_button_states[5];
char frame_mouse_button_states[5];
int mouse_position[2];
int mouse_motion[2];
char mouse_wheel_direction = 0;
MouseMode mouse_mode = MOUSEMODE_CAPTURED;

char in_game = 1;

void Input_HandleKeyboard(int key, char down)
{
    if(key >= 322) return;
    if(down) frame_key_states[key] = 1;
    key_states[key] = down;
}

void Input_HandleMouseMotion(int x, int y, int rx, int ry)
{
    mouse_position[0] = x;
    mouse_position[1] = y;
    mouse_motion[0] += rx;
    mouse_motion[1] += ry;
}

void Input_HandleMouseButton(int button, char down)
{
    if(down) frame_mouse_button_states[button - 1] = 1;
    mouse_button_states[button - 1] = down;
}

void Input_HandleMouseWheel(int direction)
{
    mouse_wheel_direction = direction;
}

int Input_GetMouseWheelDirection()
{
    return mouse_wheel_direction;
}

char Input_IsKeyPressedd(int key)
{
    return key_states[key];
}

char Input_IsKeyJustPressed(int key)
{
    if(key < 0 || key >= sizeof(frame_key_states) / sizeof(frame_key_states[0]))
    {
        printf("Failed to get key %d.\n", key);
        return 0;
    }
    return frame_key_states[key];
}

int Input_IsMouseButtonPressed(int button)
{
    if(!in_game) return 0;
    if(button < 0 && button > 5) return 0;
    return mouse_button_states[button - 1];
}

int Input_IsMouseButtonJustPressedd(int button)
{
    if(!in_game) return 0;
    if(button < 0 && button > 5) return 0;
    return frame_mouse_button_states[button - 1];
}

void Input_GetMouseMotion(int *x, int *y)
{
    *x = mouse_motion[0];
    *y = mouse_motion[1];
}

char mouse_mode_updated = 0;
void Input_SetMouseMode(MouseMode mode)
{
    mouse_mode_updated = 1;
    mouse_mode = mode;
}

void Input_RenderEnd()
{
    memset(frame_key_states, 0, sizeof(frame_key_states));
    memset(mouse_motion, 0, sizeof(mouse_motion));
    memset(frame_mouse_button_states, 0, sizeof(frame_mouse_button_states));
    mouse_wheel_direction = 0;
    

    char focus = (SDL_GetWindowFlags(g_window) & SDL_WINDOW_INPUT_FOCUS) != 0;

    if(mouse_mode_updated || focus != in_game)
    {
        in_game = focus;
        mouse_mode_updated = 0;
        if(mouse_mode == MOUSEMODE_CURSOR || !in_game)
        {
            SDL_SetWindowRelativeMouseMode(g_window, false);
            if(in_game)
                SDL_WarpMouseInWindow(g_window, g_width / 2, g_height / 2);
        }
        else if (mouse_mode == MOUSEMODE_CAPTURED)
        {
            SDL_SetWindowRelativeMouseMode(g_window, true);
        }
    }
}

void Input_Setup()
{
    memset(key_states, 0, sizeof(key_states));
    memset(frame_key_states, 0, sizeof(frame_key_states));
    memset(mouse_button_states, 0, sizeof(mouse_button_states));
    memset(frame_mouse_button_states, 0, sizeof(frame_mouse_button_states));
    memset(mouse_motion, 0, sizeof(mouse_motion));
    memset(mouse_position, 0, sizeof(mouse_position));
    Input_SetMouseMode(MOUSEMODE_CAPTURED);
}