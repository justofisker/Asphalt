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
#include "Input.h"
#include "PostProcess.h"
#include "Render.h"

static char paused = 0;
unsigned int postprocess_shader;
#define PLAYER_SIZE 0.2f
#define PLAYER_HEIGHT 1.65f
vec3 sky_color = {0.4f, 0.5f, 1.0f};

Mesh* look_block_mesh;
static void setup_look_block()
{
    typedef struct _Vertex
    {
        vec3 position;
    } Vertex;

    typedef unsigned short Index;

    Vertex verticies[8] = {
        { 1.0f,  1.0f, -0.0f},
        { 1.0f, -0.0f, -0.0f},
        {-0.0f, -0.0f, -0.0f},
        {-0.0f,  1.0f, -0.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f, -0.0f,  1.0f},
        {-0.0f, -0.0f,  1.0f},
        {-0.0f,  1.0f,  1.0f},
    };

    Index indicies[36] = {
        2, 1, 0,
        0, 3, 2,
        1, 5, 4,
        4, 0, 1,
        4, 5, 6,
        6, 7, 4,
        7, 6, 2,
        2, 3, 7,
        6, 5, 1,
        1, 2, 6,
        0, 4, 7,
        7, 3, 0,
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VB, IB;
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
    glBindVertexArray(0);

    Mesh* mesh = malloc(sizeof(Mesh));
    mesh->array_object = VAO;
    mesh->vertex_buffer = VB;
    mesh->index_buffer = IB;
    mesh->index_count = 36;
    mesh->index_type = GL_UNSIGNED_SHORT;

    look_block_mesh = mesh;
}
static void render_look_block(int x, int y, int z)
{
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){x, y, z});
    glUseProgram(global_color_shader);
    glUniformMatrix4fv(global_color_model_loc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(global_color_view_loc, 1, GL_FALSE, global_view[0]);
    glUniformMatrix4fv(global_color_projection_loc, 1, GL_FALSE, global_projection[0]);
    glUniform4f(global_color_color_loc, 0.0f, 0.0f, 0.0f, 1.0f);
    glBindVertexArray(look_block_mesh->array_object);
    glLineWidth(1.5f);
    glDrawElements(GL_LINES, look_block_mesh->index_count, look_block_mesh->index_type, (void*)0);
    glBindVertexArray(0);
}

