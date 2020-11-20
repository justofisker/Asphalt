#ifndef INPUT_H
#define INPUT_H

#define INPUT_KEY_ESCAPE 27
#define INPUT_KEY_BACKSPACE 8
#define INPUT_KEY_DELETE 127

typedef enum _MouseMode {
    MOUSEMODE_CURSOR = 0,
    MOUSEMODE_CAPTURED
} MouseMode;

void keyboard_func(unsigned char key, int x, int y);
void keyboard_func_up(unsigned char key, int x, int y);
void special_func(int key, int x, int y);
void special_func_up(int key, int x, int y);
void mouse_func(int state, int button, int x, int y);
void mouse_motion_func(int x, int y);

char is_key_pressed(unsigned char key);
char is_key_just_pressed(unsigned char key);
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