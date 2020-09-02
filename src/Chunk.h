#ifndef CHUNK_H
#define CHUNK_H

#define CHUNK_SIZE_XZ 16
#define CHUNK_SIZE_Y 256

typedef struct _Mesh Mesh;

typedef struct _Chunk
{
    unsigned short blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ];
    Mesh *mesh;
    Mesh *transparent_mesh;
    int x, y;
} Chunk;

Chunk *create_chunk(int x, int y);
void create_mesh_from_chunk(Chunk *chunk);
void regenerate_chunk_mesh(Chunk *chunk);
void render_chunk(Chunk *chunk, char transparent);
void free_chunk(Chunk *chunk);

Chunk *get_chunk(int x, int y);
void generate_chunks();
void render_chunks();
short get_block_id_at(int x, int y, int z);
void set_block_at(int x, int y, int z, short block);

#endif // CHUNK_H