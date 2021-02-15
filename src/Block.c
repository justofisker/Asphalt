#include "Block.h"

#include <stdlib.h>

BlockInfo blocks_info[BLOCKID_COUNT];

void Block_Setup()
{

    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_NORTH][0] = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_EAST][0]  = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_SOUTH][0] = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_WEST][0]  = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_UP][0]    = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_DOWN][0]  = 0;
    blocks_info[BLOCKID_AIR].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks_info[BLOCKID_AIR].flags = BLOCKFLAG_TRANSPARENT | BLOCKFLAG_NO_COLLISION;
    blocks_info[BLOCKID_AIR].name = "Air";

    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_NORTH][0] = 1;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_EAST][0]  = 1;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_SOUTH][0] = 1;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_WEST][0]  = 1;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_UP][0]    = 0;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_DOWN][0]  = 2;
    blocks_info[BLOCKID_GRASS].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks_info[BLOCKID_GRASS].flags = BLOCKFLAG_NONE;
    blocks_info[BLOCKID_GRASS].name = "Grass";

    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_NORTH][0] = 2;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_EAST][0]  = 2;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_SOUTH][0] = 2;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_WEST][0]  = 2;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_UP][0]    = 2;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_DOWN][0]  = 2;
    blocks_info[BLOCKID_DIRT].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks_info[BLOCKID_DIRT].flags = BLOCKFLAG_NONE;
    blocks_info[BLOCKID_DIRT].name = "Dirt";

    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_NORTH][0] = 3;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_EAST][0]  = 3;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_SOUTH][0] = 3;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_WEST][0]  = 3;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_UP][0]    = 3;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_DOWN][0]  = 3;
    blocks_info[BLOCKID_STONE].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks_info[BLOCKID_STONE].flags = BLOCKFLAG_NONE;
    blocks_info[BLOCKID_STONE].name = "Stone";

    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_NORTH][0] = 4;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_EAST][0]  = 4;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_SOUTH][0] = 4;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_WEST][0]  = 4;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_UP][0]    = 4;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_DOWN][0]  = 4;
    blocks_info[BLOCKID_WATER].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks_info[BLOCKID_WATER].flags = BLOCKFLAG_TRANSPARENT | BLOCKFLAG_NO_COLLISION | BLOCKFLAG_FLUID_MOVEMENT;
    blocks_info[BLOCKID_WATER].name = "Water";

    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_NORTH][0] = 5;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_NORTH][1] = 0;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_EAST][0]  = 5;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_EAST][1]  = 0;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_SOUTH][0] = 5;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_SOUTH][1] = 0;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_WEST][0]  = 5;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_WEST][1]  = 0;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_UP][0]    = 5;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_UP][1]    = 0;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_DOWN][0]  = 5;
    blocks_info[BLOCKID_SAND].tex_pos[BLOCKSIDE_DOWN][1]  = 0;
    blocks_info[BLOCKID_SAND].flags = BLOCKFLAG_NONE;
    blocks_info[BLOCKID_SAND].name = "Sand";
}

BlockInfo* Block_GetBlockInfo(short id)
{
    if(id >= 0 && id < BLOCKID_COUNT)
    {
        return &blocks_info[id];
    }
    return 0;
}