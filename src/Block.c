#include "Block.h"

#include <stdlib.h>

#define BLOCK_COUNT 3
Block blocks[BLOCK_COUNT];

void setup_blocks()
{

    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_NORTH][0] = 1;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_EAST][0]  = 1;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_SOUTH][0] = 1;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_WEST][0]  = 1;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_UP][0]    = 0;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_DOWN][0]  = 2;
    blocks[BLOCK_GRASS - 1].tex_pos[BLOCKSIDE_DOWN][1]  = 0;

    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_NORTH][0] = 2;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_EAST][0]  = 2;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_SOUTH][0] = 2;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_WEST][0]  = 2;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_UP][0]    = 2;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_DOWN][0]  = 2;
    blocks[BLOCK_DIRT - 1].tex_pos[BLOCKSIDE_DOWN][1]  = 0;

    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_NORTH][0] = 3;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_EAST][0]  = 3;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_SOUTH][0] = 3;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_WEST][0]  = 3;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_UP][0]    = 3;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_DOWN][0]  = 3;
    blocks[BLOCK_STONE - 1].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
}

Block* get_block(short id)
{
    if(id && id > 0 && id <= BLOCK_COUNT)
    {
        return &blocks[id - 1];
    }
    return 0;
}