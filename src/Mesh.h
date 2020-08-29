#ifndef MESH_H
#define MESH_H

typedef struct _Chunk Chunk;

typedef struct _Mesh
{
    unsigned int array_object;
    unsigned int vertex_buffer;
    unsigned int index_buffer;
} Mesh;

Mesh *create_mesh_from_chunk(Chunk *chunk);
void free_mesh(Mesh *mesh);

#endif // MESH_H