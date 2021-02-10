#include "Input.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

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

void keyboard_func(int key, char down)
{
    if(key >= 322) return;
    if(down) frame_key_states[key] = 1;
    key_states[key] = down;
}

void mouse_motion_func(int x, int y, int rx, int ry)
{
    mouse_position[0] = x;
    mouse_position[1] = y;
    mouse_motion[0] += rx;
    mouse_motion[1] += ry;
}

void mouse_button_func(int button, char down)
{
    if(down) frame_mouse_button_states[button - 1] = 1;
    mouse_button_states[button - 1] = down;
}

void mouse_wheel_func(int direction)
{
    mouse_wheel_direction = direction;
}

int get_mouse_wheel_direction()
{
    return mouse_wheel_direction;
}

char is_key_pressed(int key)
{
    return key_states[key];
}

char is_key_just_pressed(int key)
{
    return frame_key_states[key];
}

int is_mouse_button_pressed(int button)
{
    if(!in_game) return 0;
    if(button < 0 && button > 5) return 0;
    return mouse_button_states[button - 1];
}

int is_mouse_button_just_pressed(int button)
{
    if(!in_game) return 0;
    if(button < 0 && button > 5) return 0;
    return frame_mouse_button_states[button - 1];
}

void get_mouse_motion(int *x, int *y)
{
    *x = mouse_motion[0];
    *y = mouse_motion[1];
}

char mouse_mode_updated = 0;
void set_mouse_mode(MouseMode mode)
{
    mouse_mode_updated = 1;
    mouse_mode = mode;
}

SDL_Cursor *cursor_arrow;

void input_render_start()
{
    char focus = (SDL_GetWindowFlags(g_window) & SDL_WINDOW_INPUT_FOCUS) != 0;

    if(mouse_mode_updated || focus != in_game)
    {
        in_game = focus;
        mouse_mode_updated = 0;
        if(mouse_mode == MOUSEMODE_CURSOR || !in_game)
        {
            SDL_SetRelativeMouseMode(0);
            if(in_game)
                SDL_WarpMouseInWindow(g_window, g_width / 2, g_height / 2);
        }
        else if (mouse_mode == MOUSEMODE_CAPTURED)
        {
            SDL_SetRelativeMouseMode(1);
        }
    }
}

void input_render_end()
{
    memset(frame_key_states, 0, sizeof(frame_key_states));
    memset(mouse_motion, 0, sizeof(mouse_motion));
    memset(frame_mouse_button_states, 0, sizeof(frame_mouse_button_states));
    mouse_wheel_direction = 0;
}

void setup_input()
{
    memset(key_states, 0, sizeof(key_states));
    memset(frame_key_states, 0, sizeof(frame_key_states));
    memset(mouse_button_states, 0, sizeof(mouse_button_states));
    memset(frame_mouse_button_states, 0, sizeof(frame_mouse_button_states));
    memset(mouse_motion, 0, sizeof(mouse_motion));
    memset(mouse_position, 0, sizeof(mouse_position));
}