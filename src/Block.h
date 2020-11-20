#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_AIR   0
#define BLOCK_GRASS 1
#define BLOCK_DIRT  2
#define BLOCK_STONE 3
#define BLOCK_WATER 4
#define BLOCK_SAND  5
#define BLOCK_COUNT 6

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

typedef struct _Block
{
    unsigned int tex_pos[6][2];
    int flags;
    char *name;
} Block;


Block* get_block(short id);

void setup_blocks();

#endif // BLOCK_H
