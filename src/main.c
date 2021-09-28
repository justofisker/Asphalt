#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <SDL.h>
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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#if _MSC_VER && !__INTEL_COMPILER
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif // MSVC

static char paused = 0;
#define PLAYER_SIZE 0.2f
#define PLAYER_HEIGHT 1.65f
Uint64 last_frame = 0;

static void OnResizeGameWindow(int w, int h)
{
    g_width = w > 1 ? w : 1;
    g_height = h > 1 ? h : 1;
    glViewport(0, 0, g_width, g_height);
    
    PostProcess_ResizeBuffer();

    //glClearDepth(1.0);
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
    g_font_arial = Text_CreateFont("res/fonts/Arial.ttf", 24);
    Text_EndCreateFont();

    int w, h;
    SDL_GL_GetDrawableSize(g_window, &w, &h);
    OnResizeGameWindow(w, h);

    // Populate 5x5 chunk around play
    int x, y;
    for (x = -2; x <= 2; x++)
    {
        for(y = -2; y <= 2; y++)
        {
            Chunk *chunk = Chunk_CreateChunk(x, y);
            Chunk_SetChunkArraySlot(x, y, chunk);
            Chunk_PopulateChunkMeshBuffers(chunk);
            chunk->create_mesh = 1;
        }
    }
    
    int z;
    x = rand() % (16 * 5) - 2 * 16;
    z = rand() % (16 * 5) - 2 * 16;
    g_player_position[0] = x + 0.5f;
    g_player_position[2] = z + 0.5f;

    for(y = CHUNK_SIZE_Y; y >= 0; y--)
    {
        if(Chunk_GetBlockIdAt(x, y, z))
        {
            g_player_position[1] = y + 1;
            break;
        }
    }
}

float time_passed = 0.0f;
int frames = 0;
float time_since_space = 1.f;
float time_since_forward = 1.f;
char sprint_mode = 0;
char fly_mode = 0;

vec3 velocity = GLM_VEC3_ZERO_INIT;
char on_ground = 0;
char fullscreen = 0;

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
    vec3 origin;
    vec3 direction = {0.0f, 0.0f, -1.0f};

    glm_vec3_copy(g_player_position, origin);
    glm_vec3_add(origin, g_camera_offset, origin);
    glm_vec3_rotate(direction, -g_camera_rotation[0], (vec3){1.0f, 0.0f, 0.0f});
    glm_vec3_rotate(direction, -g_camera_rotation[1], (vec3){0.0f, 1.0f, 0.0f});

    char hit_flags;
    ivec3 block_pos;
    Util_RaycastToBlock(origin, direction, 10.0f, &hit_flags, &block_pos);

    if(!paused && hit_flags) {
        // Block Break Raycast
        if(Input_IsMouseButtonJustPressedd(SDL_BUTTON_LEFT)) {
            Chunk_SetBlockIdAt(block_pos[0], block_pos[1], block_pos[2], 0);

            Util_RaycastToBlock(origin, direction, 10.0f, &hit_flags, &block_pos);
        }

        // Raycast Place BLock
        if(Input_IsMouseButtonJustPressedd(SDL_BUTTON_RIGHT))
        {
            ivec3 place_position;
            place_position[0] = block_pos[0];
            place_position[1] = block_pos[1];
            place_position[2] = block_pos[2];

            switch(hit_flags)
            {
            case RAYCAST_HITFLAG_FACE_NORTH:
                place_position[2]++;
                break;
            case RAYCAST_HITFLAG_FACE_EAST:
                place_position[0]++;
                break;
            case RAYCAST_HITFLAG_FACE_SOUTH:
                place_position[2]--;
                break;
            case RAYCAST_HITFLAG_FACE_WEST:
                place_position[0]--;
                break;
            case RAYCAST_HITFLAG_FACE_TOP:
                place_position[1]++;
                break;
            case RAYCAST_HITFLAG_FACE_BOTTOM:
                place_position[1]--;
                break;
            }

            Chunk_SetBlockIdAt(place_position[0], place_position[1], place_position[2], g_block_selected);

            Util_RaycastToBlock(origin, direction, 10.0f, &hit_flags, &block_pos);
        }
    }

    if(hit_flags) {
        *looking_at_block = 1;
        look_block_pos[0] = block_pos[0];
        look_block_pos[1] = block_pos[1];
        look_block_pos[2] = block_pos[2];
    }
}

static char running = true;
static SDL_Event event;

void gameLoop() {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (!event.key.repeat)
                    Input_HandleKeyboard(event.key.keysym.sym, event.key.state);
                break;
            case SDL_MOUSEMOTION:
                Input_HandleMouseMotion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                Input_HandleMouseButton(event.button.button, event.button.state);
                break;
            case SDL_MOUSEWHEEL:
                Input_HandleMouseWheel(event.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int w, h;
                    SDL_GL_GetDrawableSize(g_window, &w, &h);
                    OnResizeGameWindow(w, h);
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
            frames++;
            if(time_passed >= 1.0f)
            {
                g_fps = frames;
                frames = 0;
                time_passed -= 1.0f;
            }

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
                    // Resize Window Event not called on exiting fullscreen
                    int w, h;
                    SDL_GL_GetDrawableSize(g_window, &w, &h);
                    OnResizeGameWindow(w, h);
                }
            }

            HandleMovement(delta);

            g_looking_at_block = 0;

            g_block_selected += Input_GetMouseWheelDirection();
            g_block_selected = mod(g_block_selected - 1, BLOCKID_COUNT - 1) + 1;

            HandleRaycastBlocks(&g_looking_at_block, g_look_block_pos);

            if (Input_IsMouseButtonJustPressedd(SDL_BUTTON_MIDDLE))
            {
                g_block_selected = Chunk_GetBlockIdAt(g_look_block_pos[0], g_look_block_pos[1], g_look_block_pos[2]);
            }
        }

        Render_RenderWorld();
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
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (g_window == NULL)
    {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GLContext context = SDL_GL_CreateContext(g_window);
    
    if (!context) {
        SDL_Log("Failed to create OpenGL context!\n");
        exit(0);
    }

    if (!gladLoadGLES2Loader(SDL_GL_GetProcAddress))
    {
        SDL_Log("Failed to Load glad!\n");
        exit(0);
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

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(gameLoop, 0, true);
#else
    while(running) { gameLoop(); }
#endif

    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
