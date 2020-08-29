#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cglm/cglm.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define WINDOW_TITLE "Minecraft Clone"

static int width = 1920, height = 1080;

static char key_states[256];
static void KeyboardEvent(unsigned char key, char is_pressed)
{
    key_states[key] = is_pressed;
}

static void KeyboardDown(unsigned char button, int x, int y)
{
    if(button >= 'a' && button <= 'z')
        button -= 'a' - 'A';
    KeyboardEvent(button, 1);
}

static void KeyboardUp(unsigned char button, int x, int y)
{
    if(button >= 'a' && button <= 'z')
        button -= 'a' - 'A';
    KeyboardEvent(button, 0);
}

static char get_key_state(unsigned char key)
{
    if(key >= 'a' && key <= 'z')
        key -= 'a' - 'A';
    return key_states[key];
}

static void setup()
{
    memset(key_states, 0, sizeof(key_states));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

static void mouse_motion(int x, int y)
{

}

static void Resize(int w, int h)
{
    width = w > 1 ? w : 1;
    height = h > 1 ? h : 1;
    glViewport(0, 0, width, height);
    
    glClearDepth(1.0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

static void Render(void)
{
    glClearColor(0.0f, 0.2f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    if(gladLoadGL()) {
        // you need an OpenGL context before loading glad
        printf("I did load GL with no context!\n");
        exit(-1);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutCreateWindow(WINDOW_TITLE);

    glutReshapeFunc(Resize);
    glutDisplayFunc(Render);
    glutKeyboardFunc(KeyboardDown);
    glutKeyboardUpFunc(KeyboardUp);
    glutPassiveMotionFunc(mouse_motion);
    glutMotionFunc(mouse_motion);

    if(!gladLoadGL()) {
        printf("Something went wrong!\n");
        exit(-1);
    }

    // gladLoadGLLoader(&glutGetProcAddress);
    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    if (GLVersion.major < 2) {
        printf("Your system doesn't support OpenGL >= 2!\n");
        return -1;
    }

    printf("OpenGL %s, GLSL %s\n%s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_RENDERER));

    setup();

    glutMainLoop();

    return 0;
}
