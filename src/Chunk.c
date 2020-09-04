#define CHUNK_DEBUG
#include "Chunk.h"

#include "Mesh.h"
#include "Globals.h"
#include "Util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GL/glut.h>
#include "Block.h"

typedef struct _Vertex
{
    vec3 position;
    vec2 uv;
    vec3 normal;
} Vertex;

typedef unsigned int Index;

#define TEX_CELL_COUNT_XY 8
#define TEX_CELL_SIZE_XY (1.0f / (float)TEX_CELL_COUNT_XY)

#define CHUNK_ARR_SIZE 64
Chunk *chunks[CHUNK_ARR_SIZE][CHUNK_ARR_SIZE];
Chunk *chunks_to_free[CHUNK_ARR_SIZE][CHUNK_ARR_SIZE];
int last_x = 0;
int last_y = 0;
static char dontGenerate = 0;

void set_chunk(int x, int y, Chunk* chunk)
{
    Chunk *previous = chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)];
    chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)] = chunk;
    if(previous)
        chunks_to_free[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)] = previous;
}

#ifdef _WIN32
HANDLE chunk_thread;
DWORD chunk_thread_id;

DWORD WINAPI create_chunks(LPVOID lpParam)
#elif __linux__

#endif
{
    while(1)
    {

    int x, y;
    int chunks_generated = 0;
    int cur_x = (int)(global_player_position[0]) / CHUNK_SIZE_XZ;
    int cur_y = (int)(global_player_position[2]) / CHUNK_SIZE_XZ;
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
        for(int i =0; i < maxI; i++){
            if ((-CHUNK_ARR_SIZE/2 <= x) && (x <= CHUNK_ARR_SIZE/2) && (-CHUNK_ARR_SIZE/2 <= y) && (y <= CHUNK_ARR_SIZE/2)){
                if(!get_chunk(x + cur_x, y + cur_y))
                {
                    Chunk *chunk = create_chunk(x + cur_x, y + cur_y);
                    create_vertex_index_buffer(chunk);
                    chunk->create_mesh = 1;
                    if(chunks[mod(x + cur_x, CHUNK_ARR_SIZE)][mod(y + cur_y, CHUNK_ARR_SIZE)]) while(chunks[mod(x + cur_x, CHUNK_ARR_SIZE)][mod(y + cur_y, CHUNK_ARR_SIZE)]->locked) { Sleep(0); }
                    set_chunk(x + cur_x, y + cur_y, chunk);
                    
                    chunk = get_chunk(x + cur_x - 1, y + cur_y);
                    if(chunk)
                    {
                        while(chunk->locked) { Sleep(0); }
                        chunk->locked = 1;
                        create_vertex_index_buffer(chunk);
                        chunk->create_mesh = 1;
                        chunk->locked = 0;
                    }
                    chunk = get_chunk(x + cur_x + 1, y + cur_y);
                    if(chunk)
                    {
                        while(chunk->locked) { Sleep(0); }
                        chunk->locked = 1;
                        create_vertex_index_buffer(chunk);
                        chunk->create_mesh = 1;
                        chunk->locked = 0;
                    }
                    chunk = get_chunk(x + cur_x, y + cur_y - 1);
                    if(chunk)
                    {
                        while(chunk->locked) { Sleep(0); }
                        chunk->locked = 1;
                        create_vertex_index_buffer(chunk);
                        chunk->create_mesh = 1;
                        chunk->locked = 0;
                    }
                    chunk = get_chunk(x + cur_x, y + cur_y + 1);
                    if(chunk)
                    {
                        while(chunk->locked) { Sleep(0); }
                        chunk->locked = 1;
                        create_vertex_index_buffer(chunk);
                        chunk->create_mesh = 1;
                        chunk->locked = 0;
                    }


                    if (cur_x != (int)(global_player_position[0]) / CHUNK_SIZE_XZ
                     || cur_y != (int)(global_player_position[2]) / CHUNK_SIZE_XZ)
                    { 
                        cur_x = (int)(global_player_position[0]) / CHUNK_SIZE_XZ;
                        cur_y = (int)(global_player_position[2]) / CHUNK_SIZE_XZ;
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

    Sleep(50);

    }
}

Chunk *create_chunk(int _x, int _y)
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

    int x,y, z;
    for(x = 0; x < CHUNK_SIZE_XZ; ++x)
    {
        for(z = 0; z < CHUNK_SIZE_XZ; ++z)
        {
            //           V Because we cant use negative numbers (please fix it is very hacky)
            int act_x = 1000000 + CHUNK_SIZE_XZ * _x + x;
            int act_z = 1000000 + CHUNK_SIZE_XZ * _y + z;
            int act_y = fmaxf(fminf(CHUNK_SIZE_Y, 
                (perlin2d(act_x, act_z, 0.02f, 5) * 20)
                + (perlin2d(act_x, act_z, 0.0044, 2) * 100)
            ), 1);
            chunk->blocks[x][act_y - 1][z] = act_y > 55 ? BLOCK_GRASS : BLOCK_SAND;
            for(y = act_y - 2; y >= 0; y--)
                chunk->blocks[x][y][z]= y > 20 ? BLOCK_DIRT : BLOCK_STONE;
            for(y = 0; y < 55; y++)
                if(!chunk->blocks[x][y][z])
                    chunk->blocks[x][y][z] = BLOCK_WATER;
        }
    }

    return chunk;
}

void create_vertex_index_buffer(Chunk* chunk)
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

    // Solid Mesh
    {
        int face_count = 0;

        int x, y, z, i;
        for(x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            for(y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for(z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    if((get_block(chunk->blocks[x][y][z])->flags & BLOCKFLAG_TRANSPARENT) == 0)
                    {
                        char north = z + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x, chunk->y + 1) ? get_block(get_chunk(chunk->x, chunk->y + 1)->blocks[x][y][0]                )->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x][y][z + 1])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char south = z - 1 ==            -1 ? (get_chunk(chunk->x, chunk->y - 1) ? get_block(get_chunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1])->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x][y][z - 1])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char west  = x - 1 ==            -1 ? (get_chunk(chunk->x - 1, chunk->y) ? get_block(get_chunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z])->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x - 1][y][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char east  = x + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x + 1, chunk->y) ? get_block(get_chunk(chunk->x + 1, chunk->y)->blocks[0][y][z]                )->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x + 1][y][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char up    = y + 1 ==  CHUNK_SIZE_Y ? 1 : get_block(chunk->blocks[x][y + 1][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char down  = y - 1 ==            -1 ? 1 : get_block(chunk->blocks[x][y - 1][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        face_count += north;
                        face_count += south;
                        face_count += west;
                        face_count += east;
                        face_count += up;
                        face_count += down;
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
                    if((get_block(chunk->blocks[x][y][z])->flags & BLOCKFLAG_TRANSPARENT) == 0)
                    {
                        char north = z + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x, chunk->y + 1) ? get_block(get_chunk(chunk->x, chunk->y + 1)->blocks[x][y][0]                )->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x][y][z + 1])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char south = z - 1 ==            -1 ? (get_chunk(chunk->x, chunk->y - 1) ? get_block(get_chunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1])->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x][y][z - 1])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char west  = x - 1 ==            -1 ? (get_chunk(chunk->x - 1, chunk->y) ? get_block(get_chunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z])->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x - 1][y][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char east  = x + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x + 1, chunk->y) ? get_block(get_chunk(chunk->x + 1, chunk->y)->blocks[0][y][z]                )->flags & BLOCKFLAG_TRANSPARENT != 0 : 1) : get_block(chunk->blocks[x + 1][y][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char up    = y + 1 ==  CHUNK_SIZE_Y ? 1 : get_block(chunk->blocks[x][y + 1][z])->flags & BLOCKFLAG_TRANSPARENT != 0;
                        char down  = y - 1 ==            -1 ? 1 : get_block(chunk->blocks[x][y - 1][z])->flags & BLOCKFLAG_TRANSPARENT != 0;

                        Block* block = get_block(chunk->blocks[x][y][z]);
                        if(!block)
                        {
                            printf("MISSING BLOCK INFO %d\n", chunk->blocks[x][y][z]);
                            continue;
                        }

                        if(south)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            offset += 4;
                        }
                        if(east)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 3] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            offset += 4;
                        }
                        if(north)
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            offset += 4;
                        }
                        if(west)
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 1] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            offset += 4;
                        }
                        if(up)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 1] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 2] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            offset += 4;
                        }
                        if(down)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 3] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
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
        int face_count = 0;

        int x, y, z, i;
        for(x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            for(y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for(z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    if(chunk->blocks[x][y][z] && (get_block(chunk->blocks[x][y][z])->flags & BLOCKFLAG_TRANSPARENT) != 0)
                    {
                        char north = z + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x, chunk->y + 1) ? get_chunk(chunk->x, chunk->y + 1)->blocks[x][y][0]                 == 0 : 0) : chunk->blocks[x][y][z + 1] == 0;
                        char south = z - 1 ==            -1 ? (get_chunk(chunk->x, chunk->y - 1) ? get_chunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1] == 0 : 0) : chunk->blocks[x][y][z - 1] == 0;
                        char west  = x - 1 ==            -1 ? (get_chunk(chunk->x - 1, chunk->y) ? get_chunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z] == 0 : 0) : chunk->blocks[x - 1][y][z] == 0;
                        char east  = x + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x + 1, chunk->y) ? get_chunk(chunk->x + 1, chunk->y)->blocks[0][y][z]                 == 0 : 0) : chunk->blocks[x + 1][y][z] == 0;
                        char up    = y + 1 ==  CHUNK_SIZE_Y ? 1 : chunk->blocks[x][y + 1][z] == 0;
                        char down  = y - 1 ==            -1 ? 1 : chunk->blocks[x][y - 1][z] == 0;
                        face_count += north;
                        face_count += south;
                        face_count += west;
                        face_count += east;
                        face_count += up;
                        face_count += down;
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
                    if(chunk->blocks[x][y][z] && (get_block(chunk->blocks[x][y][z])->flags & BLOCKFLAG_TRANSPARENT) != 0)
                    {
                        char north = z + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x, chunk->y + 1) ? get_chunk(chunk->x, chunk->y + 1)->blocks[x][y][0]                 == 0 : 0) : chunk->blocks[x][y][z + 1] == 0;
                        char south = z - 1 ==            -1 ? (get_chunk(chunk->x, chunk->y - 1) ? get_chunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1] == 0 : 0) : chunk->blocks[x][y][z - 1] == 0;
                        char west  = x - 1 ==            -1 ? (get_chunk(chunk->x - 1, chunk->y) ? get_chunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z] == 0 : 0) : chunk->blocks[x - 1][y][z] == 0;
                        char east  = x + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x + 1, chunk->y) ? get_chunk(chunk->x + 1, chunk->y)->blocks[0][y][z]                 == 0 : 0) : chunk->blocks[x + 1][y][z] == 0;
                        char up    = y + 1 ==  CHUNK_SIZE_Y ? 1 : chunk->blocks[x][y + 1][z] == 0;
                        char down  = y - 1 ==            -1 ? 1 : chunk->blocks[x][y - 1][z] == 0;

                        Block* block = get_block(chunk->blocks[x][y][z]);
                        if(!block)
                        {
                            printf("MISSING BLOCK INFO %d\n", chunk->blocks[x][y][z]);
                            continue;
                        }

                        if(south)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 1)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_SOUTH][1] + 0)  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                            offset += 4;
                        }
                        if(east)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 1)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            verticies[offset + 3] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_EAST][1] + 0)  ,  1.0f,  0.0f,  0.0f}; // EAST
                            offset += 4;
                        }
                        if(north)
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 1)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_NORTH][1] + 0)  ,  0.0f,  0.0f,  1.0f}; // NORTH
                            offset += 4;
                        }
                        if(west)
                        {
                            verticies[offset + 0] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 1] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 1)  , -1.0f,  0.0f,  0.0f}; // WEST
                            verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_WEST][1] + 0)  , -1.0f,  0.0f,  0.0f}; // WEST
                            offset += 4;
                        }
                        if(up)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.9f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.9f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 1) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.9f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            verticies[offset + 3] = (Vertex){0.0f + x, 0.9f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_UP][1] + 0) ,  0.0f,  1.0f,  0.0f}; // UP
                            offset += 4;
                        }
                        if(down)
                        {
                            verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 1), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 0)  ,  0.0f, -1.0f,  0.0f}; // DOWN
                            verticies[offset + 3] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][0] + 0), 1.0f - TEX_CELL_SIZE_XY * (block->tex_pos[BLOCKSIDE_DOWN][1] + 1)  ,  0.0f, -1.0f,  0.0f}; // DOWN
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

