#define CHUNK_DEBUG
#include "Chunk.h"

#include "Mesh.h"
#include "Globals.h"
#include "Util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cglm/cglm.h>
#include <cglm/plane.h>
#include <glad/glad.h>
#include <SDL.h>
#include "Block.h"
#include <time.h>
#include <math.h>
#include "Render.h"

typedef struct _Vertex
{
    vec3 position;
    vec2 uv;
    vec3 normal;
} Vertex;

typedef unsigned int Index;

#define TEX_CELL_COUNT_XY 8
#define TEX_CELL_SIZE_XY (1.0f / (float)TEX_CELL_COUNT_XY)

#define CHUNK_ARR_SIZE CHUNK_VIEW_DISTANCE * 2 + 2
Chunk *chunks[CHUNK_ARR_SIZE][CHUNK_ARR_SIZE];
Chunk *chunks_to_free[CHUNK_ARR_SIZE][CHUNK_ARR_SIZE];
int last_x = 0;
int last_y = 0;
static char dontGenerate = 0;

void Chunk_SetChunkArraySlot(int x, int y, Chunk* chunk)
{
    Chunk *previous = chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)];
    chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)] = chunk;
    if(previous)
        chunks_to_free[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)] = previous;
}

#define WAIT_FOR_UNLOCK(chunk) while(chunk->locked) { SDL_Delay(0); }

static inline void Chunk_FillSuroundingChunk(int x, int y)
{
    Chunk* chunk = Chunk_GetChunk(x, y); 
    if(chunk && !chunk->mesh 
        && (Chunk_GetChunk(x - 1, y    )) 
        && (Chunk_GetChunk(x + 1, y    )) 
        && (Chunk_GetChunk(x,     y - 1)) 
        && (Chunk_GetChunk(x,     y + 1))) 
    { 
        Chunk_PopulateChunkMeshBuffers(chunk); 
        chunk->create_mesh = 1; 
    }
}

