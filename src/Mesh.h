#ifndef MESH_H
#define MESH_H

typedef struct _Chunk Chunk;
typedef unsigned int GLenum;

typedef struct _Mesh
{
    unsigned int array_object;
    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int index_count;
    GLenum index_type;
} Mesh;

void free_mesh(Mesh *mesh);

#endif // MESH_H