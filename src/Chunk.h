#ifndef CHUNK_H
#define CHUNK_H

#define CHUNK_SIZE_XZ 16
#define CHUNK_SIZE_Y 256
#define CHUNK_VIEW_DISTANCE 12

#include "Block.h"

typedef struct _Vertex Vertex;
typedef struct _Mesh Mesh;

typedef struct _Chunk
{
    Block blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ];
    Mesh *mesh;
    Mesh *transparent_mesh;
    int x, y;

    Vertex *solid_vertex_buffer;
    int solid_vertex_count;
    unsigned int *solid_index_buffer;
    int solid_index_count;
    Vertex *transparent_vertex_buffer;
    int transparent_vertex_count;
    unsigned int *transparent_index_buffer;
    int transparent_index_count;
    char create_mesh;
    char locked;
} Chunk;

void Chunk_SetChunkArraySlot(int x, int y, Chunk* chunk);
Chunk *Chunk_CreateChunk(int x, int y);
void Chunk_RegenerateChunkMesh(Chunk *chunk);
void Chunk_PopulateChunkMeshBuffers(Chunk *chunk);
void Chunk_RenderChunk(Chunk *chunk, char transparent);
void Chunk_FreeChunk(Chunk *chunk);

Chunk *Chunk_GetChunk(int x, int y);
void Chunk_SetupGenerationThread();
void Render_RenderChunks();
unsigned int Chunk_GetBlockIdAt(int x, int y, int z);
void Chunk_SetBlockIdAt(int x, int y, int z, unsigned int block);

#endif // CHUNK_H