static void Chunk_PushChunkMeshBuffers(Chunk *chunk)
{
    if(chunk->mesh) Mesh_FreeMesh(chunk->mesh);
    if(chunk->transparent_mesh) Mesh_FreeMesh(chunk->transparent_mesh);
    if (chunk->solid_vertex_buffer == 0
        || chunk->solid_index_buffer == 0
        || chunk->transparent_vertex_buffer == 0
        || chunk->transparent_index_buffer == 0)
        return;
    chunk->mesh = malloc(sizeof(Mesh));
    chunk->transparent_mesh = malloc(sizeof(Mesh));

    glGenVertexArrays(1, &chunk->mesh->array_object);
    glBindVertexArray(chunk->mesh->array_object);

    glGenBuffers(1, &chunk->mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh->vertex_buffer); 

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * chunk->solid_vertex_count, chunk->solid_vertex_buffer, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glGenBuffers(1, &chunk->mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * chunk->solid_index_count, chunk->solid_index_buffer, GL_STATIC_DRAW);

    glBindVertexArray(0);

    chunk->mesh->index_count = chunk->solid_index_count;
    chunk->mesh->index_type = GL_UNSIGNED_INT;

    glGenVertexArrays(1, &chunk->transparent_mesh->array_object);
    glBindVertexArray(chunk->transparent_mesh->array_object);

    glGenBuffers(1, &chunk->transparent_mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->transparent_mesh->vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * chunk->transparent_vertex_count, chunk->transparent_vertex_buffer, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glGenBuffers(1, &chunk->transparent_mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->transparent_mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * chunk->transparent_index_count, chunk->transparent_index_buffer, GL_STATIC_DRAW);

    glBindVertexArray(0);

    chunk->transparent_mesh->index_count = chunk->transparent_index_count;
    chunk->transparent_mesh->index_type = GL_UNSIGNED_INT;

    free(chunk->solid_vertex_buffer);
    free(chunk->solid_index_buffer);
    free(chunk->transparent_vertex_buffer);
    free(chunk->transparent_index_buffer);
    chunk->solid_vertex_buffer = 0;
    chunk->solid_index_buffer = 0;
    chunk->transparent_vertex_buffer = 0;
    chunk->transparent_index_buffer = 0;
    chunk->create_mesh = 0;
}

SDL_Thread *generation_thread;

int Chunk_AsyncGenerateChunks(void* arg)
{
#ifndef __EMSCRIPTEN__
    srand((unsigned int)time(0));
    while(1)
#endif
    {

    int x = 0, y = 0;
    int chunks_generated = 0;
    int cur_x = (int)(g_player_position[0]) / CHUNK_SIZE_XZ;
    int cur_y = (int)(g_player_position[2]) / CHUNK_SIZE_XZ;
    spiral: if(!dontGenerate || last_x != cur_x || last_y != cur_y)
    {
        last_x = cur_x;
        last_y = cur_y;
        int x,y,dx,dy;
        x = 0;
        y = 0;
        dx = 0;
        dy = -1;
        int t = CHUNK_ARR_SIZE;
        int maxI = t*t;
        int i;
        for(i = 0; i < maxI; i++){
            int chunk_x = x + cur_x;
            int chunk_y = y + cur_y;
            if ((-CHUNK_ARR_SIZE/2 <= x) && (x <= CHUNK_ARR_SIZE/2) && (-CHUNK_ARR_SIZE/2 <= y) && (y <= CHUNK_ARR_SIZE/2)){
                if(!Chunk_GetChunk(chunk_x, chunk_y))
                {
                    Chunk *chunk = Chunk_CreateChunk(chunk_x, chunk_y);
                    if(chunks[mod(chunk_x, CHUNK_ARR_SIZE)][mod(chunk_y, CHUNK_ARR_SIZE)]) WAIT_FOR_UNLOCK(chunks[mod(chunk_x, CHUNK_ARR_SIZE)][mod(chunk_y, CHUNK_ARR_SIZE)])
                    Chunk_SetChunkArraySlot(chunk_x, chunk_y, chunk);
                    
                    Chunk_FillSuroundingChunk(chunk_x - 1, chunk_y);
                    Chunk_FillSuroundingChunk(chunk_x + 1, chunk_y);
                    Chunk_FillSuroundingChunk(chunk_x, chunk_y - 1);
                    Chunk_FillSuroundingChunk(chunk_x, chunk_y + 1);

                    if (cur_x != (int)(g_player_position[0]) / CHUNK_SIZE_XZ
                     || cur_y != (int)(g_player_position[2]) / CHUNK_SIZE_XZ)
                    { 
                        cur_x = (int)(g_player_position[0]) / CHUNK_SIZE_XZ;
                        cur_y = (int)(g_player_position[2]) / CHUNK_SIZE_XZ;
                        goto spiral;
                    }
                }
            }
            if( (x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1-y))){
                t = dx;
                dx = -dy;
                dy = t;
            }
            x += dx;
            y += dy;
        }
        
        dontGenerate = 1;
    }
    else
    {
        last_x = cur_x;
        last_y = cur_y;
    }

    SDL_Delay(50);

    }

    return 0;
}

Chunk *Chunk_CreateChunk(int _x, int _y)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    memset(chunk->blocks, 0, sizeof(chunk->blocks));

    chunk->x = _x;
    chunk->y = _y;
    chunk->mesh = 0;
    chunk->transparent_mesh = 0;
    chunk->create_mesh = 0;

    chunk->solid_vertex_buffer = 0;
    chunk->solid_index_buffer = 0;
    chunk->transparent_vertex_buffer = 0;
    chunk->transparent_index_buffer = 0;
    chunk->locked = 0;

    memset(chunk->blocks, 0, sizeof(chunk->blocks));
    int x,y, z;
    for(x = 0; x < CHUNK_SIZE_XZ; ++x)
    {
        for(z = 0; z < CHUNK_SIZE_XZ; ++z)
        {
            //           V Because we cant use negative numbers (please fix it is very hacky)
            int act_x = 1000000 + CHUNK_SIZE_XZ * _x + x;
            int act_z = 1000000 + CHUNK_SIZE_XZ * _y + z;
            int act_y = fmaxf(fminf(CHUNK_SIZE_Y, 
                (Util_Perlin2D(act_x, act_z, 0.02f, 5) * 20)
                + (Util_Perlin2D(act_x, act_z, 0.0044, 2) * 100)
            ), 1);
            chunk->blocks[x][act_y - 1][z] = (Block){ act_y > 55 ? BLOCKID_GRASS : BLOCKID_SAND, 0 };
            for(y = act_y - 2; y >= 0; y--)
                chunk->blocks[x][y][z]= (Block){  act_y - 5 < y ? BLOCKID_DIRT : BLOCKID_STONE, 0 };
            for(y = act_y; y < CHUNK_SIZE_Y; y++)
                chunk->blocks[x][y][z] = (Block){ 0, 0 };
            for(y = 1; y < 55; y++)
                if(!chunk->blocks[x][y][z].id)
                    chunk->blocks[x][y][z] = (Block){ BLOCKID_WATER, 0, };
            chunk->blocks[x][0][z] = (Block){ BLOCKID_BEDROCK, 0 };
        }
    }

    return chunk;
}

