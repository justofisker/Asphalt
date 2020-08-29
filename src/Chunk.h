#ifndef CHUNK_H
#define CHUNK_H

typedef struct _Chunk
{
    unsigned int blocks[16][32][16];
} Chunk;

Chunk *create_chunk();
void free_chunk(Chunk *chunk);

#endif // CHUNK_H