#ifndef INPUT_H
#define INPUT_H

typedef enum _MouseMode {
    MOUSEMODE_CURSOR = 0,
    MOUSEMODE_CAPTURED
} MouseMode;

void keyboard_func(int key, char down);
void mouse_motion_func(int x, int y, int rx, int ry);
void mouse_button_func(int button, char down);
void mouse_wheel_func(int direction);

char is_key_pressed(int key);
char is_key_just_pressed(int key);
char is_special_pressed(int key);
char is_special_just_pressed(int key);
int is_mouse_button_pressed(int button);
int is_mouse_button_just_pressed(int button);
void get_mouse_motion(int *x, int *y);
void set_mouse_mode(MouseMode mode);
int get_mouse_wheel_direction();

void setup_input();
void input_render_start();
void input_render_end();

#endif // INPUT_H