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
#include "Block.h"
#include "Sprite.h"
#include "Input.h"

static char paused = 0;

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

Sprite *crosshair;

static void setup()
{
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    global_basic_shader = compile_shader("res/shader/basic_vertex.glsl", "res/shader/basic_fragment.glsl");
    global_texture = create_texture("res/texture/blocks.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 0, 0.0f);

    global_basic_model_loc = glGetUniformLocation(global_basic_shader, "u_Model");
    global_basic_view_loc = glGetUniformLocation(global_basic_shader, "u_View");
    global_basic_projection_loc = glGetUniformLocation(global_basic_shader, "u_Projection");
    global_basic_texture_loc = glGetUniformLocation(global_basic_shader, "u_Texture");

    crosshair = create_sprite(create_texture("res/texture/crosshair.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, 0, 0.0f));

    glm_vec3_zero(global_camera_position);
    global_camera_position[1] = 40.0f;
    glm_vec3_zero(global_camera_rotation);
    glm_mat4_identity(global_view);
    glm_mat4_identity(global_projection);

    global_last_frame = clock();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    generate_chunks();
    setup_blocks();
    setup_input();
    set_mouse_mode(MOUSEMODE_CAPTURED);
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
    input_render_start();
    glClearColor(0.0f, 0.2f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    clock_t time = clock();
    float delta = ((float) (time - global_last_frame)) / CLOCKS_PER_SEC;
    time_passed += delta;
    global_last_frame = time;

    if(is_key_just_pressed(INPUT_KEY_ESCAPE))
    {
        paused = !paused;
        if(paused)
            set_mouse_mode(MOUSEMODE_CURSOR);
        else
            set_mouse_mode(MOUSEMODE_CAPTURED);
    }

    if(!paused)
    {
        int motion_x, motion_y;
        get_mouse_motion(&motion_x, &motion_y);
        global_camera_rotation[0] += glm_rad((float)motion_y / 10);
        global_camera_rotation[0] = fmaxf(fminf(glm_rad(90.f), global_camera_rotation[0]), glm_rad(-90.f));
        global_camera_rotation[1] += glm_rad((float)motion_x / 10);
        global_camera_rotation[1] = fmodf(global_camera_rotation[1], GLM_PIf * 2);
    }

    if(!paused)
    {
        float speed = 10.0f;
        vec3 direction = {0, 0, 0};
        if(is_key_pressed('a'))
            direction[0] -= 1.0f;
        if(is_key_pressed('d'))
            direction[0] += 1.0f;
        if(is_key_pressed('w'))
            direction[2] -= 1.0f;
        if(is_key_pressed('s'))
            direction[2] += 1.0f;
        if(is_key_pressed(' '))
            direction[1] += 1.0f;
#ifdef _WIN32
        if(GetKeyState(VK_SHIFT) & 0x8000)
#else
        if(is_key_pressed('c'))
#endif
            direction[1] -= 1.0f;
        glm_normalize(direction);
        glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
        glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
        glm_vec3_add(global_camera_position, direction, global_camera_position);
    }
    

    frames++;
    if(time_passed >= 1.0f)
    {
        frames = 0;
        time_passed -= 1.0f;
    }

    // Raycast
    if(!paused && is_mouse_button_just_pressed(GLUT_LEFT_BUTTON)){
        float ray_inc = 0.05f;
        float max_distance = 10.0f;

        vec3 origin;
        vec3 direction = {0.0f, 0.0f, -1.0f};

        glm_vec3_copy(global_camera_position, origin);
        glm_vec3_rotate(direction, -global_camera_rotation[0], (vec3){1.0f, 0.0f, 0.0f});
        glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.0f, 1.0f, 0.0f});
        glm_normalize(direction);
        glm_vec3_mul(direction, (vec3){ray_inc, ray_inc, ray_inc}, direction);

        {
            vec3 target = {0.0f, 0.0f, 0.0f};
            while((target[0]*target[0] + target[1]*target[1] + target[2]*target[2]) < max_distance*max_distance)
            {
                glm_vec3_add(target, direction, target);

                vec3 position;
                glm_vec3_add(target, origin, position);

                short id = get_block_id_at(floorf(position[0]), floorf(position[1]), floorf(position[2]));

                if(id)
                {
                    set_block_at(floorf(position[0]), floorf(position[1]), floorf(position[2]), 0);
                    break;
                }
            }
        }
    }


    render_chunks();

    crosshair->position[0] = global_width / 2;
    crosshair->position[1] = global_height / 2;
    draw_sprite(crosshair);

    input_render_end();

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
    glutKeyboardFunc(keyboard_func);
    glutKeyboardUpFunc(keyboard_func_up);
    glutSpecialFunc(special_func);
    glutSpecialUpFunc(special_func_up);
    glutPassiveMotionFunc(mouse_motion_func);
    glutMotionFunc(mouse_motion_func);
    glutMouseFunc(mouse_func);

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
