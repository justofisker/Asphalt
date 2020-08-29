#include "Mesh.h"

#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Chunk.h"
#include <cglm/cglm.h>

void free_mesh(Mesh *mesh)
{
    glDeleteVertexArrays(1, &mesh->array_object);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->index_buffer);
    free(mesh);
}