void Chunk_PopulateChunkMeshBuffers(Chunk *chunk)
{
    if(chunk->solid_vertex_buffer)
        free(chunk->solid_vertex_buffer);
    if(chunk->solid_index_buffer)
        free(chunk->solid_index_buffer);
    if(chunk->transparent_vertex_buffer)
        free(chunk->transparent_vertex_buffer);
    if(chunk->transparent_index_buffer)
        free(chunk->transparent_index_buffer);
    chunk->solid_vertex_buffer = 0;
    chunk->solid_index_buffer = 0;
    chunk->transparent_vertex_buffer = 0;
    chunk->transparent_index_buffer = 0;
    char faces_needed[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ] = { 0 };

    // Solid Mesh
    {
        chunk->aabb_solid[0][0] = CHUNK_SIZE_XZ;
        chunk->aabb_solid[0][2] = CHUNK_SIZE_XZ;
        chunk->aabb_solid[0][1] = CHUNK_SIZE_Y;
        chunk->aabb_solid[1][0] = 0;
        chunk->aabb_solid[1][1] = 0;
        chunk->aabb_solid[1][2] = 0;
        int face_count = 0;

        int x, y, z, i;
        for(x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            for(y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for(z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    if((Block_GetBlockInfo(chunk->blocks[x][y][z].id)->flags & BLOCKFLAG_TRANSPARENT) == 0)
                    {
                        if(x < chunk->aabb_solid[0][0]) chunk->aabb_solid[0][0] = x;
                        if(x + 1 > chunk->aabb_solid[1][0]) chunk->aabb_solid[1][0] = x + 1;
                        if(y < chunk->aabb_solid[0][1]) chunk->aabb_solid[0][1] = y;
                        if(y + 1 > chunk->aabb_solid[1][1]) chunk->aabb_solid[1][1] = y + 1;
                        if(z < chunk->aabb_solid[0][2]) chunk->aabb_solid[0][2] = z;
                        if(z + 1 > chunk->aabb_solid[1][2]) chunk->aabb_solid[1][2] = z + 1;


                        char north;
                        if(z + 1 == CHUNK_SIZE_XZ)
                        {
                            if(Chunk_GetChunk(chunk->x, chunk->y + 1))
                                north = Block_GetBlockInfo(Chunk_GetChunk(chunk->x, chunk->y + 1)->blocks[x][y][0].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                            else 
                                north = 1;
                        } else
                            north = Block_GetBlockInfo(chunk->blocks[x][y][z + 1].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                        
                        char south;
                        if(z - 1 == -1)
                        {
                            if(Chunk_GetChunk(chunk->x, chunk->y - 1))
                                south = Block_GetBlockInfo(Chunk_GetChunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                            else
                                south = 1;
                        } else
                            south = Block_GetBlockInfo(chunk->blocks[x][y][z - 1].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char west;
                        if(x - 1 == -1)
                        {
                            if(Chunk_GetChunk(chunk->x - 1, chunk->y))
                                west = Block_GetBlockInfo(Chunk_GetChunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                            else
                                west = 1;
                        } else
                            west = Block_GetBlockInfo(chunk->blocks[x - 1][y][z].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char east;
                        if(x + 1 == CHUNK_SIZE_XZ)
                        {
                            if(Chunk_GetChunk(chunk->x + 1, chunk->y))
                                east =  Block_GetBlockInfo(Chunk_GetChunk(chunk->x + 1, chunk->y)->blocks[0][y][z].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                            else
                                east = 1;
                        } else
                            east = Block_GetBlockInfo(chunk->blocks[x + 1][y][z].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char up;
                        if(y + 1 == CHUNK_SIZE_Y)
                            up = 1;
                        else
                            up = Block_GetBlockInfo(chunk->blocks[x][y + 1][z].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char down;
                        if(y - 1 == -1)
                            down = 1;
                        else
                            down = Block_GetBlockInfo(chunk->blocks[x][y - 1][z].id)->flags & BLOCKFLAG_TRANSPARENT != 0;
                        face_count += north + south + west + east + up + down;
                        faces_needed[x][y][z] = (north ? (1 << BLOCKSIDE_NORTH) : 0)
                                              | (east  ? (1 << BLOCKSIDE_EAST ) : 0)
                                              | (south ? (1 << BLOCKSIDE_SOUTH) : 0)
                                              | (west  ? (1 << BLOCKSIDE_WEST ) : 0)
                                              | (up    ? (1 << BLOCKSIDE_UP   ) : 0)
                                              | (down  ? (1 << BLOCKSIDE_DOWN ) : 0);
                    }
                }
            }
        }

        chunk->solid_vertex_count = face_count * 4;
        chunk->solid_index_count = face_count * 6;

        int offset = 0;
        size_t vertex_size = sizeof(Vertex) * 4 * face_count;
        Vertex *verticies = malloc(vertex_size);
        for(x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            for(y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for(z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    if((Block_GetBlockInfo(chunk->blocks[x][y][z].id)->flags & BLOCKFLAG_TRANSPARENT) == 0)
                    {
                        BlockInfo* block_info = Block_GetBlockInfo(chunk->blocks[x][y][z].id);

                        if(!block_info)
                        {
                            printf("MISSING BLOCK INFO %d\n", chunk->blocks[x][y][z].id);
                            continue;
                        }

                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_SOUTH))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_EAST))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 3] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_NORTH))
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_WEST))
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 1] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_UP))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 1] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 2] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_DOWN))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 3] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            offset += 4;
                        }
                    }
                }
            }
        }

        Index* indicies = malloc(sizeof(Index) * 6 * face_count);
        for(i = 0; i < face_count; ++i)
        {
            indicies[i * 6 + 0] = 0 + 4 * i;
            indicies[i * 6 + 1] = 1 + 4 * i;
            indicies[i * 6 + 2] = 2 + 4 * i;
            indicies[i * 6 + 3] = 2 + 4 * i;
            indicies[i * 6 + 4] = 3 + 4 * i;
            indicies[i * 6 + 5] = 0 + 4 * i;
        }

        chunk->solid_vertex_buffer = verticies;
        chunk->solid_index_buffer = indicies;
    }
    // Transparent Mesh
    {
        chunk->aabb_transparent[0][0] = CHUNK_SIZE_XZ;
        chunk->aabb_transparent[0][2] = CHUNK_SIZE_XZ;
        chunk->aabb_transparent[0][1] = CHUNK_SIZE_Y;
        chunk->aabb_transparent[1][0] = 0;
        chunk->aabb_transparent[1][1] = 0;
        chunk->aabb_transparent[1][2] = 0;
        
        int face_count = 0;

        int x, y, z, i;
        for(x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            for(y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for(z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    if(chunk->blocks[x][y][z].id && (Block_GetBlockInfo(chunk->blocks[x][y][z].id)->flags & BLOCKFLAG_TRANSPARENT) != 0)
                    {
                        if(x < chunk->aabb_transparent[0][0]) chunk->aabb_transparent[0][0] = x;
                        if(x + 1 > chunk->aabb_transparent[1][0]) chunk->aabb_transparent[1][0] = x + 1;
                        if(y < chunk->aabb_transparent[0][1]) chunk->aabb_transparent[0][1] = y;
                        if(y + 1 > chunk->aabb_transparent[1][1]) chunk->aabb_transparent[1][1] = y + 1;
                        if(z < chunk->aabb_transparent[0][2]) chunk->aabb_transparent[0][2] = z;
                        if(z + 1 > chunk->aabb_transparent[1][2]) chunk->aabb_transparent[1][2] = z + 1;

                        char north;
                        if(z + 1 == CHUNK_SIZE_XZ)
                        {
                            if(Chunk_GetChunk(chunk->x, chunk->y + 1))
                                north = Chunk_GetChunk(chunk->x, chunk->y + 1)->blocks[x][y][0].id == 0;
                            else north = 0;
                        } else north = chunk->blocks[x][y][z + 1].id == 0;
                        char south;
                        if(z - 1 == -1)
                        {
                            if(Chunk_GetChunk(chunk->x, chunk->y - 1))
                                south = Chunk_GetChunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1].id == 0;
                            else south = 0;
                        } else south = chunk->blocks[x][y][z - 1].id == 0;
                        char west;
                        if(x - 1 == -1)
                        {
                            if(Chunk_GetChunk(chunk->x - 1, chunk->y))
                                west = Chunk_GetChunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z].id == 0;
                            else west = 0;
                        } else west = chunk->blocks[x - 1][y][z].id == 0;
                        char east;
                        if(x + 1 == CHUNK_SIZE_XZ)
                        {
                            if(Chunk_GetChunk(chunk->x + 1, chunk->y))
                                east = Chunk_GetChunk(chunk->x + 1, chunk->y)->blocks[0][y][z].id                 == 0;
                            else east = 0;
                        } else east = chunk->blocks[x + 1][y][z].id == 0;;
                        char up;
                        if(y + 1 ==  CHUNK_SIZE_Y)
                            up = 0;
                        else up = chunk->blocks[x][y + 1][z].id == 0; 
                        char down;
                        if(y - 1 == -1)
                            down = 1;
                        else down = chunk->blocks[x][y - 1][z].id == 0;
                        face_count += north + south + west + east + up + down;
                        faces_needed[x][y][z] = (north ? (1 << BLOCKSIDE_NORTH) : 0)
                                              | (east  ? (1 << BLOCKSIDE_EAST ) : 0)
                                              | (south ? (1 << BLOCKSIDE_SOUTH) : 0)
                                              | (west  ? (1 << BLOCKSIDE_WEST ) : 0)
                                              | (up    ? (1 << BLOCKSIDE_UP   ) : 0)
                                              | (down  ? (1 << BLOCKSIDE_DOWN ) : 0);
                    }
                }
            }
        }

        chunk->transparent_vertex_count = face_count * 4;
        chunk->transparent_index_count = face_count * 6;

        int offset = 0;
        size_t vertex_size = sizeof(Vertex) * 4 * face_count;
        Vertex *verticies = malloc(vertex_size);
        for(x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            for(y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for(z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    if(chunk->blocks[x][y][z].id && (Block_GetBlockInfo(chunk->blocks[x][y][z].id)->flags & BLOCKFLAG_TRANSPARENT) != 0)
                    {
                        BlockInfo* block_info = Block_GetBlockInfo(chunk->blocks[x][y][z].id);
                        if(!block_info)
                        {
                            printf("MISSING BLOCK INFO %d\n", chunk->blocks[x][y][z].id);
                            continue;
                        }

                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_SOUTH))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_EAST))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 3] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_NORTH))
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_WEST))
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 1] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_UP))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.9f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.9f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.9f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 3] = (Vertex){0.0f + x, 0.9f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            offset += 4;
                        }
                        if(faces_needed[x][y][z] & (1 << BLOCKSIDE_DOWN))
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 3] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block_info->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            offset += 4;
                        }
                    }
                }
            }
        }

        Index* indicies = malloc(sizeof(Index) * 6 * face_count);
        for(i = 0; i < face_count; ++i)
        {
            indicies[i * 6 + 0] = 0 + 4 * i;
            indicies[i * 6 + 1] = 1 + 4 * i;
            indicies[i * 6 + 2] = 2 + 4 * i;
            indicies[i * 6 + 3] = 2 + 4 * i;
            indicies[i * 6 + 4] = 3 + 4 * i;
            indicies[i * 6 + 5] = 0 + 4 * i;
        }

        chunk->transparent_vertex_buffer = verticies;
        chunk->transparent_index_buffer = indicies;
    }
    
}

