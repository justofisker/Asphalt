#ifndef BLOCK_H
#define BLOCK_H

typedef enum _BlockSide
{
    BLOCKSIDE_NORTH = 0,
    BLOCKSIDE_EAST,
    BLOCKSIDE_SOUTH,
    BLOCKSIDE_WEST,
    BLOCKSIDE_UP,
    BLOCKSIDE_DOWN
} BlockSide;

typedef struct _Block
{
    unsigned int tex_pos[6][2];
} Block;


Block* get_block(int id);

void setup_blocks();

#endif // BLOCK_H
