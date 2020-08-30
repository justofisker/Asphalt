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

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    if(type == GL_DEBUG_TYPE_ERROR)
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

static void setup()
{
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    global_basic_shader = compile_shader("res/shader/basic_vertex.glsl", "res/shader/basic_fragment.glsl");
    global_texture = create_texture("res/texture/grass.png", GL_LINEAR, GL_NEAREST, GL_CLAMP_TO_EDGE);

    global_basic_model_loc = glGetUniformLocation(global_basic_shader, "u_Model");
    global_basic_view_loc = glGetUniformLocation(global_basic_shader, "u_View");
    global_basic_projection_loc = glGetUniformLocation(global_basic_shader, "u_Projection");
    global_basic_texture_loc = glGetUniformLocation(global_basic_shader, "u_Texture");

    glm_vec3_zero(global_camera_position);
    global_camera_position[1] -= 40.0f;
    glm_vec3_zero(global_camera_rotation);
    glm_mat4_identity(global_view);
    glm_mat4_identity(global_projection);

    global_last_frame = clock();
    memset(key_states, 0, sizeof(key_states));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    //
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);
    //
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    generate_chunks();
}

static void mouse_motion(int x, int y)
{
    global_camera_rotation[0] += glm_rad(((float)y - global_height / 2) / 10);
    global_camera_rotation[0] = fmaxf(fminf(glm_rad(85.f), global_camera_rotation[0]), glm_rad(-85.f));
    global_camera_rotation[1] += glm_rad(((float)x - global_width / 2) / 10);
    global_camera_rotation[1] = fmodf(global_camera_rotation[1], GLM_PIf * 2);
}

static void reset_mouse()
{
    glutWarpPointer(global_width / 2, global_height / 2);
    glutSetCursor(GLUT_CURSOR_NONE);
}

static void Resize(int w, int h)
{
    global_width = w > 1 ? w : 1;
    global_height = h > 1 ? h : 1;
    glViewport(0, 0, global_width, global_height);
    
    glClearDepth(1.0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

static float time_passed = 0.0f;
static int frames = 0;

static void Render(void)
{
    glClearColor(0.0f, 0.2f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    clock_t time = clock();
    float delta = ((float) (time - global_last_frame)) / CLOCKS_PER_SEC;
    time_passed += delta;
    global_last_frame = time;

    reset_mouse();

    float speed = 150.0f;
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
    glm_vec3_mul(direction, (vec3){delta * speed, -delta * speed, delta * speed}, direction);
    glm_vec3_add(global_camera_position, direction, global_camera_position);

    frames++;
    if(time_passed >= 1.0f)
    {
        printf("fps: %d\n", frames);
        frames = 0;
        time_passed -= 1.0f;
    }
    
    render_chunks();

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    srand(time(0));
    if(gladLoadGL()) {
        // you need an OpenGL context before loading glad
        printf("I did load GL with no context!\n");
        exit(-1);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(global_width, global_height);
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