void create_mesh_from_chunk(Chunk *chunk)
{
    if(chunk->mesh) free_mesh(chunk->mesh);
    if(chunk->transparent_mesh) free_mesh(chunk->transparent_mesh);
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

void regenerate_chunk_mesh(Chunk *chunk)
{
    create_vertex_index_buffer(chunk);
    create_mesh_from_chunk(chunk);
}

void render_chunk(Chunk *chunk, char transparent)
{

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){chunk->x * CHUNK_SIZE_XZ, 0.0f, chunk->y * CHUNK_SIZE_XZ});

    glUniformMatrix4fv(global_block_model_loc, 1, GL_FALSE, model[0]);
    glBindVertexArray((transparent ? chunk->transparent_mesh : chunk->mesh)->array_object);
    glDrawElements(GL_TRIANGLES, (transparent ? chunk->transparent_mesh : chunk->mesh)->index_count, (transparent ? chunk->transparent_mesh : chunk->mesh)->index_type, (void*)0);

    glBindVertexArray(0);
}

void free_chunk(Chunk *chunk)
{
    if(chunk->mesh) free_mesh(chunk->mesh);
    if(chunk->transparent_mesh) free_mesh(chunk->transparent_mesh);
    if(chunk->solid_vertex_buffer) free(chunk->solid_vertex_buffer);
    if(chunk->solid_index_buffer) free(chunk->solid_index_buffer);
    if(chunk->transparent_vertex_buffer) free(chunk->transparent_vertex_buffer);
    if(chunk->transparent_index_buffer) free(chunk->transparent_index_buffer);
    free(chunk);
}

