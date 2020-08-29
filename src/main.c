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

#include "Mesh.h"
#include "Chunk.h"
#include "Texture.h"

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

static Chunk *chunk[4];

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

static void setup()
{
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    global_basic_shader = compile_shader("res/shader/basic_vertex.glsl", "res/shader/basic_fragment.glsl");
    global_texture = create_texture("res/texture/grass.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);

    global_basic_model_loc = glGetUniformLocation(global_basic_shader, "u_Model");
    global_basic_view_loc = glGetUniformLocation(global_basic_shader, "u_View");
    global_basic_projection_loc = glGetUniformLocation(global_basic_shader, "u_Projection");
    global_basic_texture_loc = glGetUniformLocation(global_basic_shader, "u_Texture");

    glm_vec3_zero(global_camera_position);
    glm_vec3_zero(global_camera_rotation);
    glm_mat4_identity(global_view);
    glm_mat4_identity(global_projection);

    last_frame = clock();
    memset(key_states, 0, sizeof(key_states));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    //
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);
    //
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    int x, y;
    for(x = 0; x < 2; x++)
        for(y = 0; y < 2; y++)
            chunk[x + y * 2] = create_chunk(-x, -y);
}

static void mouse_motion(int x, int y)
{
    global_camera_rotation[0] += glm_rad(((float)y - height / 2) / 10);
    global_camera_rotation[0] = fmaxf(fminf(glm_rad(85.f), global_camera_rotation[0]), glm_rad(-85.f));
    global_camera_rotation[1] += glm_rad(((float)x - width / 2) / 10);
    global_camera_rotation[1] = fmodf(global_camera_rotation[1], GLM_PIf * 2);
}

static void reset_mouse()
{
    glutWarpPointer(width / 2, height / 2);
    glutSetCursor(GLUT_CURSOR_NONE);
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

    reset_mouse();

    vec3 direction = {0, 0, 0};
    if(get_key_state('a'))
        direction[0] += 1.0f;
    if(get_key_state('d'))
        direction[0] -= 1.0f;
    if(get_key_state('w'))
        direction[2] += 1.0f;
    if(get_key_state('s'))
        direction[2] -= 1.0f;
    if(get_key_state(' '))
        direction[1] += 1.0f;
    if(GetKeyState(VK_SHIFT) & 0x8000)
        direction[1] -= 1.0f;
    glm_normalize(direction);
    glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
    glm_vec3_mul(direction, (vec3){delta * 6, -delta * 6, delta * 6}, direction);
    glm_vec3_add(global_camera_position, direction, global_camera_position);

    //printf("x: %.1f  \ty: %.1f  \tz: %.1f\n", global_camera_position[0], global_camera_position[1], global_camera_position[2]);
    //printf("x: %.1f  \ty: %.1f  \tz: %.1f\n", global_camera_rotation[0] / GLM_PIf * 180.0f, global_camera_rotation[1] / GLM_PIf * 180.0f, global_camera_rotation[2] / GLM_PIf * 180.0f);

    glm_perspective(glm_rad(70.0f), (float)width / height, 0.01f, 100.0f, global_projection);
    glm_mat4_identity(global_view);
    mat4 rotation;
    glm_euler_xyz(global_camera_rotation, rotation);
    glm_mul_rot(global_view, rotation, global_view);
    glm_translate(global_view, global_camera_position);

    int i;
    for(i = 0; i < 4; i++)
        render_chunk(chunk[i]);

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
