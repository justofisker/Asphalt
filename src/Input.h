#ifndef INPUT_H
#define INPUT_H

#ifdef _WIN32

#define KEY_ESCAPE VK_ESCAPE
#define KEY_SHIFT  VK_SHIFT

#endif // _WIN32
#ifdef __linux__

#define KEY_ESCAPE 0
#define KEY_SHIFT  0

#endif // __linux__

char is_in_game();
char is_key_down(int key);
char is_key_just_pressed(int key);

#endif // INPUT_H