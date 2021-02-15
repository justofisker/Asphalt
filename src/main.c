#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
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
#define PLAYER_SIZE 0.2f
#define PLAYER_HEIGHT 1.65f
vec3 sky_color = {0.4f, 0.5f, 1.0f};

static Font *font_arial;

Uint64 last_frame = 0;

static void OnResizeGameWindow(int w, int h)
{
    g_width = w > 1 ? w : 1;
    g_height = h > 1 ? h : 1;
    glViewport(0, 0, g_width, g_height);
    
    PostProcess_ResizeBuffer();

    glClearDepth(1.0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

static void SetupGame()
{
    Render_Setup();
    last_frame = SDL_GetPerformanceCounter();
    Block_Setup();
    Input_Setup();
    PostProcess_Setup();
    Chunk_SetupGenerationThread();
    Render_SetupLookBlock();
    Text_Setup();
    Text_BeginCreateFont();
    font_arial = Text_CreateFont("res/fonts/Arial.ttf", 24);
    Text_EndCreateFont();

    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    OnResizeGameWindow(w, h);
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

char looking_at_block = 0;
int look_block_pos[3];

static void HandleCollision(vec3 *pos_delta)
{
        // Collison Detection
    if(1){
        char change_y = (int)floorf(g_player_position[1] + (*pos_delta)[1]) - (int)floorf(g_player_position[1]);

        char change_x_positive = (int)floorf(g_player_position[0] + (*pos_delta)[0] + PLAYER_SIZE) - (int)floorf(g_player_position[0] + PLAYER_SIZE);
        char change_z_positive = (int)floorf(g_player_position[2] + (*pos_delta)[2] + PLAYER_SIZE) - (int)floorf(g_player_position[2] + PLAYER_SIZE);
        char change_x_negative = (int)floorf(g_player_position[0] + (*pos_delta)[0] - PLAYER_SIZE) - (int)floorf(g_player_position[0] - PLAYER_SIZE);
        char change_z_negative = (int)floorf(g_player_position[2] + (*pos_delta)[2] - PLAYER_SIZE) - (int)floorf(g_player_position[2] - PLAYER_SIZE);
        char change_x = (change_x_positive > 0) ? change_x_positive : (change_x_negative < 0) ? change_x_negative : 0;
        char change_z = (change_z_positive > 0) ? change_z_positive : (change_z_negative < 0) ? change_z_negative : 0;

        int y_1 = (int)floorf(g_player_position[1] + 0.f);
        int y_2 = (int)floorf(g_player_position[1] + 1.0f);
        int y_3 = (int)floorf(g_player_position[1] + PLAYER_HEIGHT);

        if(change_y < 0)
        {
            if(
                (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0] - PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0] + PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0] + PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0] - PLAYER_SIZE), y_1 + change_y, floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                (*pos_delta)[1] = (float)(y_1) - g_player_position[1];
                velocity[1] = 0.0f;
                on_ground = 1;
            } else on_ground = 0;       
        } else
        {
            on_ground = 0;
            if((*pos_delta)[1] > 0.f)
            {
                if(
                    (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0]), floorf(g_player_position[1] + (*pos_delta)[1] + PLAYER_HEIGHT), floorf(g_player_position[2])))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
                {
                    (*pos_delta)[1] = (float)(y_3 + 1) - (g_player_position[1] + PLAYER_HEIGHT);
                    velocity[1] = 0.0f;
                }
            }
        }

        int y_0 = (int)floorf(g_player_position[1] + (*pos_delta)[1]);

        if(change_x && change_z)
        {
            if(
                (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_0, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_1, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_2, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_3, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                if(fabsf((*pos_delta)[0]) > fabsf((*pos_delta)[2]))
                {
                    if(
                        !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_0, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_1, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_2, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                     && !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_x, y_3, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE))))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    )
                    {
                        (*pos_delta)[2] = 0.f;   
                    }
                }
                else if(fabsf((*pos_delta)[2]) > fabsf((*pos_delta)[0]))
                {
                    if(
                       !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_0, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_1, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_2, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    && !(Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + (change_x > 0 ? PLAYER_SIZE : -PLAYER_SIZE)), y_3, (int)floorf(g_player_position[2] + (change_z > 0 ? PLAYER_SIZE : -PLAYER_SIZE)) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                    )
                    {
                        (*pos_delta)[0] = 0.f;  
                    }
                }
                else
                {
                    (*pos_delta)[0] = 0.f;
                    (*pos_delta)[2] = 0.f;
                }
            }
        }

        change_x_positive = (int)floorf(g_player_position[0] + (*pos_delta)[0] + PLAYER_SIZE) - (int)floorf(g_player_position[0] + PLAYER_SIZE);
        change_z_positive = (int)floorf(g_player_position[2] + (*pos_delta)[2] + PLAYER_SIZE) - (int)floorf(g_player_position[2] + PLAYER_SIZE);
        change_x_negative = (int)floorf(g_player_position[0] + (*pos_delta)[0] - PLAYER_SIZE) - (int)floorf(g_player_position[0] - PLAYER_SIZE);
        change_z_negative = (int)floorf(g_player_position[2] + (*pos_delta)[2] - PLAYER_SIZE) - (int)floorf(g_player_position[2] - PLAYER_SIZE);
        change_x = (change_x_positive > 0) ? change_x_positive : (change_x_negative < 0) ? change_x_negative : 0;
        change_z = (change_z_positive > 0) ? change_z_positive : (change_z_negative < 0) ? change_z_negative : 0;

        if(change_x)
        {
            if(
                (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_0, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_1, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_2, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_3, (int)floorf(g_player_position[2] + PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_0, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_1, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_2, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0]) + change_x, y_3, (int)floorf(g_player_position[2] - PLAYER_SIZE)))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                (*pos_delta)[0] = 0.f;
            }
        }
        if(change_z)
        {
            if(
                (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + PLAYER_SIZE), y_0, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + PLAYER_SIZE), y_1, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + PLAYER_SIZE), y_2, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] + PLAYER_SIZE), y_3, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] - PLAYER_SIZE), y_0, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] - PLAYER_SIZE), y_1, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] - PLAYER_SIZE), y_2, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
             || (Block_GetBlockInfo(Chunk_GetBlockIdAt((int)floorf(g_player_position[0] - PLAYER_SIZE), y_3, (int)floorf(g_player_position[2]) + change_z))->flags & BLOCKFLAG_NO_COLLISION) == 0
                )
            {
                (*pos_delta)[2] = 0.f;
            }
        }
    }
}