void Chunk_RegenerateChunkMesh(Chunk *chunk)
{
    Chunk_PopulateChunkMeshBuffers(chunk);
    Chunk_PushChunkMeshBuffers(chunk);
}

void Chunk_FreeChunk(Chunk *chunk)
{
    if(chunk->mesh) Mesh_FreeMesh(chunk->mesh);
    if(chunk->transparent_mesh) Mesh_FreeMesh(chunk->transparent_mesh);
    if(chunk->solid_vertex_buffer) free(chunk->solid_vertex_buffer);
    if(chunk->solid_index_buffer) free(chunk->solid_index_buffer);
    if(chunk->transparent_vertex_buffer) free(chunk->transparent_vertex_buffer);
    if(chunk->transparent_index_buffer) free(chunk->transparent_index_buffer);
    free(chunk);
}

void Chunk_SetupGenerationThread()
{
    memset(chunks, 0, sizeof(chunks));
    memset(chunks_to_free, 0, sizeof(chunks_to_free));
#ifdef __EMSCRIPTEN__
    //Chunk_AsyncGenerateChunks(0);
#else
    generation_thread = SDL_CreateThread(Chunk_AsyncGenerateChunks, "Chunk Generation", (void*)NULL);
#endif
}

void Render_RenderChunks()
{
    glUseProgram(g_block_shader);
    glUniformMatrix4fv(g_block_view_loc, 1, GL_FALSE, g_view[0]);
    glUniformMatrix4fv(g_block_projection_loc, 1, GL_FALSE, g_projection[0]);
    glUniform1f(g_block_view_near_loc, g_camera_info.fNear);
    glUniform1f(g_block_view_far_loc, g_camera_info.fFar);
    Texture_Bind(g_texture, 0);
    glUniform1i(g_block_texture_loc, 0);

    int x, y, stage;

    mat4 viewProj;
    glm_mat4_mul(g_projection, g_view, viewProj);
    vec4 planes[6];
    glm_frustum_planes(viewProj, planes);
    g_chunks_drawn = 0;

    for(stage = 0; stage < 5; stage++) {
        if(stage == 4) {
            glDisable(GL_CULL_FACE);
            glUseProgram(g_block_shader);
        }
        if(stage == 3) {

        }
        for(x = 0; x < CHUNK_ARR_SIZE; x++)
        {
            for(y = 0; y < CHUNK_ARR_SIZE; y++)
            {
                Chunk *chunk = chunks[x][y];
                switch(stage) {
                case 0:
                    if(chunks_to_free[x][y])
                    {
                        Chunk_FreeChunk(chunks_to_free[x][y]);
                        chunks_to_free[x][y] = 0;
                    }
                    break;
                case 1:
                    if(chunk && chunk->create_mesh && !chunk->locked)
                    {
                        chunk->locked = 1;
                        Chunk_PushChunkMeshBuffers(chunk);
                        chunks[x][y]->locked = 0;
                    }
                    break;
                case 2:
                    if(chunk) {
                        if(chunk->mesh && chunk->mesh->index_count)
                        {
                            vec3 box[2] = { {chunk->x * CHUNK_SIZE_XZ + chunk->aabb_solid[0][0] - g_player_position[0], chunk->aabb_solid[0][1] - g_player_position[1], chunk->y * CHUNK_SIZE_XZ + chunk->aabb_solid[0][2] - g_player_position[2]},
                                            {chunk->x * CHUNK_SIZE_XZ + chunk->aabb_solid[1][0] - g_player_position[0], chunk->aabb_solid[1][1] - g_player_position[1], chunk->y * CHUNK_SIZE_XZ + chunk->aabb_solid[1][2] - g_player_position[2]} };
                            if(glm_aabb_frustum(box, planes)) {
                                Chunk_RenderChunk(chunk, 0);
                                g_chunks_drawn++;
                            }
                            
                        }
                    }
                    break;
                case 4:
                    if(chunk) {
                        if(chunk->transparent_mesh && chunk->transparent_mesh->index_count)
                        {
                            vec3 box[2] = { {chunk->x * CHUNK_SIZE_XZ + chunk->aabb_transparent[0][0] - g_player_position[0], chunk->aabb_transparent[0][1] - g_player_position[1], chunk->y * CHUNK_SIZE_XZ + chunk->aabb_transparent[0][2] - g_player_position[2]},
                                            {chunk->x * CHUNK_SIZE_XZ + chunk->aabb_transparent[1][0] - g_player_position[0], chunk->aabb_transparent[1][1] - g_player_position[1], chunk->y * CHUNK_SIZE_XZ + chunk->aabb_transparent[1][2] - g_player_position[2]} };
                            if(glm_aabb_frustum(box, planes)) {
                                Chunk_RenderChunk(chunk, 1);
                                g_chunks_drawn++;
                            }
                        }
                    }
                    break;
                case 3:
                    if(g_draw_aabb_debug && chunk)
                    {
                        if(chunk->mesh && chunk->mesh->index_count)
                            Render_RenderDebugBox(
                                (vec3){chunk->x * CHUNK_SIZE_XZ + chunk->aabb_solid[0][0], chunk->aabb_solid[0][1], chunk->y * CHUNK_SIZE_XZ + chunk->aabb_solid[0][2]},
                                (vec3){chunk->aabb_solid[1][0] - chunk->aabb_solid[0][0], chunk->aabb_solid[1][1] - chunk->aabb_solid[0][1], chunk->aabb_solid[1][2] - chunk->aabb_solid[0][2]},
                                (vec4){1.0f, 1.0f, 0.0f, 1.0f}, 0);
                        if(chunk->transparent_mesh && chunk->transparent_mesh->index_count)
                            Render_RenderDebugBox(
                                (vec3){chunk->x * CHUNK_SIZE_XZ + chunk->aabb_transparent[0][0], chunk->aabb_transparent[0][1], chunk->y * CHUNK_SIZE_XZ + chunk->aabb_transparent[0][2]},
                                (vec3){chunk->aabb_transparent[1][0] - chunk->aabb_transparent[0][0], chunk->aabb_transparent[1][1] - chunk->aabb_transparent[0][1], chunk->aabb_transparent[1][2] - chunk->aabb_transparent[0][2]},
                                (vec4){0.0f, 1.0f, 0.0f, 1.0f}, 0);
                    }
                }
            }
        }
    }
    glEnable(GL_CULL_FACE);
}