Chunk *get_chunk(int x, int y)
{
    Chunk* chunk = chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)];
    if(chunk && chunk->x == x && chunk->y == y)
        return chunk;
    return 0;
}

void generate_chunks()
{
    memset(chunks, 0, sizeof(chunks));
    memset(chunks_to_free, 0, sizeof(chunks_to_free));
#ifdef _WIN32
    chunk_thread = CreateThread(NULL, 0, create_chunks, NULL, 0, &chunk_thread_id);
#endif
}

void render_chunks()
{
    glm_perspective(glm_rad(70.0f), (float)global_width / global_height, 0.1f, 700.0f, global_projection);
    glm_mat4_identity(global_view);
    mat4 rotation;
    glm_euler_xyz(global_camera_rotation, rotation);
    glm_mul_rot(global_view, rotation, global_view);
    glm_translate(global_view, (vec3){-global_player_position[0], -global_player_position[1], -global_player_position[2]});
    glm_translate(global_view, (vec3){-global_camera_offset[0], -global_camera_offset[1], -global_camera_offset[2]});

    glUseProgram(global_block_shader);
    bind_texture(global_texture, 0);
    glUniformMatrix4fv(global_block_view_loc, 1, GL_FALSE, global_view[0]);
    glUniformMatrix4fv(global_block_projection_loc, 1, GL_FALSE, global_projection[0]);
    glUniform1i(global_block_texture_loc, 0);

    int x, y;
    for(x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for(y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            if(chunks_to_free[x][y])
            {
                free_chunk(chunks_to_free[x][y]);
                chunks_to_free[x][y] = 0;
            }
        }
    }

    for(x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for(y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            Chunk *chunk = chunks[x][y];
            if(chunk && chunk->create_mesh && !chunk->locked)
            {
                chunk->locked = 1;
                create_mesh_from_chunk(chunk);
                chunks[x][y]->locked = 0;
            }
        }
    }

    for(x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for(y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            Chunk *chunk = chunks[x][y];
            if(chunk) {
                if(chunk->mesh && chunk->mesh->index_count)
                {
                    render_chunk(chunk, 0);
                }
            }
        }
    }
    glDisable(GL_CULL_FACE);
    for(x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for(y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            Chunk *chunk = chunks[x][y];
            if(chunk) {
                if(chunk->transparent_mesh && chunk->transparent_mesh->index_count)
                {
                    render_chunk(chunk, 1);
                }
            }
        }
    }
    glEnable(GL_CULL_FACE);
}

