#include "Block.h"

#include <stdlib.h>

#define BLOCK_COUNT 6
Block blocks[BLOCK_COUNT];

void setup_blocks()
{

    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_NORTH][0] = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_EAST][0]  = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_SOUTH][0] = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_WEST][0]  = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_UP][0]    = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_DOWN][0]  = 0;
    blocks[BLOCK_AIR].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks[BLOCK_AIR].flags = BLOCKFLAG_TRANSPARENT;

    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_NORTH][0] = 1;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_EAST][0]  = 1;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_SOUTH][0] = 1;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_WEST][0]  = 1;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_UP][0]    = 0;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_DOWN][0]  = 2;
    blocks[BLOCK_GRASS].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks[BLOCK_GRASS].flags = 0;

    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_NORTH][0] = 2;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_EAST][0]  = 2;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_SOUTH][0] = 2;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_WEST][0]  = 2;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_UP][0]    = 2;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_DOWN][0]  = 2;
    blocks[BLOCK_DIRT].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks[BLOCK_DIRT].flags = 0;

    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_NORTH][0] = 3;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_EAST][0]  = 3;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_SOUTH][0] = 3;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_WEST][0]  = 3;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_UP][0]    = 3;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_DOWN][0]  = 3;
    blocks[BLOCK_STONE].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks[BLOCK_STONE].flags = 0;

    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_NORTH][0] = 4;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_EAST][0]  = 4;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_SOUTH][0] = 4;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_WEST][0]  = 4;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_UP][0]    = 4;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_DOWN][0]  = 4;
    blocks[BLOCK_WATER].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks[BLOCK_WATER].flags = BLOCKFLAG_TRANSPARENT;

    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_NORTH][0] = 5;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_EAST][0]  = 5;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_SOUTH][0] = 5;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_WEST][0]  = 5;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_UP][0]    = 5;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_DOWN][0]  = 5;
    blocks[BLOCK_SAND].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks[BLOCK_SAND].flags = 0;
}

Block* get_block(short id)
{
    if(id >= 0 && id < BLOCK_COUNT)
    {
        return &blocks[id];
    }
    return 0;
}