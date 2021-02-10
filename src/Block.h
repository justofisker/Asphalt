#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

typedef enum _BlockId
{
    BLOCKID_AIR = 0,
    BLOCKID_GRASS,
    BLOCKID_DIRT,
    BLOCKID_STONE,
    BLOCKID_WATER,
    BLOCKID_SAND,
    BLOCKID_COUNT
} BlockId;

typedef enum _BlockSide
{
    BLOCKSIDE_NORTH = 0,
    BLOCKSIDE_EAST,
    BLOCKSIDE_SOUTH,
    BLOCKSIDE_WEST,
    BLOCKSIDE_UP,
    BLOCKSIDE_DOWN
} BlockSide;

typedef enum _BlockFlags
{
    BLOCKFLAG_NONE = 0,
    BLOCKFLAG_TRANSPARENT = 1,
    BLOCKFLAG_NO_COLLISION = 2,
    BLOCKFLAG_FLUID_MOVEMENT = 4,
} BlockFlags;

typedef struct _BlockInfo
{
    unsigned int tex_pos[6][2];
    int flags;
    char *name;
} BlockInfo;

typedef struct _Block
{
    uint16_t id;
    uint8_t data;
} Block;

/* Data 
    0 - 3 Light Data
    4 - 7 ?
*/

BlockInfo* get_block_info(short id);

void setup_blocks();

#endif // BLOCK_H