static void HandleMovement(float delta)
{
    vec3 pos_delta = GLM_VEC3_ZERO_INIT;

    time_since_space += delta;
    if(Input_IsKeyJustPressed(SDLK_SPACE) && !paused)
    {
        if(time_since_space < 0.25)
            fly_mode = !fly_mode;
        else
            time_since_space = 0.f;
    }
    time_since_forward += delta;
    if(Input_IsKeyJustPressed(SDLK_w) && !paused)
    {
        if(time_since_forward < 0.25)
            sprint_mode = 1;
        else
            time_since_forward = 0.f;
    }
    if(Input_IsKeyJustPressed(SDLK_r) && !paused)
    {
        sprint_mode = 1;
    }
    if(!Input_IsKeyPressedd(SDLK_w) || paused)
    {
        sprint_mode = 0;
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
        char in_fluid = (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0]),floorf(g_player_position[1]),floorf(g_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT)
                     || (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0]),floorf(g_player_position[1] + 1.0f),floorf(g_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT)
                     || (Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(g_player_position[0]),floorf(g_player_position[1] + PLAYER_HEIGHT),floorf(g_player_position[2])))->flags & BLOCKFLAG_FLUID_MOVEMENT);
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
                if(Input_IsKeyPressedd(SDLK_a))
                    direction[0] -= 1.0f;
                if(Input_IsKeyPressedd(SDLK_d))
                    direction[0] += 1.0f;
                if(Input_IsKeyPressedd(SDLK_w))
                    direction[2] -= 1.0f;
                if(Input_IsKeyPressedd(SDLK_s))
                    direction[2] += 1.0f;
                if(Input_IsKeyPressedd(SDLK_SPACE))
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
                if(Input_IsKeyPressedd(SDLK_a))
                    direction[0] -= 1.0f;
                if(Input_IsKeyPressedd(SDLK_d))
                    direction[0] += 1.0f;
                if(Input_IsKeyPressedd(SDLK_w))
                    direction[2] -= 1.0f;
                if(Input_IsKeyPressedd(SDLK_s))
                    direction[2] += 1.0f;
                if(Input_IsKeyPressedd(SDLK_SPACE) && on_ground)
                    velocity[1] = 8.f;
            }
        }
        if(sprint_mode)
            speed *= 1.65f;
        glm_normalize(direction);
        glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
        glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
        glm_vec3_add(pos_delta, direction, pos_delta);
    }
        else
        {
            float speed = 150.f;
            vec3 direction = {0, 0, 0};
            // Fly
            if(!paused) {
                if(Input_IsKeyPressedd(SDLK_a))
                    direction[0] -= 1.0f;
                if(Input_IsKeyPressedd(SDLK_d))
                    direction[0] += 1.0f;
                if(Input_IsKeyPressedd(SDLK_w))
                    direction[2] -= 1.0f;
                if(Input_IsKeyPressedd(SDLK_s))
                    direction[2] += 1.0f;
                if(Input_IsKeyPressedd(SDLK_SPACE))
                    velocity[1] = 10.0f;
                if(SDL_GetModState() & KMOD_SHIFT)
                    velocity[1] = -10.0f;
            }
            if(sprint_mode)
                speed *= 1.65f;
            glm_normalize(direction);
            glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.f, 1.f, 0.f});
            glm_vec3_mul(direction, (vec3){delta * speed, delta * speed, delta * speed}, direction);
            glm_vec3_add(pos_delta, direction, pos_delta);
        }
    
        vec3 velocity_movement;
        glm_vec3_copy(velocity, velocity_movement);
        glm_vec3_mul(velocity_movement, (vec3){delta, delta, delta}, velocity_movement);
        glm_vec3_add(pos_delta, velocity_movement, pos_delta);
    }

    HandleCollision(&pos_delta);

    glm_vec3_add(g_player_position, pos_delta, g_player_position);
}

