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
#include "PostProcess.h"

static char paused = 0;
unsigned int post_process_water, post_process_invert;
#define PLAYER_SIZE 0.15f
#define PLAYER_HEIGHT 1.65f

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
    post_process_water = compile_shader("res/shader/post_process_vertex.glsl", "res/shader/water_fragment.glsl");
    post_process_invert = compile_shader("res/shader/post_process_vertex.glsl", "res/shader/invert_fragment.glsl");
    global_texture = create_texture("res/texture/blocks.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 0, 0.0f);

    global_basic_model_loc = glGetUniformLocation(global_basic_shader, "u_Model");
    global_basic_view_loc = glGetUniformLocation(global_basic_shader, "u_View");
    global_basic_projection_loc = glGetUniformLocation(global_basic_shader, "u_Projection");
    global_basic_texture_loc = glGetUniformLocation(global_basic_shader, "u_Texture");

    crosshair = create_sprite(create_texture("res/texture/crosshair.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, 0, 0.0f));

    glm_vec3_zero(global_player_position);
    global_player_position[1] = 255.0f;
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
    setup_postprocess();
    set_mouse_mode(MOUSEMODE_CAPTURED);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
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

float time_passed = 0.0f;
int frames = 0;
float time_since_space = 1.f;
char fly_mode = 0;

vec3 velocity = GLM_VEC3_ZERO_INIT;
char on_ground = 0;
int block_selected = 1;

static void Render(void)
{
    input_render_start();
    glClearColor(0.0f, 0.2f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    clock_t time = clock();
    float delta = ((float) (time - global_last_frame)) / CLOCKS_PER_SEC;
    delta = fminf(delta, 0.05f);
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

    vec3 movement = GLM_VEC3_ZERO_INIT;

    time_since_space += delta;
    if(is_key_just_pressed(' '))
    {
        if(time_since_space < 0.5)
            fly_mode = !fly_mode;
        else
            time_since_space = 0.f;
    }
    if(on_ground)
        fly_mode = 0;

    if(fly_mode)
        glm_vec3_zero(velocity);

    if(!fly_mode)
    {
        vec3 direction = {0, 0, 0};
        float speed;
        // Normal Walk
        char in_fluid = (get_block(get_block_id_at(floorf(global_player_position[0]),floorf(global_player_position[1]),floorf(global_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT)
                     || (get_block(get_block_id_at(floorf(global_player_position[0]),floorf(global_player_position[1] + 1.0f),floorf(global_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT)
                     || (get_block(get_block_id_at(floorf(global_player_position[0]),floorf(global_player_position[1] + PLAYER_HEIGHT),floorf(global_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT);
        if(in_fluid)
        {
            if(velocity[1] < -3.f)
                velocity[1] += 50.f * delta;
            else
            {
                velocity[1] -= 5.f * delta;
                velocity[1] = max(-3.f, velocity[1]);
            }
            if (!paused) {
                speed = 4.0f;
                if(is_key_pressed('a'))
                    direction[0] -= 1.0f;
                if(is_key_pressed('d'))
                    direction[0] += 1.0f;
                if(is_key_pressed('w'))
                    direction[2] -= 1.0f;
                if(is_key_pressed('s'))
                    direction[2] += 1.0f;
                if(is_key_pressed(' '))
                {
                    velocity[1] += 14.f * delta;
                    velocity[1] = min(velocity[1], 4.f);
                }
            }
        }
        else
        {
            velocity[1] -= 24.0f * delta;
            if (!paused) {
                speed = 4.0f;
                if(is_key_pressed('a'))
                    direction[0] -= 1.0f;
                if(is_key_pressed('d'))
                    direction[0] += 1.0f;
                if(is_key_pressed('w'))
                    direction[2] -= 1.0f;
                if(is_key_pressed('s'))
                    direction[2] += 1.0f;
                if(is_key_pressed(' ') && on_ground)
                    velocity[1] = 8.f;
            }
        }
        
        glm_normalize(direction);
        glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
        glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
        glm_vec3_add(movement, direction, movement);
    }
    else
    {
        float speed = 50.f;
        vec3 direction = {0, 0, 0};
        // Fly
        if(!paused) {
            if(is_key_pressed('a'))
                direction[0] -= 1.0f;
            if(is_key_pressed('d'))
                direction[0] += 1.0f;
            if(is_key_pressed('w'))
                direction[2] -= 1.0f;
            if(is_key_pressed('s'))
                direction[2] += 1.0f;
            if(is_key_pressed(' '))
                velocity[1] = 10.0f;
#ifdef _WIN32
            if(GetKeyState(VK_SHIFT) & 0x8000)
#else
            if(is_key_pressed('c'))
#endif
                velocity[1] = -10.0f;
        }
        glm_normalize(direction);
        glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
        glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
        glm_vec3_add(movement, direction, movement);
    }

    vec3 velocity_movement;
    glm_vec3_copy(velocity, velocity_movement);
    glm_vec3_mul(velocity_movement, (vec3){delta, delta, delta}, velocity_movement);
    glm_vec3_add(movement, velocity_movement, movement);

    // Collison Detection
    {
        //char change_x_positive = 
        char change_y_negative = (int)floorf(global_player_position[1] + movement[1]) - (int)floorf(global_player_position[1]);
        char change_x_positive = (int)floorf(global_player_position[0] + movement[0] + PLAYER_SIZE) - (int)floorf(global_player_position[0] + PLAYER_SIZE);
        char change_x_negative = (int)floorf(global_player_position[0] + movement[0] - PLAYER_SIZE) - (int)floorf(global_player_position[0] - PLAYER_SIZE);
        char change_z_positive = (int)floorf(global_player_position[2] + movement[2] + PLAYER_SIZE) - (int)floorf(global_player_position[2] + PLAYER_SIZE);
        char change_z_negative = (int)floorf(global_player_position[2] + movement[2] - PLAYER_SIZE) - (int)floorf(global_player_position[2] - PLAYER_SIZE);

        int y_1 = (int)floorf(global_player_position[1]) + (int)floorf(fmodf(global_player_position[1], 1.0f) + 0.f);
        int y_2 = (int)floorf(global_player_position[1]) + (int)floorf(fmodf(global_player_position[1], 1.0f) + 1.0f);
        int y_3 = (int)floorf(global_player_position[1]) + (int)floorf(fmodf(global_player_position[1], 1.0f) + PLAYER_HEIGHT);

        if(change_x_positive > 0)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_positive, y_1, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_positive, y_2, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_positive, y_3, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_positive, y_1, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_positive, y_2, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_positive, y_3, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[0] = 0.f;
            }
        } else if (change_x_negative < 0)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_negative, y_1, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_negative, y_2, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_negative, y_3, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_negative, y_1, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_negative, y_2, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x_negative, y_3, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[0] = 0.f;
            }
        }
        if(change_z_positive > 0)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_1, (int)floorf(global_player_position[2]) + change_z_positive))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_2, (int)floorf(global_player_position[2]) + change_z_positive))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_3, (int)floorf(global_player_position[2]) + change_z_positive))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_1, (int)floorf(global_player_position[2]) + change_z_positive))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_2, (int)floorf(global_player_position[2]) + change_z_positive))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_3, (int)floorf(global_player_position[2]) + change_z_positive))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[2] = 0.f;
            }
        } else if (change_z_negative < 0)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_1, (int)floorf(global_player_position[2]) + change_z_negative))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_2, (int)floorf(global_player_position[2]) + change_z_negative))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_3, (int)floorf(global_player_position[2]) + change_z_negative))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_1, (int)floorf(global_player_position[2]) + change_z_negative))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_2, (int)floorf(global_player_position[2]) + change_z_negative))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_3, (int)floorf(global_player_position[2]) + change_z_negative))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[2] = 0.f;
            }
        }

        if(change_y_negative < 0)
        {
            if(
                (get_block(get_block_id_at(floorf(global_player_position[0] - PLAYER_SIZE), y_1 + change_y_negative, floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at(floorf(global_player_position[0] + PLAYER_SIZE), y_1 + change_y_negative, floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at(floorf(global_player_position[0] + PLAYER_SIZE), y_1 + change_y_negative, floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at(floorf(global_player_position[0] - PLAYER_SIZE), y_1 + change_y_negative, floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[1] = 0.0f;
                velocity[1] = 0.0f;
                on_ground = 1;
            } else on_ground = 0;       
        } else
        {
            on_ground = 0;
            if(movement[1] > 0.f)
            {
                if(
                    (get_block(get_block_id_at(floorf(global_player_position[0]), floorf(global_player_position[1] + movement[1] + PLAYER_HEIGHT), floorf(global_player_position[2])))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
                {
                    movement[1] = 0.0f;
                    velocity[1] = 0.0f;
                }
            }
        }
    }
    
    glm_vec3_add(global_player_position, movement, global_player_position);
    

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

        glm_vec3_copy(global_player_position, origin);
        glm_vec3_add(origin, global_camera_offset, origin);
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

                char found_block = !(get_block(get_block_id_at(floorf(position[0]), floorf(position[1]), floorf(position[2])))->flags & BLOCKFLAG_NO_COLLISION);

                if(found_block)
                {
                    set_block_at(floorf(position[0]), floorf(position[1]), floorf(position[2]), 0);
                    break;
                }
            }
        }
    }

    if(is_key_just_pressed('1'))
        block_selected = 1;
    if(is_key_just_pressed('2'))
        block_selected = 2;
    if(is_key_just_pressed('3'))
        block_selected = 3;
    // Raycast Place BLock
    if(!paused && is_mouse_button_just_pressed(GLUT_RIGHT_BUTTON))
    {
        float max_distance = 10.0f;

        vec3 origin;
        vec3 direction = {0.0f, 0.0f, -1.0f};
        vec3 ray_inc = {0.05f, 0.05f, 0.05f};

        glm_vec3_copy(global_player_position, origin);
        glm_vec3_add(origin, global_camera_offset, origin);
        glm_vec3_rotate(direction, -global_camera_rotation[0], (vec3){1.0f, 0.0f, 0.0f});
        glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.0f, 1.0f, 0.0f});
        glm_normalize(direction);
        glm_vec3_mul(ray_inc, direction, ray_inc);

        {
            vec3 target = {0.0f, 0.0f, 0.0f};
            while((target[0]*target[0] + target[1]*target[1] + target[2]*target[2]) < max_distance*max_distance)
            {
                glm_vec3_add(target, ray_inc, target);

                vec3 position;
                glm_vec3_add(target, origin, position);

                char found_block = !(get_block(get_block_id_at(floorf(position[0]), floorf(position[1]), floorf(position[2])))->flags & BLOCKFLAG_NO_COLLISION);

                if(found_block)
                {
                    int player_pos[3] = { floorf(global_player_position[0]), floorf(global_player_position[1]), floorf(global_player_position[2]) };
                    int block_pos[3] = { floorf(position[0]), floorf(position[1]), floorf(position[2]) };

                    if(player_pos[0] == block_pos[0] && player_pos[2] == block_pos[2] && (player_pos[1] == block_pos[1] || player_pos[1] + 1 == block_pos[1]))
                        break;


                    glm_vec3_sub(position, ray_inc, position);

                    set_block_at(floorf(position[0]), floorf(position[1]), floorf(position[2]), block_selected);

                    break;
                }
            }
        }
    }

    render_start_postprocess();
    glClearColor(107.f / 255.f, 213.f / 255.f, 234.f / 255.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_chunks();
    render_end_postprocess();
    vec4 water_color = {1.0f, 1.0f, 1.0f, 1.0f};
    if(get_block_id_at(floorf(global_player_position[0]), floorf(global_player_position[1] + global_camera_offset[1]), floorf(global_player_position[2])) == BLOCK_WATER)
    {
        water_color[0] = 0.5f;
        water_color[1] = 0.5f;
        water_color[2] = 1.0f;
        water_color[3] = 1.0f;
    }
    glUseProgram(post_process_water);
    int water_color_loc = glGetUniformLocation(post_process_water, "water_color");
    glUniform4fv(water_color_loc, 1, water_color);
    do_postprocess(post_process_water, 0);

    crosshair->position[0] = global_width / 2;
    crosshair->position[1] = global_height / 2;
    glDisable(GL_DEPTH_TEST);
    draw_sprite(crosshair);
    glEnable(GL_DEPTH_TEST);

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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    
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