static void setup()
{
    global_block_shader = compile_shader("res/shader/block_vertex.glsl", "res/shader/block_fragment.glsl");
    postprocess_shader = compile_shader("res/shader/postprocess_vertex.glsl", "res/shader/postprocess_fragment.glsl");
    global_color_shader = compile_shader("res/shader/color_vertex.glsl", "res/shader/color_fragment.glsl");
    global_texture = create_texture("res/texture/blocks.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 0, 0.0f);

    global_block_model_loc = glGetUniformLocation(global_block_shader, "u_Model");
    global_block_view_loc = glGetUniformLocation(global_block_shader, "u_View");
    global_block_projection_loc = glGetUniformLocation(global_block_shader, "u_Projection");
    global_block_texture_loc = glGetUniformLocation(global_block_shader, "u_Texture");
    global_block_view_near_loc = glGetUniformLocation(global_block_shader, "u_ViewNear");
    global_block_view_far_loc = glGetUniformLocation(global_block_shader, "u_ViewFar");
    global_color_model_loc = glGetUniformLocation(global_color_shader, "u_Model");
    global_color_view_loc = glGetUniformLocation(global_color_shader, "u_View");
    global_color_projection_loc = glGetUniformLocation(global_color_shader, "u_Projection");
    global_color_color_loc = glGetUniformLocation(global_color_shader, "u_Color");

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

    setup_blocks();
    setup_input();
    setup_postprocess();
    set_mouse_mode(MOUSEMODE_CAPTURED);
    setup_chunk_thread();
    setup_look_block();
}

static void Resize(int w, int h)
{
    global_width = w > 1 ? w : 1;
    global_height = h > 1 ? h : 1;
    glViewport(0, 0, global_width, global_height);
    
    resize_postprocess();

    glClearDepth(1.0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

float time_passed = 0.0f;
int frames = 0;
float time_since_space = 1.f;
float time_since_forward = 1.f;
char sprint_mode = 0;
char fly_mode = 0;

vec3 velocity = GLM_VEC3_ZERO_INIT;
char on_ground = 0;
int block_selected = 1;
char fullscreen = 0;

static void Render(void)
{
    input_render_start();

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
    if(is_key_just_pressed(' ') && !paused)
    {
        if(time_since_space < 0.25)
            fly_mode = !fly_mode;
        else
            time_since_space = 0.f;
    }
    time_since_forward += delta;
    if(is_key_just_pressed('w') && !paused)
    {
        if(time_since_forward < 0.25)
            sprint_mode = 1;
        else
            time_since_forward = 0.f;
    }
    if(is_key_just_pressed('r') && !paused)
    {
        sprint_mode = 1;
    }
    if(!is_key_pressed('w') || paused)
    {
        sprint_mode = 0;
    }
    if(is_key_just_pressed('f'))
    {
        fullscreen = !fullscreen;
        if(fullscreen)
        {
            glutFullScreen();
        }
        else
        {
            glutPositionWindow(glutGet(GLUT_SCREEN_WIDTH) / 2 - 1280 / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - 720 / 2);
            glutReshapeWindow(1280, 720);
        }
    }



    // movement
    if(1) {
        if(on_ground)
            fly_mode = 0;

        if(fly_mode)
            glm_vec3_zero(velocity);

        if(!fly_mode)
    {
        vec3 direction = {0, 0, 0};
        float speed = 0.0f;
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
                velocity[1] = fmaxf(-3.f, velocity[1]);
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
                    velocity[1] = fminf(velocity[1], 4.f);
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
        if(sprint_mode)
            speed *= 1.65f;
        glm_normalize(direction);
        glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
        glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
        glm_vec3_add(movement, direction, movement);
    }
        else
        {
            float speed = 150.f;
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
            if(sprint_mode)
                speed *= 1.65f;
            glm_normalize(direction);
            glm_vec3_rotate(direction, -global_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
            glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
            glm_vec3_add(movement, direction, movement);
        }
    
        vec3 velocity_movement;
        glm_vec3_copy(velocity, velocity_movement);
        glm_vec3_mul(velocity_movement, (vec3){delta, delta, delta}, velocity_movement);
        glm_vec3_add(movement, velocity_movement, movement);
    }

    // Collison Detection
    if(1){
        char change_y = (int)floorf(global_player_position[1] + movement[1]) - (int)floorf(global_player_position[1]);

        char change_x_positive = (int)floorf(global_player_position[0] + movement[0] + PLAYER_SIZE) - (int)floorf(global_player_position[0] + PLAYER_SIZE);
        char change_z_positive = (int)floorf(global_player_position[2] + movement[2] + PLAYER_SIZE) - (int)floorf(global_player_position[2] + PLAYER_SIZE);
        char change_x_negative = (int)floorf(global_player_position[0] + movement[0] - PLAYER_SIZE) - (int)floorf(global_player_position[0] - PLAYER_SIZE);
        char change_z_negative = (int)floorf(global_player_position[2] + movement[2] - PLAYER_SIZE) - (int)floorf(global_player_position[2] - PLAYER_SIZE);
        char change_x = (change_x_positive > 0) ? change_x_positive : (change_x_negative < 0) ? change_x_negative : 0;
        char change_z = (change_z_positive > 0) ? change_z_positive : (change_z_negative < 0) ? change_z_negative : 0;

        int y_1 = (int)floorf(global_player_position[1] + 0.f);
        int y_2 = (int)floorf(global_player_position[1] + 1.0f);
        int y_3 = (int)floorf(global_player_position[1] + PLAYER_HEIGHT);

        if(change_y < 0)
        {
            if(
                (get_block(get_block_id_at(floorf(global_player_position[0] - PLAYER_SIZE), y_1 + change_y, floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at(floorf(global_player_position[0] + PLAYER_SIZE), y_1 + change_y, floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at(floorf(global_player_position[0] + PLAYER_SIZE), y_1 + change_y, floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at(floorf(global_player_position[0] - PLAYER_SIZE), y_1 + change_y, floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[1] = (float)(y_1) - global_player_position[1];
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
                    movement[1] = (float)(y_3 + 1) - (global_player_position[1] + PLAYER_HEIGHT);
                    velocity[1] = 0.0f;
                }
            }
        }

        int y_0 = (int)floorf(global_player_position[1] + movement[1]);

        if(change_x && change_z)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_0, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_1, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_2, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_3, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                if(fabsf(movement[0]) > fabsf(movement[2]))
                {
                    if(
                        !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_0, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_1, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_2, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_3, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    )
                    {
                        movement[2] = 0.f;   
                    }
                }
                else if(fabsf(movement[2]) > fabsf(movement[0]))
                {
                    if(
                       !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_0, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_1, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_2, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(get_block(get_block_id_at((int)floorf(global_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_3, (int)floorf(global_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    )
                    {
                        movement[0] = 0.f;  
                    }
                }
                else
                {
                    movement[0] = 0.f;
                    movement[2] = 0.f;
                }
            }
        }

        change_x_positive = (int)floorf(global_player_position[0] + movement[0] + PLAYER_SIZE) - (int)floorf(global_player_position[0] + PLAYER_SIZE);
        change_z_positive = (int)floorf(global_player_position[2] + movement[2] + PLAYER_SIZE) - (int)floorf(global_player_position[2] + PLAYER_SIZE);
        change_x_negative = (int)floorf(global_player_position[0] + movement[0] - PLAYER_SIZE) - (int)floorf(global_player_position[0] - PLAYER_SIZE);
        change_z_negative = (int)floorf(global_player_position[2] + movement[2] - PLAYER_SIZE) - (int)floorf(global_player_position[2] - PLAYER_SIZE);
        change_x = (change_x_positive > 0) ? change_x_positive : (change_x_negative < 0) ? change_x_negative : 0;
        change_z = (change_z_positive > 0) ? change_z_positive : (change_z_negative < 0) ? change_z_negative : 0;

        if(change_x)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_0, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_1, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_2, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_3, (int)floorf(global_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_0, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_1, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_2, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0]) + change_x, y_3, (int)floorf(global_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[0] = 0.f;
            }
        }
        if(change_z)
        {
            if(
                (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_0, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_1, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_2, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] + PLAYER_SIZE), y_3, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_0, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_1, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_2, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block(get_block_id_at((int)floorf(global_player_position[0] - PLAYER_SIZE), y_3, (int)floorf(global_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[2] = 0.f;
            }
        }
    }
    
    glm_vec3_add(global_player_position, movement, global_player_position);
    
    char looking_at_block = 0;
    int look_block_pos[3];

    // Block Break Raycast
    if(1){
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

                int block_pos[3] = {floorf(position[0]), floorf(position[1]), floorf(position[2])};

                char found_block = !(get_block(get_block_id_at(block_pos[0], block_pos[1], block_pos[2]))->flags & BLOCKFLAG_NO_COLLISION);

                if(found_block) {
                    looking_at_block = 1;
                    memcpy_s(look_block_pos, sizeof(look_block_pos), block_pos, sizeof(block_pos));

                    if(!paused && is_mouse_button_just_pressed(GLUT_LEFT_BUTTON))
                    {
                        set_block_at(block_pos[0], block_pos[1], block_pos[2], 0);
                    }

                    break;
                }
            }
        }
    }

    if(is_key_just_pressed('1'))
        block_selected = BLOCK_GRASS;
    if(is_key_just_pressed('2'))
        block_selected = BLOCK_DIRT;
    if(is_key_just_pressed('3'))
        block_selected = BLOCK_STONE;
    if(is_key_just_pressed('4'))
        block_selected = BLOCK_SAND;
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
                    glm_vec3_sub(position, ray_inc, position);

                    int block_pos[3] = { floorf(position[0]), floorf(position[1]), floorf(position[2]) };

                    {
                        if( ((int)floorf(global_player_position[0]) == block_pos[0] && (int)floorf(global_player_position[1]) == block_pos[1] && (int)floorf(global_player_position[2]) == block_pos[2])
                         || ((int)floorf(global_player_position[0]) == block_pos[0] && (int)floorf(global_player_position[1] + 1.0f) == block_pos[1] && (int)floorf(global_player_position[2]) == block_pos[2])
                         || ((int)floorf(global_player_position[0]) == block_pos[0] && (int)floorf(global_player_position[1] + PLAYER_HEIGHT) == block_pos[1] && (int)floorf(global_player_position[2]) == block_pos[2]) )
                        {
                            break;
                        }
                    }

                    set_block_at(floorf(position[0]), floorf(position[1]), floorf(position[2]), block_selected);

                    break;
                }
            }
        }
    }

    render_start_postprocess();
    render_begin();
    {
        if(looking_at_block) render_look_block(look_block_pos[0], look_block_pos[1], look_block_pos[2]);
        render_chunks();
        render_end_postprocess();
        glUseProgram(postprocess_shader);
        int u_bInWater = glGetUniformLocation(postprocess_shader, "u_bInWater");
        glUniform1i (u_bInWater, (get_block_id_at(floorf(global_player_position[0]), floorf(global_player_position[1] + global_camera_offset[1]), floorf(global_player_position[2])) == BLOCK_WATER) );
        int u_ScreenHeight = glGetUniformLocation(postprocess_shader, "u_ScreenHeight");
        glUniform1i(u_ScreenHeight, global_height);
        int u_ScreenWidth = glGetUniformLocation(postprocess_shader, "u_ScreenWidth");
        glUniform1i(u_ScreenWidth, global_width);
        int u_SkyColor = glGetUniformLocation(postprocess_shader, "u_SkyColor");
        glUniform3fv(u_SkyColor, 1, sky_color);
        do_postprocess(postprocess_shader, 0, 1);
        glDisable(GL_DEPTH_TEST);

        input_render_end();
    }
    render_end();
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
