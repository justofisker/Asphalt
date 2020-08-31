#include "Input.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>

#include "Globals.h"

char key_states[256];
char frame_key_states[256];
char special_states[256];
char frame_special_states[256];
char mouse_button_states[2];
char frame_mouse_button_states[2];
int mouse_position[2];
int mouse_motion[2];
MouseMode mouse_mode = MOUSEMODE_CURSOR;

void keyboard_func(unsigned char key, int x, int y)
{
    key_states[key] = 1;
    frame_key_states[key] = 1;
}
void keyboard_func_up(unsigned char key, int x, int y)
{
    key_states[key] = 0;
}
void special_func(int key, int x, int y)
{
    special_states[key] = 1;
    frame_special_states[key] = 1;
}
void special_func_up(int key, int x, int y)
{
    special_states[key] = 0;
}

void mouse_func(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        if(button == GLUT_LEFT_BUTTON)
        {
                  mouse_button_states[0] = 1;
            frame_mouse_button_states[0] = 1;
        }
        if(button == GLUT_RIGHT_BUTTON)
        {
                  mouse_button_states[1] = 1;
            frame_mouse_button_states[1] = 1;
        }
    }
    if(state == GLUT_UP)
    {
        if(button == GLUT_LEFT_BUTTON)
            mouse_button_states[0] = 0;
        if(button == GLUT_RIGHT_BUTTON)
            mouse_button_states[1] = 0;
    }
}

void mouse_motion_func(int x, int y)
{
    mouse_motion[0] += x - mouse_position[0];
    mouse_motion[1] += y - mouse_position[1];
    mouse_position[0] = x;
    mouse_position[1] = y;
}

void set_mouse_mode(MouseMode mode)
{
    mouse_mode = mode;
}

char is_key_pressed(unsigned char key)
{
    return key_states[key];
}

char is_key_just_pressed(unsigned char key)
{
    return frame_key_states[key];
}

char is_special_pressed(int key)
{
    return special_states[key];
}

char is_special_just_pressed(int key)
{
    return frame_special_states[key];
}

int is_mouse_button_pressed(int button)
{
    return button == GLUT_LEFT_BUTTON ? mouse_button_states[0] : button == GLUT_RIGHT_BUTTON ? mouse_button_states[1] : 0;
}

int is_mouse_button_just_pressed(int button)
{
    return button == GLUT_LEFT_BUTTON ? frame_mouse_button_states[0] : button == GLUT_RIGHT_BUTTON ? frame_mouse_button_states[1] : 0;
}

void get_mouse_motion(int *x, int *y)
{
    *x = mouse_motion[0];
    *y = mouse_motion[1];
}

void input_render_start()
{
    if(mouse_mode == MOUSEMODE_CURSOR)
    {
        glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
    }
    else if (mouse_mode == MOUSEMODE_CAPTURED)
    {
        glutSetCursor(GLUT_CURSOR_NONE);
        mouse_position[0] = global_width / 2;
        mouse_position[1] = global_height / 2;
        glutWarpPointer(global_width / 2, global_height / 2);
    }
}

void input_render_end()
{
    memset(frame_key_states, 0, sizeof(frame_key_states));
    memset(frame_special_states, 0, sizeof(frame_special_states));
    memset(mouse_motion, 0, sizeof(mouse_motion));
    memset(frame_mouse_button_states, 0, sizeof(frame_mouse_button_states));
}

void setup_input()
{
    memset(key_states, 0, sizeof(key_states));
    memset(special_states, 0, sizeof(special_states));
    memset(frame_key_states, 0, sizeof(frame_key_states));
    memset(frame_special_states, 0, sizeof(frame_special_states));
    memset(mouse_button_states, 0, sizeof(mouse_button_states));
    memset(frame_mouse_button_states, 0, sizeof(frame_mouse_button_states));
    memset(mouse_motion, 0, sizeof(mouse_motion));
    memset(mouse_position, 0, sizeof(mouse_position));
}