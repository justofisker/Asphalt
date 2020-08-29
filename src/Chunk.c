#include "Chunk.h"

#include "Mesh.h"
#include "Globals.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

Chunk *create_chunk(int x, int y)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    memset(chunk->blocks, 0, sizeof(chunk->blocks));

    chunk->x = x;
    chunk->y = y;

    int z;
    for(x = 0; x < CHUNK_SIZE_XZ; ++x)
    {
        for(y = 0; y < 3; ++y)
        {
            for(z = 0; z < CHUNK_SIZE_XZ; ++z)
            {
                chunk->blocks[x][y][z] = 1;
            }
        }
    }
    
    chunk->mesh = create_mesh_from_chunk(chunk);

    return chunk;
}

typedef struct _Vertex
{
    vec3 position;
    vec2 uv;
    vec3 normal;
} Vertex;

typedef unsigned int Index;

Mesh *create_mesh_from_chunk(Chunk *chunk)
{
    Mesh *mesh = malloc(sizeof(Mesh));

    int face_count = 0;

    int x, y, z, i;
    for(x = 0; x < CHUNK_SIZE_XZ; x++)
    {
        for(y = 0; y < CHUNK_SIZE_XZ; y++)
        {
            for(z = 0; z < CHUNK_SIZE_XZ; z++)
            {
                if(chunk->blocks[x][y][z] == 1)
                {
                    char north = z + 1 == CHUNK_SIZE_XZ ? 1 : chunk->blocks[x][y][z + 1] == 0;
                    char south = z - 1 == -1 ? 1 : chunk->blocks[x][y][z - 1] == 0;
                    char west  = x - 1 == -1 ? 1 : chunk->blocks[x - 1][y][z] == 0;
                    char east  = x + 1 == CHUNK_SIZE_XZ ? 1 : chunk->blocks[x + 1][y][z] == 0;
                    char up    = y + 1 == CHUNK_SIZE_Y ? 0 : chunk->blocks[x][y + 1][z] == 0;
                    char down  = y - 1 == -1 ? 1 : chunk->blocks[x][y - 1][z] == 0;
                    face_count += north + south + west + east + up + down;
                }
            }
        }
    }

    int offset = 0;
    Vertex *verticies = malloc(sizeof(Vertex) * 4 * face_count);
    for(x = 0; x < CHUNK_SIZE_XZ; ++x)
    {
        for(y = 0; y < CHUNK_SIZE_Y; ++y)
        {
            for(z = 0; z < CHUNK_SIZE_XZ; ++z)
            {
                if(chunk->blocks[x][y][z])
                {
                    char north = z + 1 == CHUNK_SIZE_XZ ? 1 : chunk->blocks[x][y][z + 1] == 0;
                    char south = z - 1 == -1 ? 1 : chunk->blocks[x][y][z - 1] == 0;
                    char west  = x - 1 == -1 ? 1 : chunk->blocks[x - 1][y][z] == 0;
                    char east  = x + 1 == CHUNK_SIZE_XZ ? 1 : chunk->blocks[x + 1][y][z] == 0;
                    char up    = y + 1 == CHUNK_SIZE_Y ? 1 : chunk->blocks[x][y + 1][z] == 0;
                    char down  = y - 1 == -1 ? 1 : chunk->blocks[x][y - 1][z] == 0;
                    
                    if(north)
                    {
                        verticies[offset + 0] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  0.0f, 0.0f  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  1.0f, 0.0f  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  1.0f, 1.0f  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  0.0f, 1.0f  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        offset += 4;
                    }
                    if(south)
                    {
                        verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  0.0f, 0.0f  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  1.0f, 0.0f  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  1.0f, 1.0f  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  0.0f, 1.0f  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        offset += 4;
                    }
                    if(west)
                    {
                        verticies[offset + 0] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  0.0f, 0.0f  , -1.0f,  0.0f,  0.0f}; // WEST
                        verticies[offset + 1] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  1.0f, 0.0f  , -1.0f,  0.0f,  0.0f}; // WEST
                        verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  1.0f, 1.0f  , -1.0f,  0.0f,  0.0f}; // WEST
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  0.0f, 1.0f  , -1.0f,  0.0f,  0.0f}; // WEST
                        offset += 4;
                    }
                    if(east)
                    {
                        verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  0.0f, 0.0f  ,  1.0f,  0.0f,  0.0f}; // EAST
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  1.0f, 0.0f  ,  1.0f,  0.0f,  0.0f}; // EAST
                        verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  1.0f, 1.0f  ,  1.0f,  0.0f,  0.0f}; // EAST
                        verticies[offset + 3] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  0.0f, 1.0f  ,  1.0f,  0.0f,  0.0f}; // EAST
                        offset += 4;
                    }
                    if(up)
                    {
                        verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  0.0f, 0.0f  ,  0.0f,  1.0f,  0.0f}; // UP
                        verticies[offset + 1] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  1.0f, 0.0f  ,  0.0f,  1.0f,  0.0f}; // UP
                        verticies[offset + 2] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  1.0f, 1.0f  ,  0.0f,  1.0f,  0.0f}; // UP
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  0.0f, 1.0f  ,  0.0f,  1.0f,  0.0f}; // UP
                        offset += 4;
                    }
                    if(down)
                    {
                        verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  0.0f, 0.0f  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  1.0f, 0.0f  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  1.0f, 1.0f  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        verticies[offset + 3] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  0.0f, 1.0f  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        offset += 4;
                    }

                }
            }
        }
    }


    Index *indicies = malloc(sizeof(Index) * 6 * face_count);
    for(i = 0; i < face_count; ++i)
    {
        indicies[i * 6 + 0] = 0 + 4 * i;
        indicies[i * 6 + 1] = 1 + 4 * i;
        indicies[i * 6 + 2] = 2 + 4 * i;
        indicies[i * 6 + 3] = 2 + 4 * i;
        indicies[i * 6 + 4] = 3 + 4 * i;
        indicies[i * 6 + 5] = 0 + 4 * i;
    }

    glGenVertexArrays(1, &mesh->array_object);
    glBindVertexArray(mesh->array_object);

    glGenBuffers(1, &mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4 * face_count, verticies, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glGenBuffers(1, &mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * 6 * face_count, indicies, GL_STATIC_DRAW);

    glBindVertexArray(0);

    mesh->index_count = face_count * 6;
    mesh->index_type = GL_UNSIGNED_INT;

    free(verticies);
    free(indicies);

    return mesh;
}

void regenerate_chunk_mesh(Chunk *chunk)
{
    free_mesh(chunk->mesh);
    chunk->mesh = create_mesh_from_chunk(chunk);
}

void render_chunk(Chunk *chunk)
{   
    glUseProgram(global_basic_shader);

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){chunk->x * CHUNK_SIZE_XZ, 0.0f, chunk->y * CHUNK_SIZE_XZ});

    bind_texture(global_texture, 0);
    glUniformMatrix4fv(global_basic_model_loc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(global_basic_view_loc, 1, GL_FALSE, global_view[0]);
    glUniformMatrix4fv(global_basic_projection_loc, 1, GL_FALSE, global_projection[0]);
    glUniform1i(global_basic_texture_loc, 0);
    glBindVertexArray(chunk->mesh->array_object);
    glDrawElements(GL_TRIANGLES, chunk->mesh->index_count, chunk->mesh->index_type, (void*)0);
    glBindVertexArray(0);
}

void free_chunk(Chunk *chunk)
{
    free_mesh(chunk->mesh);
    free(chunk);
}