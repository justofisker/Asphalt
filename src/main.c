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

#include "Util.h"
#include "Globals.h"

#define WINDOW_TITLE "Minecraft Clone"

static int width = 1920, height = 1080;

clock_t last_frame;

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

unsigned int basic_shader;

unsigned int vao, vb, ib;

static void setup()
{
    basic_shader = compile_shader("res/shader/basic_vertex.glsl", "res/shader/basic_fragment.glsl");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);

    float verticies[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);

    unsigned char indicies[] = {
        0, 1, 2,
        2, 3, 0,
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glm_vec3_zero(global_camera_position);
    glm_vec3_zero(global_camera_rotation);
    glm_mat4_identity(global_view);
    glm_mat4_identity(global_projection);

    last_frame = clock();
    memset(key_states, 0, sizeof(key_states));
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);
    //
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);
    //
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
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

    clock_t time = clock();
    float delta = ((float) (time - last_frame)) / CLOCKS_PER_SEC;
    last_frame = time;

    vec3 direction = {0, 0, 0};
    if(get_key_state('a'))
        direction[0] += 1.0f;
    if(get_key_state('d'))
        direction[0] -= 1.0f;
    if(get_key_state('w'))
        direction[2] += 1.0f;
    if(get_key_state('s'))
        direction[2] -= 1.0f;
    glm_normalize(direction);
    glm_vec3_mul(direction, (vec3){delta, delta, delta}, direction);
    
    glm_vec3_add(global_camera_position, direction, global_camera_position);

    //printf("x: %.1f  \ty: %.1f  \tz: %.1f\n", global_camera_position[0], global_camera_position[1], global_camera_position[2]);

    glm_perspective(70.0f, (float)width / height, 0.01f, 1000.0f, global_projection);
    glm_mat4_identity(global_view);
    glm_translate(global_view, global_camera_position);

    mat4 model = GLM_MAT4_IDENTITY_INIT;

    glUseProgram(basic_shader);
    int model_loc = glGetUniformLocation(basic_shader, "u_Model");
    int view_loc = glGetUniformLocation(basic_shader, "u_View");
    int projection_loc = glGetUniformLocation(basic_shader, "u_Projection");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, global_view[0]);
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, global_projection[0]);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);
    glBindVertexArray(0);
        

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
