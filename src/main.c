#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#ifdef MINGW
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
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
#include "TextRenderer.h"
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
    glm_translate(model, (vec3){x  - g_player_position[0], y - g_player_position[1], z - g_player_position[2]});
    glUseProgram(g_color_shader);
    glUniformMatrix4fv(g_color_model_loc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(g_color_view_loc, 1, GL_FALSE, g_view[0]);
    glUniformMatrix4fv(g_color_projection_loc, 1, GL_FALSE, g_projection[0]);
    glUniform4f(g_color_color_loc, 0.0f, 0.0f, 0.0f, 1.0f);
    glBindVertexArray(look_block_mesh->array_object);
    glLineWidth(1.5f);
    glDrawElements(GL_LINES, look_block_mesh->index_count, look_block_mesh->index_type, (void*)0);
    glBindVertexArray(0);
}

static Font *font_arial;


Uint64 last_frame = 0;

static void setup()
{
    g_block_shader = compile_shader("res/shader/block_vertex.glsl", "res/shader/block_fragment.glsl");
    postprocess_shader = compile_shader("res/shader/postprocess_vertex.glsl", "res/shader/postprocess_fragment.glsl");
    g_color_shader = compile_shader("res/shader/color_vertex.glsl", "res/shader/color_fragment.glsl");
    g_texture = create_texture("res/texture/blocks.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 0, 0.0f);

    g_block_model_loc = glGetUniformLocation(g_block_shader, "u_Model");
    g_block_view_loc = glGetUniformLocation(g_block_shader, "u_View");
    g_block_projection_loc = glGetUniformLocation(g_block_shader, "u_Projection");
    g_block_texture_loc = glGetUniformLocation(g_block_shader, "u_Texture");
    g_block_view_near_loc = glGetUniformLocation(g_block_shader, "u_ViewNear");
    g_block_view_far_loc = glGetUniformLocation(g_block_shader, "u_ViewFar");
    g_color_model_loc = glGetUniformLocation(g_color_shader, "u_Model");
    g_color_view_loc = glGetUniformLocation(g_color_shader, "u_View");
    g_color_projection_loc = glGetUniformLocation(g_color_shader, "u_Projection");
    g_color_color_loc = glGetUniformLocation(g_color_shader, "u_Color");

    glm_vec3_zero(g_player_position);
    g_player_position[1] = 255.0f;
    glm_vec3_zero(g_camera_rotation);
    glm_mat4_identity(g_view);
    glm_mat4_identity(g_projection);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    last_frame = SDL_GetPerformanceCounter();
    setup_blocks();
    setup_input();
    setup_postprocess();
    set_mouse_mode(MOUSEMODE_CAPTURED);
    setup_chunk_thread();
    setup_look_block();
    setup_textrenderer();
    begin_create_font();
    font_arial = create_font("res/fonts/Arial.ttf", 24);
    end_create_font();
}

static void Resize(int w, int h)
{
    g_width = w > 1 ? w : 1;
    g_height = h > 1 ? h : 1;
    glViewport(0, 0, g_width, g_height);
    
    resize_postprocess();

    glClearDepth(1.0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

float time_passed = 0.0f;
int frames = 0;
int fps = 0;
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

    Uint64 now = SDL_GetPerformanceCounter();
    float delta = (float)(now - last_frame) / SDL_GetPerformanceFrequency();
    last_frame = now;
    if(delta >= 0.1f) delta = 0.1f;

    time_passed += delta;

    if(is_key_just_pressed(SDLK_ESCAPE))
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
        g_camera_rotation[0] += glm_rad((float)motion_y / 10);
        g_camera_rotation[0] = fmaxf(fminf(glm_rad(90.f), g_camera_rotation[0]), glm_rad(-90.f));
        g_camera_rotation[1] += glm_rad((float)motion_x / 10);
        g_camera_rotation[1] = fmodf(g_camera_rotation[1], GLM_PIf * 2);
    }

    vec3 movement = GLM_VEC3_ZERO_INIT;

    time_since_space += delta;
    if(is_key_just_pressed(SDLK_SPACE) && !paused)
    {
        if(time_since_space < 0.25)
            fly_mode = !fly_mode;
        else
            time_since_space = 0.f;
    }
    time_since_forward += delta;
    if(is_key_just_pressed(SDLK_w) && !paused)
    {
        if(time_since_forward < 0.25)
            sprint_mode = 1;
        else
            time_since_forward = 0.f;
    }
    if(is_key_just_pressed(SDLK_r) && !paused)
    {
        sprint_mode = 1;
    }
    if(!is_key_pressed(SDLK_w) || paused)
    {
        sprint_mode = 0;
    }
    if(is_key_just_pressed(SDLK_f))
    {
        fullscreen = !fullscreen;
        if(fullscreen)
        {
            SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else
        {
            SDL_SetWindowFullscreen(g_window, 0);
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
        char in_fluid = (get_block_info(get_block_id_at(floorf(g_player_position[0]),floorf(g_player_position[1]),floorf(g_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT)
                     || (get_block_info(get_block_id_at(floorf(g_player_position[0]),floorf(g_player_position[1] + 1.0f),floorf(g_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT)
                     || (get_block_info(get_block_id_at(floorf(g_player_position[0]),floorf(g_player_position[1] + PLAYER_HEIGHT),floorf(g_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT);
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
                if(is_key_pressed(SDLK_a))
                    direction[0] -= 1.0f;
                if(is_key_pressed(SDLK_d))
                    direction[0] += 1.0f;
                if(is_key_pressed(SDLK_w))
                    direction[2] -= 1.0f;
                if(is_key_pressed(SDLK_s))
                    direction[2] += 1.0f;
                if(is_key_pressed(SDLK_SPACE))
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
                if(is_key_pressed(SDLK_a))
                    direction[0] -= 1.0f;
                if(is_key_pressed(SDLK_d))
                    direction[0] += 1.0f;
                if(is_key_pressed(SDLK_w))
                    direction[2] -= 1.0f;
                if(is_key_pressed(SDLK_s))
                    direction[2] += 1.0f;
                if(is_key_pressed(SDLK_SPACE) && on_ground)
                    velocity[1] = 8.f;
            }
        }
        if(sprint_mode)
            speed *= 1.65f;
        glm_normalize(direction);
        glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
        glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
        glm_vec3_add(movement, direction, movement);
    }
        else
        {
            float speed = 150.f;
            vec3 direction = {0, 0, 0};
            // Fly
            if(!paused) {
                if(is_key_pressed(SDLK_a))
                    direction[0] -= 1.0f;
                if(is_key_pressed(SDLK_d))
                    direction[0] += 1.0f;
                if(is_key_pressed(SDLK_w))
                    direction[2] -= 1.0f;
                if(is_key_pressed(SDLK_s))
                    direction[2] += 1.0f;
                if(is_key_pressed(SDLK_SPACE))
                    velocity[1] = 10.0f;
                if(SDL_GetModState() & KMOD_SHIFT)
                    velocity[1] = -10.0f;
            }
            if(sprint_mode)
                speed *= 1.65f;
            glm_normalize(direction);
            glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
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
        char change_y = (int)floorf(g_player_position[1] + movement[1]) - (int)floorf(g_player_position[1]);

        char change_x_positive = (int)floorf(g_player_position[0] + movement[0] + PLAYER_SIZE) - (int)floorf(g_player_position[0] + PLAYER_SIZE);
        char change_z_positive = (int)floorf(g_player_position[2] + movement[2] + PLAYER_SIZE) - (int)floorf(g_player_position[2] + PLAYER_SIZE);
        char change_x_negative = (int)floorf(g_player_position[0] + movement[0] - PLAYER_SIZE) - (int)floorf(g_player_position[0] - PLAYER_SIZE);
        char change_z_negative = (int)floorf(g_player_position[2] + movement[2] - PLAYER_SIZE) - (int)floorf(g_player_position[2] - PLAYER_SIZE);
        char change_x = (change_x_positive > 0) ? change_x_positive : (change_x_negative < 0) ? change_x_negative : 0;
        char change_z = (change_z_positive > 0) ? change_z_positive : (change_z_negative < 0) ? change_z_negative : 0;

        int y_1 = (int)floorf(g_player_position[1] + 0.f);
        int y_2 = (int)floorf(g_player_position[1] + 1.0f);
        int y_3 = (int)floorf(g_player_position[1] + PLAYER_HEIGHT);

        if(change_y < 0)
        {
            if(
                (get_block_info(get_block_id_at(floorf(g_player_position[0] - PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at(floorf(g_player_position[0] + PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at(floorf(g_player_position[0] + PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at(floorf(g_player_position[0] - PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[1] = (float)(y_1) - g_player_position[1];
                velocity[1] = 0.0f;
                on_ground = 1;
            } else on_ground = 0;       
        } else
        {
            on_ground = 0;
            if(movement[1] > 0.f)
            {
                if(
                    (get_block_info(get_block_id_at(floorf(g_player_position[0]), floorf(g_player_position[1] + movement[1] + PLAYER_HEIGHT), floorf(g_player_position[2])))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
                {
                    movement[1] = (float)(y_3 + 1) - (g_player_position[1] + PLAYER_HEIGHT);
                    velocity[1] = 0.0f;
                }
            }
        }

        int y_0 = (int)floorf(g_player_position[1] + movement[1]);

        if(change_x && change_z)
        {
            if(
                (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_0, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_1, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_2, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_3, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                if(fabsf(movement[0]) > fabsf(movement[2]))
                {
                    if(
                        !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_0, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_1, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_2, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_3, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    )
                    {
                        movement[2] = 0.f;   
                    }
                }
                else if(fabsf(movement[2]) > fabsf(movement[0]))
                {
                    if(
                       !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_0, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_1, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_2, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(get_block_info(get_block_id_at((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_3, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
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

        change_x_positive = (int)floorf(g_player_position[0] + movement[0] + PLAYER_SIZE) - (int)floorf(g_player_position[0] + PLAYER_SIZE);
        change_z_positive = (int)floorf(g_player_position[2] + movement[2] + PLAYER_SIZE) - (int)floorf(g_player_position[2] + PLAYER_SIZE);
        change_x_negative = (int)floorf(g_player_position[0] + movement[0] - PLAYER_SIZE) - (int)floorf(g_player_position[0] - PLAYER_SIZE);
        change_z_negative = (int)floorf(g_player_position[2] + movement[2] - PLAYER_SIZE) - (int)floorf(g_player_position[2] - PLAYER_SIZE);
        change_x = (change_x_positive > 0) ? change_x_positive : (change_x_negative < 0) ? change_x_negative : 0;
        change_z = (change_z_positive > 0) ? change_z_positive : (change_z_negative < 0) ? change_z_negative : 0;

        if(change_x)
        {
            if(
                (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_0, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_1, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_2, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_3, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_0, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_1, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_2, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0]) + change_x, y_3, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[0] = 0.f;
            }
        }
        if(change_z)
        {
            if(
                (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + PLAYER_SIZE), y_0, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + PLAYER_SIZE), y_1, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + PLAYER_SIZE), y_2, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] + PLAYER_SIZE), y_3, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] - PLAYER_SIZE), y_0, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] - PLAYER_SIZE), y_1, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] - PLAYER_SIZE), y_2, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (get_block_info(get_block_id_at((int)floorf(g_player_position[0] - PLAYER_SIZE), y_3, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                movement[2] = 0.f;
            }
        }
    }
    
    glm_vec3_add(g_player_position, movement, g_player_position);

    char looking_at_block = 0;
    int look_block_pos[3];

    block_selected += get_mouse_wheel_direction();
    block_selected = mod(block_selected - 1, BLOCKID_COUNT - 1) + 1;

    // Block Break Raycast
    if(1){
        float ray_inc = 0.05f;
        float max_distance = 10.0f;

        vec3 origin;
        vec3 direction = {0.0f, 0.0f, -1.0f};

        glm_vec3_copy(g_player_position, origin);
        glm_vec3_add(origin, g_camera_offset, origin);
        glm_vec3_rotate(direction, -g_camera_rotation[0], (vec3){1.0f, 0.0f, 0.0f});
        glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.0f, 1.0f, 0.0f});
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

                char found_block = !(get_block_info(get_block_id_at(block_pos[0], block_pos[1], block_pos[2]))->flags & BLOCKFLAG_NO_COLLISION);

                if(found_block) {
                    looking_at_block = 1;
                    memcpy(look_block_pos, block_pos, sizeof(block_pos));

                    if(!paused && is_mouse_button_just_pressed(SDL_BUTTON_LEFT))
                    {
                        set_block_id_at(block_pos[0], block_pos[1], block_pos[2], 0);
                    }

                    break;
                }
            }
        }
    }

    // Raycast Place BLock
    if(!paused && is_mouse_button_just_pressed(SDL_BUTTON_RIGHT))
    {
        float max_distance = 10.0f;

        vec3 origin;
        vec3 direction = {0.0f, 0.0f, -1.0f};
        vec3 ray_inc = {0.05f, 0.05f, 0.05f};

        glm_vec3_copy(g_player_position, origin);
        glm_vec3_add(origin, g_camera_offset, origin);
        glm_vec3_rotate(direction, -g_camera_rotation[0], (vec3){1.0f, 0.0f, 0.0f});
        glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.0f, 1.0f, 0.0f});
        glm_normalize(direction);
        glm_vec3_mul(ray_inc, direction, ray_inc);

        {
            vec3 target = {0.0f, 0.0f, 0.0f};
            while((target[0]*target[0] + target[1]*target[1] + target[2]*target[2]) < max_distance*max_distance)
            {
                glm_vec3_add(target, ray_inc, target);

                vec3 position;
                glm_vec3_add(target, origin, position);

                char found_block = !(get_block_info(get_block_id_at(floorf(position[0]), floorf(position[1]), floorf(position[2])))->flags & BLOCKFLAG_NO_COLLISION);

                if(found_block)
                {
                    glm_vec3_sub(position, ray_inc, position);

                    int block_pos[3] = { floorf(position[0]), floorf(position[1]), floorf(position[2]) };

                    {
                        if( ((int)floorf(g_player_position[0]) == block_pos[0] && (int)floorf(g_player_position[1]) == block_pos[1] && (int)floorf(g_player_position[2]) == block_pos[2])
                         || ((int)floorf(g_player_position[0]) == block_pos[0] && (int)floorf(g_player_position[1] + 1.0f) == block_pos[1] && (int)floorf(g_player_position[2]) == block_pos[2])
                         || ((int)floorf(g_player_position[0]) == block_pos[0] && (int)floorf(g_player_position[1] + PLAYER_HEIGHT) == block_pos[1] && (int)floorf(g_player_position[2]) == block_pos[2]) )
                        {
                            break;
                        }
                    }

                    set_block_id_at(floorf(position[0]), floorf(position[1]), floorf(position[2]), block_selected);

                    break;
                }
            }
        }
    }

    if(is_mouse_button_just_pressed(SDL_BUTTON_MIDDLE))
    {
        block_selected = get_block_id_at(look_block_pos[0], look_block_pos[1], look_block_pos[2]);
    }

    render_start_postprocess();
    render_begin();
    {
        if(looking_at_block) render_look_block(look_block_pos[0], look_block_pos[1], look_block_pos[2]);
        render_chunks();
        render_end_postprocess();
        glUseProgram(postprocess_shader);
        int u_bInWater = glGetUniformLocation(postprocess_shader, "u_bInWater");
        glUniform1i (u_bInWater, (get_block_id_at(floorf(g_player_position[0]), floorf(g_player_position[1] + g_camera_offset[1]), floorf(g_player_position[2])) == BLOCKID_WATER) );
        int u_ScreenHeight = glGetUniformLocation(postprocess_shader, "u_ScreenHeight");
        glUniform1i(u_ScreenHeight, g_height);
        int u_ScreenWidth = glGetUniformLocation(postprocess_shader, "u_ScreenWidth");
        glUniform1i(u_ScreenWidth, g_width);
        int u_SkyColor = glGetUniformLocation(postprocess_shader, "u_SkyColor");
        glUniform3fv(u_SkyColor, 1, sky_color);
        do_postprocess(postprocess_shader, 0, 1);
        glDisable(GL_DEPTH_TEST);

        if(1) {
            frames++;
            if(time_passed >= 1.0f)
            {
                fps = frames;
                frames = 0;
                time_passed -= 1.0f;
            }

            char buf[256];
            sprintf(buf, "FPS: %d", fps);
            render_text(buf, 20, 20, (float[4]){1.0f, 1.0f, 1.0f, 1.0f}, font_arial);
        }
        {
            char buf[256];
            sprintf(buf, "x: %d y: %d z: %d", (int)floorf(g_player_position[0]), (int)floorf(g_player_position[1]), (int)floorf(g_player_position[2]));
            render_text(buf, 20, 20 + font_arial->size, (float[4]){1.0f, 1.0f, 1.0f, 1.0f}, font_arial);
        }
        {
            char buf[256];
            sprintf(buf, "block in hand: %s", get_block_info(block_selected)->name);
            render_text(buf, 20, 20 + font_arial->size * 2, (float[4]){1.0f, 1.0f, 1.0f, 1.0f}, font_arial);
        }
        glEnable(GL_DEPTH_TEST);

        input_render_end();
    }
    render_end();
}

int main(int argc, char* argv[])
{
    srand(time(0));

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    g_window = SDL_CreateWindow("Asphalt",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if(g_window == NULL)
    {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_CreateContext(g_window);

    if(!gladLoadGL()) {
        printf("No OpenGL context!\n");
        exit(-1);
    }

    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    if (GLVersion.major < 2) {
        printf("Your system doesn't support OpenGL >= 2!\n");
        return -1;
    }

    printf("OpenGL %s, GLSL %s\n%s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_RENDERER));

    setup();

    char running = true;
    SDL_Event event;
    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                if(!event.key.repeat)
                    keyboard_func(event.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                if(!event.key.repeat)
                    keyboard_func(event.key.keysym.sym, 0);
                break;
            case SDL_MOUSEMOTION:
                mouse_motion_func(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouse_button_func(event.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                mouse_button_func(event.button.button, 0);
                break;
            case SDL_MOUSEWHEEL:
                mouse_wheel_func(event.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event = SDL_WINDOWEVENT_RESIZED)
                {
                    int x, y;
                    SDL_GetWindowSize(g_window, &x, &y);
                    Resize(x, y);
                }
                break;
            }
        }

        Render();

    }

    SDL_DestroyWindow(g_window);

    SDL_Quit();

    return 0;
}