short get_block_id_at(int x, int y, int z)
{
    if(y < 0 || y >= CHUNK_SIZE_Y)
        return 0;
    Chunk *chunk = get_chunk(floorf((float)x / CHUNK_SIZE_XZ), floorf((float)z / CHUNK_SIZE_XZ));
    if(!chunk)
        return 0;
    return chunk->blocks[mod(x, CHUNK_SIZE_XZ)][y][mod(z, CHUNK_SIZE_XZ)];
}

void set_block_at(int x, int y, int z, short block)
{
    if(y < 0 || y >= CHUNK_SIZE_Y)
        return;
    int chunk_x = floorf((float)x / CHUNK_SIZE_XZ);
    int chunk_y = floorf((float)z / CHUNK_SIZE_XZ);
    Chunk *chunk = get_chunk(chunk_x, chunk_y);
    if(!chunk)
        return;
    x = mod(x, CHUNK_SIZE_XZ);
    z = mod(z, CHUNK_SIZE_XZ);
    if (chunk->blocks[x][y][z] != block)
    {
        chunk->blocks[x][y][z] = block;
        regenerate_chunk_mesh(chunk);
        if(x == 0)
        {
            chunk = get_chunk(chunk_x - 1, chunk_y);
            if(chunk)
                regenerate_chunk_mesh(chunk);
        }
        else if(x == CHUNK_SIZE_XZ - 1)
        {
            chunk = get_chunk(chunk_x + 1, chunk_y);
            if(chunk)
                regenerate_chunk_mesh(chunk);
        }
        if(z == 0)
        {
            chunk = get_chunk(chunk_x, chunk_y - 1);
            if(chunk)
                regenerate_chunk_mesh(chunk);
        }
        else if(z == CHUNK_SIZE_XZ - 1)
        {
            chunk = get_chunk(chunk_x, chunk_y + 1);
            if(chunk)
                regenerate_chunk_mesh(chunk);
        }
    }
}