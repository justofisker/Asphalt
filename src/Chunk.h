#ifndef CHUNK_H
#define CHUNK_H

#define CHUNK_SIZE_XZ 16
#define CHUNK_SIZE_Y 64

typedef struct _Mesh Mesh;

typedef struct _Chunk
{
    unsigned int blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ];
    Mesh *mesh;
    int x, y;
} Chunk;

Chunk *create_chunk(int x, int y);
Mesh *create_mesh_from_chunk(Chunk *chunk);
void regenerate_chunk_mesh(Chunk *chunk);
void render_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);

#endif // CHUNK_H