void Chunk_RenderChunk(Chunk *chunk, char transparent)
{
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){chunk->x * CHUNK_SIZE_XZ - g_player_position[0], 0.0f - g_player_position[1], chunk->y * CHUNK_SIZE_XZ - g_player_position[2]});

    glUniformMatrix4fv(g_block_model_loc, 1, GL_FALSE, model[0]);
    glBindVertexArray((transparent ? chunk->transparent_mesh : chunk->mesh)->array_object);
    glDrawElements(GL_TRIANGLES, (transparent ? chunk->transparent_mesh : chunk->mesh)->index_count, (transparent ? chunk->transparent_mesh : chunk->mesh)->index_type, (void*)0);

    glBindVertexArray(0);
}

unsigned int Chunk_GetBlockIdAt(int x, int y, int z)
{
    if(y < 0 || y >= CHUNK_SIZE_Y)
        return 0;
    Chunk *chunk = Chunk_GetChunk(floorf((float)x / CHUNK_SIZE_XZ), floorf((float)z / CHUNK_SIZE_XZ));
    if(!chunk)
        return 0;
    return chunk->blocks[mod(x, CHUNK_SIZE_XZ)][y][mod(z, CHUNK_SIZE_XZ)].id;
}

void Chunk_SetBlockIdAt(int x, int y, int z, unsigned int block)
{
    if(y < 0 || y >= CHUNK_SIZE_Y)
        return;
    int chunk_x = floorf((float)x / CHUNK_SIZE_XZ);
    int chunk_y = floorf((float)z / CHUNK_SIZE_XZ);
    Chunk *chunk = Chunk_GetChunk(chunk_x, chunk_y);
    if(!chunk)
        return;
    x = mod(x, CHUNK_SIZE_XZ);
    z = mod(z, CHUNK_SIZE_XZ);
    chunk->blocks[x][y][z] = (Block){ block, 0 };
    Chunk_RegenerateChunkMesh(chunk);
    if(x == 0)
    {
        chunk = Chunk_GetChunk(chunk_x - 1, chunk_y);
        if(chunk)
            Chunk_RegenerateChunkMesh(chunk);
    }
    else if(x == CHUNK_SIZE_XZ - 1)
    {
        chunk = Chunk_GetChunk(chunk_x + 1, chunk_y);
        if(chunk)
            Chunk_RegenerateChunkMesh(chunk);
    }
    if(z == 0)
    {
        chunk = Chunk_GetChunk(chunk_x, chunk_y - 1);
        if(chunk)
            Chunk_RegenerateChunkMesh(chunk);
    }
    else if(z == CHUNK_SIZE_XZ - 1)
    {
        chunk = Chunk_GetChunk(chunk_x, chunk_y + 1);
        if(chunk)
            Chunk_RegenerateChunkMesh(chunk);
    }
}

Chunk *Chunk_GetChunk(int x, int y)
{
    Chunk* chunk = chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)];
    if(chunk && chunk->x == x && chunk->y == y)
        return chunk;
    return 0;
}
