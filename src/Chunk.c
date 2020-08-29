#include "Chunk.h"

#include <stdlib.h>
#include <string.h>

Chunk *create_chunk()
{
    Chunk *chunk = malloc(sizeof(Chunk));
    memset(chunk->blocks, 0, sizeof(chunk->blocks));

    int x, y, z;
    for(x = 0; x < 16; ++x)
    {
        for(y = 0; y < 3; ++y)
        {
            for(z = 0; z < 16; ++z)
            {
                chunk->blocks[x][y][z] = 1;
            }
        }
    }

    return chunk;
}

void free_chunk(Chunk *chunk)
{
    free(chunk);
}