static void HandleRaycastBlocks(char *looking_at_block, int *look_block_pos)
{

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

                char found_block = !(Block_GetBlockInfo(Chunk_GetBlockIdAt(block_pos[0], block_pos[1], block_pos[2]))->flags & BLOCKFLAG_NO_COLLISION);

                if(found_block) {
                    *looking_at_block = 1;
                    memcpy(look_block_pos, block_pos, sizeof(block_pos));

                    if(!paused && Input_IsMouseButtonJustPressedd(SDL_BUTTON_LEFT))
                    {
                        Chunk_SetBlockIdAt(block_pos[0], block_pos[1], block_pos[2], 0);
                    }

                    break;
                }
            }
        }
    }

    // Raycast Place BLock
    if(!paused && Input_IsMouseButtonJustPressedd(SDL_BUTTON_RIGHT))
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

                char found_block = !(Block_GetBlockInfo(Chunk_GetBlockIdAt(floorf(position[0]), floorf(position[1]), floorf(position[2])))->flags & BLOCKFLAG_NO_COLLISION);

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

                    Chunk_SetBlockIdAt(floorf(position[0]), floorf(position[1]), floorf(position[2]), block_selected);

                    break;
                }
            }
        }
    }

}

static void Render()
{
    PostProcess_CaptureBuffer();
    Render_Start();
    {
        if(looking_at_block) Render_RenderLookBlock(look_block_pos[0], look_block_pos[1], look_block_pos[2]);
        Render_RenderChunks();
        PostProcess_ReleaseBuffer();
        glUseProgram(g_postprocess_shader);
        int u_bInWater = glGetUniformLocation(g_postprocess_shader, "u_bInWater");
        glUniform1i (u_bInWater, (Chunk_GetBlockIdAt(floorf(g_player_position[0]), floorf(g_player_position[1] + g_camera_offset[1]), floorf(g_player_position[2])) == BLOCKID_WATER) );
        int u_ScreenHeight = glGetUniformLocation(g_postprocess_shader, "u_ScreenHeight");
        glUniform1i(u_ScreenHeight, g_height);
        int u_ScreenWidth = glGetUniformLocation(g_postprocess_shader, "u_ScreenWidth");
        glUniform1i(u_ScreenWidth, g_width);
        int u_SkyColor = glGetUniformLocation(g_postprocess_shader, "u_SkyColor");
        glUniform3fv(u_SkyColor, 1, sky_color);
        PostProcess_RenderToBuffer(g_postprocess_shader, 0, 1);
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
            Text_RenderText(buf, 20, 20, (float[4]){1.0f, 1.0f, 1.0f, 1.0f}, font_arial);
        }
        {
            char buf[256];
            sprintf(buf, "x: %d y: %d z: %d", (int)floorf(g_player_position[0]), (int)floorf(g_player_position[1]), (int)floorf(g_player_position[2]));
            Text_RenderText(buf, 20, 20 + font_arial->size, (float[4]){1.0f, 1.0f, 1.0f, 1.0f}, font_arial);
        }
        {
            char buf[256];
            sprintf(buf, "block in hand: %s", Block_GetBlockInfo(block_selected)->name);
            Text_RenderText(buf, 20, 20 + font_arial->size * 2, (float[4]){1.0f, 1.0f, 1.0f, 1.0f}, font_arial);
        }
        glEnable(GL_DEPTH_TEST);

        Input_RenderEnd();
    }
    Render_End();
}

