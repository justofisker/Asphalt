#ifndef INPUT_H
#define INPUT_H

typedef enum _MouseMode {
    MOUSEMODE_CURSOR = 0,
    MOUSEMODE_CAPTURED
} MouseMode;

void Input_HandleKeyboard(int key, char down);
void Input_HandleMouseMotion(int x, int y, int rx, int ry);
void Input_HandleMouseButton(int button, char down);
void Input_HandleMouseWheel(int direction);

char Input_IsKeyPressedd(int key);
char Input_IsKeyJustPressed(int key);
int Input_IsMouseButtonPressed(int button);
int Input_IsMouseButtonJustPressedd(int button);
void Input_GetMouseMotion(int *x, int *y);
void Input_SetMouseMode(MouseMode mode);
int Input_GetMouseWheelDirection();

void Input_Setup();
void Input_RenderEnd();

#endif // INPUT_H