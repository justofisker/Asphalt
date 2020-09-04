#ifndef CHUNK_H
#define CHUNK_H

#define CHUNK_SIZE_XZ 16
#define CHUNK_SIZE_Y 256

typedef struct _Vertex Vertex;

typedef struct _Mesh Mesh;

typedef struct _Chunk
{
    unsigned short blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ];
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

Chunk *create_chunk(int x, int y);
void create_vertex_index_buffer(Chunk* chunk);
void regenerate_chunk_mesh(Chunk *chunk);
void render_chunk(Chunk *chunk, char transparent);
void free_chunk(Chunk *chunk);

Chunk *get_chunk(int x, int y);
void generate_chunks();
void render_chunks();
short get_block_id_at(int x, int y, int z);
void set_block_at(int x, int y, int z, short block);

#endif // CHUNK_H