int main(int argc, char *argv[])
{
    srand(time(0));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    g_window = SDL_CreateWindow("Asphalt",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                1280, 720,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (g_window == NULL)
    {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_CreateContext(g_window);

    if (!gladLoadGL())
    {
        printf("No OpenGL context!\n");
        exit(-1);
    }

    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    if (GLVersion.major < 2)
    {
        printf("Your system doesn't support OpenGL >= 2!\n");
        return -1;
    }

    printf("OpenGL %s, GLSL %s\n%s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_RENDERER));

    SetupGame();

    char running = true;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                if (!event.key.repeat)
                    Input_HandleKeyboard(event.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                if (!event.key.repeat)
                    Input_HandleKeyboard(event.key.keysym.sym, 0);
                break;
            case SDL_MOUSEMOTION:
                Input_HandleMouseMotion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                Input_HandleMouseButton(event.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                Input_HandleMouseButton(event.button.button, 0);
                break;
            case SDL_MOUSEWHEEL:
                Input_HandleMouseWheel(event.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    OnResizeGameWindow(event.window.data1, event.window.data2);
                }
                break;
            }
        }

        {
            Uint64 now = SDL_GetPerformanceCounter();
            float delta = (float)(now - last_frame) / SDL_GetPerformanceFrequency();
            last_frame = now;
            if (delta >= 0.1f)
                delta = 0.1f;
            time_passed += delta;

            if (Input_IsKeyJustPressed(SDLK_ESCAPE))
            {
                paused = !paused;
                if (paused)
                    Input_SetMouseMode(MOUSEMODE_CURSOR);
                else
                    Input_SetMouseMode(MOUSEMODE_CAPTURED);
            }

            if (!paused)
            {
                int motion_x, motion_y;
                Input_GetMouseMotion(&motion_x, &motion_y);
                g_camera_rotation[0] += glm_rad((float)motion_y / 10);
                g_camera_rotation[0] = fmaxf(fminf(glm_rad(90.f), g_camera_rotation[0]), glm_rad(-90.f));
                g_camera_rotation[1] += glm_rad((float)motion_x / 10);
                g_camera_rotation[1] = fmodf(g_camera_rotation[1], GLM_PIf * 2);
            }

            if (Input_IsKeyJustPressed(SDLK_f))
            {
                fullscreen = !fullscreen;
                if (fullscreen)
                {
                    SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                else
                {
                    SDL_SetWindowFullscreen(g_window, 0);
                }
            }

            HandleMovement(delta);

            looking_at_block = 0;

            block_selected += Input_GetMouseWheelDirection();
            block_selected = mod(block_selected - 1, BLOCKID_COUNT - 1) + 1;

            HandleRaycastBlocks(&looking_at_block, look_block_pos);

            if (Input_IsMouseButtonJustPressedd(SDL_BUTTON_MIDDLE))
            {
                block_selected = Chunk_GetBlockIdAt(look_block_pos[0], look_block_pos[1], look_block_pos[2]);
            }
        }

        Render();
    }

    SDL_DestroyWindow(g_window);

    SDL_Quit();

    return 0;
}
