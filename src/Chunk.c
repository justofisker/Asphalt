#include "Chunk.h"

#include "Mesh.h"
#include "Globals.h"
#include "Util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

Chunk *create_chunk(int _x, int _y)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    memset(chunk->blocks, 0, sizeof(chunk->blocks));

    chunk->x = _x;
    chunk->y = _y;

    int x,y, z;
    for(x = 0; x < CHUNK_SIZE_XZ; ++x)
    {
        for(z = 0; z < CHUNK_SIZE_XZ; ++z)
        {
            //           V Because we cant use negative numbers (please fix it is very hacky)
            int act_x = 1000000 + CHUNK_SIZE_XZ * _x + x;
            int act_z = 1000000 + CHUNK_SIZE_XZ * _y + z;
            int act_y = max(min(CHUNK_SIZE_Y, perlin2d(act_x, act_z, .03f, 4) * 32), 1);
            chunk->blocks[x][act_y - 1][z] = 1;
            for(y = act_y - 2; y >= 0; y--)
                chunk->blocks[x][y][z]= 2;
            
        }
    }

    return chunk;
}

typedef struct _Vertex
{
    vec3 position;
    vec2 uv;
    vec3 normal;
} Vertex;

typedef unsigned int Index;

void create_vertex_index_buffer(Chunk* chunk, Vertex **_verticies, int *vertex_count, Index **_indicies, int *index_count)
{
    Mesh *mesh = malloc(sizeof(Mesh));

    int face_count = 0;

    int x, y, z, i;
    for(x = 0; x < CHUNK_SIZE_XZ; x++)
    {
        for(y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for(z = 0; z < CHUNK_SIZE_XZ; z++)
            {
                if(chunk->blocks[x][y][z])
                {
                    char north = z + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x, chunk->y + 1) ? get_chunk(chunk->x, chunk->y + 1)->blocks[x][y][0]                 == 0 : 1) : chunk->blocks[x][y][z + 1] == 0;
                    char south = z - 1 ==            -1 ? (get_chunk(chunk->x, chunk->y - 1) ? get_chunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1] == 0 : 1) : chunk->blocks[x][y][z - 1] == 0;
                    char west  = x - 1 ==            -1 ? (get_chunk(chunk->x - 1, chunk->y) ? get_chunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z] == 0 : 1) : chunk->blocks[x - 1][y][z] == 0;
                    char east  = x + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x + 1, chunk->y) ? get_chunk(chunk->x + 1, chunk->y)->blocks[0][y][z]                 == 0 : 1) : chunk->blocks[x + 1][y][z] == 0;
                    
                    char up    = y + 1 ==  CHUNK_SIZE_Y ? 1 : chunk->blocks[x][y + 1][z] == 0;
                    char down  = y - 1 ==            -1 ? 1 : chunk->blocks[x][y - 1][z] == 0;
                    face_count += north;
                    face_count += south;
                    face_count += west;
                    face_count += east;
                    face_count += up;
                    face_count += down;
                }
            }
        }
    }

    *vertex_count = face_count * 4;
    *index_count = face_count * 6;


    int tex_size_x = 128;
    int tex_size_y = 128;
    int cell_size_x = 64;
    int cell_size_y = 64;

    int offset = 0;
    size_t vertex_size = sizeof(Vertex) * 4 * face_count;
    Vertex *verticies = malloc(vertex_size);
    for(x = 0; x < CHUNK_SIZE_XZ; x++)
    {
        for(y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for(z = 0; z < CHUNK_SIZE_XZ; z++)
            {
                if(chunk->blocks[x][y][z])
                {
                    char north = z + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x, chunk->y + 1) ? get_chunk(chunk->x, chunk->y + 1)->blocks[x][y][0]                 == 0 : 1) : chunk->blocks[x][y][z + 1] == 0;
                    char south = z - 1 ==            -1 ? (get_chunk(chunk->x, chunk->y - 1) ? get_chunk(chunk->x, chunk->y - 1)->blocks[x][y][CHUNK_SIZE_XZ - 1] == 0 : 1) : chunk->blocks[x][y][z - 1] == 0;
                    char west  = x - 1 ==            -1 ? (get_chunk(chunk->x - 1, chunk->y) ? get_chunk(chunk->x - 1, chunk->y)->blocks[CHUNK_SIZE_XZ - 1][y][z] == 0 : 1) : chunk->blocks[x - 1][y][z] == 0;
                    char east  = x + 1 == CHUNK_SIZE_XZ ? (get_chunk(chunk->x + 1, chunk->y) ? get_chunk(chunk->x + 1, chunk->y)->blocks[0][y][z]                 == 0 : 1) : chunk->blocks[x + 1][y][z] == 0;

                    char up    = y + 1 ==  CHUNK_SIZE_Y ? 1 : chunk->blocks[x][y + 1][z] == 0;
                    char down  = y - 1 ==            -1 ? 1 : chunk->blocks[x][y - 1][z] == 0;
                    
                    if(north)
                    {
                        int tex_pos_x;
                        int tex_pos_y;
                        if(chunk->blocks[x][y][z] == 1)
                        {
                            tex_pos_x = 1;
                            tex_pos_y = 1;
                        }
                        else
                        {
                            tex_pos_x = 0;
                            tex_pos_y = 0;
                        }
                        verticies[offset + 0] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  0.0f,  0.0f,  1.0f}; // NORTH
                        offset += 4;
                    }
                    if(south)
                    {
                        int tex_pos_x;
                        int tex_pos_y;
                        if(chunk->blocks[x][y][z] == 1)
                        {
                            tex_pos_x = 1;
                            tex_pos_y = 1;
                        }
                        else
                        {
                            tex_pos_x = 0;
                            tex_pos_y = 0;
                        }
                        verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  0.0f,  0.0f, -1.0f}; // SOUTH
                        offset += 4;
                    }
                    if(west)
                    {
                        int tex_pos_x;
                        int tex_pos_y;
                        if(chunk->blocks[x][y][z] == 1)
                        {
                            tex_pos_x = 1;
                            tex_pos_y = 1;
                        }
                        else
                        {
                            tex_pos_x = 0;
                            tex_pos_y = 0;
                        }
                        verticies[offset + 0] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  , -1.0f,  0.0f,  0.0f}; // WEST
                        verticies[offset + 1] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  , -1.0f,  0.0f,  0.0f}; // WEST
                        verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  , -1.0f,  0.0f,  0.0f}; // WEST
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  , -1.0f,  0.0f,  0.0f}; // WEST
                        offset += 4;
                    }
                    if(east)
                    {
                        int tex_pos_x;
                        int tex_pos_y;
                        if(chunk->blocks[x][y][z] == 1)
                        {
                            tex_pos_x = 1;
                            tex_pos_y = 1;
                        }
                        else
                        {
                            tex_pos_x = 0;
                            tex_pos_y = 0;
                        }
                        verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  1.0f,  0.0f,  0.0f}; // EAST
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  1.0f,  0.0f,  0.0f}; // EAST
                        verticies[offset + 2] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  1.0f,  0.0f,  0.0f}; // EAST
                        verticies[offset + 3] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  1.0f,  0.0f,  0.0f}; // EAST
                        offset += 4;
                    }
                    if(up)
                    {
                        int tex_pos_x;
                        int tex_pos_y;
                        if(chunk->blocks[x][y][z] == 1)
                        {
                            tex_pos_x = 0;
                            tex_pos_y = 1;
                        }
                        else
                        {
                            tex_pos_x = 0;
                            tex_pos_y = 0;
                        }
                        verticies[offset + 0] = (Vertex){1.0f + x, 1.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y ,  0.0f,  1.0f,  0.0f}; // UP
                        verticies[offset + 1] = (Vertex){1.0f + x, 1.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y ,  0.0f,  1.0f,  0.0f}; // UP
                        verticies[offset + 2] = (Vertex){0.0f + x, 1.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y ,  0.0f,  1.0f,  0.0f}; // UP
                        verticies[offset + 3] = (Vertex){0.0f + x, 1.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y ,  0.0f,  1.0f,  0.0f}; // UP
                        offset += 4;
                    }
                    if(down)
                    {
                        int tex_pos_x = 0;
                        int tex_pos_y = 0;
                        verticies[offset + 0] = (Vertex){1.0f + x, 0.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        verticies[offset + 1] = (Vertex){1.0f + x, 0.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 1) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        verticies[offset + 2] = (Vertex){0.0f + x, 0.0f + y, 1.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 1) / tex_size_y  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        verticies[offset + 3] = (Vertex){0.0f + x, 0.0f + y, 0.0f + z  ,  (float)cell_size_x * (tex_pos_x + 0) / tex_size_x, (float)cell_size_y * (tex_pos_y + 0) / tex_size_y  ,  0.0f, -1.0f,  0.0f}; // DOWN
                        offset += 4;
                    }
                }
            }
        }
    }

    Index* indicies = malloc(sizeof(Index) * 6 * face_count);
    for(i = 0; i < face_count; ++i)
    {
        indicies[i * 6 + 0] = 0 + 4 * i;
        indicies[i * 6 + 1] = 1 + 4 * i;
        indicies[i * 6 + 2] = 2 + 4 * i;
        indicies[i * 6 + 3] = 2 + 4 * i;
        indicies[i * 6 + 4] = 3 + 4 * i;
        indicies[i * 6 + 5] = 0 + 4 * i;
    }

    *_verticies = verticies;
    *_indicies = indicies;
}

Mesh *create_mesh_from_chunk(Chunk *chunk)
{
    Mesh *mesh = malloc(sizeof(Mesh));

    Vertex *verticies;
    Index *indicies;

    int vertex_count;
    int index_count;

    create_vertex_index_buffer(chunk, &verticies, &vertex_count, &indicies, &index_count);

    glGenVertexArrays(1, &mesh->array_object);
    glBindVertexArray(mesh->array_object);

    glGenBuffers(1, &mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, verticies, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glGenBuffers(1, &mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * index_count, indicies, GL_STATIC_DRAW);

    glBindVertexArray(0);

    mesh->index_count = index_count;
    mesh->index_type = GL_UNSIGNED_INT;

    free(verticies);
    free(indicies);

    return mesh;
}

void regenerate_chunk_mesh(Chunk *chunk)
{
    free_mesh(chunk->mesh);
    chunk->mesh = create_mesh_from_chunk(chunk);
    return;

    //Vertex *verticies;
    //Index *indicies;
    //
    //int vertex_count;
    //int index_count;
    //
    //create_vertex_index_buffer(chunk, &verticies, &vertex_count, &indicies, &index_count);
    //
    //glBindVertexArray(chunk->mesh->array_object);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, verticies, GL_DYNAMIC_DRAW);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * index_count, indicies, GL_DYNAMIC_DRAW);
    //glBindVertexArray(0);
    //
    //free(indicies);
    //free(verticies);
}

void render_chunk(Chunk *chunk)
{  

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){chunk->x * CHUNK_SIZE_XZ, 0.0f, chunk->y * CHUNK_SIZE_XZ});

    glUniformMatrix4fv(global_basic_model_loc, 1, GL_FALSE, model[0]);
    glBindVertexArray(chunk->mesh->array_object);
    glDrawElements(GL_TRIANGLES, chunk->mesh->index_count, chunk->mesh->index_type, (void*)0);
    glBindVertexArray(0);
}

void freeArray(unsigned int **a, int m) {
    int i;
    for (i = 0; i < m; ++i) {
        free(a[i]);
    }
    free(a);
}

void free_chunk(Chunk *chunk)
{
    free_mesh(chunk->mesh);
    free(chunk);
}

#define VIEW_DISTANCE 64
#define CHUNK_ARR_SIZE VIEW_DISTANCE
#define MAX_CHUNKS_PER_FRAME 7
static Chunk *chunks[CHUNK_ARR_SIZE][CHUNK_ARR_SIZE];

Chunk *get_chunk(int x, int y)
{
    Chunk* chunk = chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)];
    if(chunk && chunk->x == x && chunk->y == y)
        return chunk;
    return 0;
}

void set_chunk(int x, int y, Chunk* chunk)
{
    //printf("x: %d y: %d\tc x: %d y: %d\n", x, y, mod(x, CHUNK_ARR_SIZE), mod(y, CHUNK_ARR_SIZE));
    if(chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)])
        free_chunk(chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)]);
    chunks[mod(x, CHUNK_ARR_SIZE)][mod(y, CHUNK_ARR_SIZE)] = chunk;
}

void generate_chunks()
{
    memset(chunks, 0, sizeof(chunks));
    //int x, y;
    //for(x = 0; x < VIEW_DISTANCE; x++)
    //{
    //    for(y = 0; y < VIEW_DISTANCE; y++)
    //    {
    //        set_chunk(x - VIEW_DISTANCE / 2, y - VIEW_DISTANCE / 2, create_chunk(x - VIEW_DISTANCE / 2, y - VIEW_DISTANCE / 2));
    //    }
    //}
    //for(x = 0; x < VIEW_DISTANCE; x++)
    //{
    //    for(y = 0; y < VIEW_DISTANCE; y++)
    //    {
    //        get_chunk(x - VIEW_DISTANCE / 2, y - VIEW_DISTANCE / 2)->mesh = create_mesh_from_chunk(get_chunk(x - VIEW_DISTANCE / 2, y - VIEW_DISTANCE / 2));
    //    }
    //}
}

void render_chunks()
{
    glm_perspective(glm_rad(70.0f), (float)global_width / global_height, 0.01f, 1000.0f, global_projection);
    glm_mat4_identity(global_view);
    mat4 rotation;
    glm_euler_xyz(global_camera_rotation, rotation);
    glm_mul_rot(global_view, rotation, global_view);
    glm_translate(global_view, global_camera_position);

    glUseProgram(global_basic_shader);
    bind_texture(global_texture, 0);
    glUniformMatrix4fv(global_basic_view_loc, 1, GL_FALSE, global_view[0]);
    glUniformMatrix4fv(global_basic_projection_loc, 1, GL_FALSE, global_projection[0]);
    glUniform1i(global_basic_texture_loc, 0);

    int x, y;

    int chunks_generated = 0;
    int cur_x = -global_camera_position[0] / CHUNK_SIZE_XZ;
    int cur_y = -global_camera_position[2] / CHUNK_SIZE_XZ;
    for(x = cur_x - VIEW_DISTANCE / 2; x < cur_x + VIEW_DISTANCE / 2; x++)
    {
        for(y = cur_y - VIEW_DISTANCE / 2; y < cur_y + VIEW_DISTANCE / 2; y++)
        {
            if(chunks_generated >= MAX_CHUNKS_PER_FRAME)
            {
                x = cur_x + VIEW_DISTANCE / 2;
                break;
            }
            if(!get_chunk(x, y))
            {
                Chunk *chunk = create_chunk(x, y);
                set_chunk(x, y, chunk);
                chunk->mesh = create_mesh_from_chunk(chunk);
                chunks_generated++;
            }
        }
    }

    for(x = 0; x < CHUNK_ARR_SIZE; x++)
    {
        for(y = 0; y < CHUNK_ARR_SIZE; y++)
        {
            if(chunks[x][y])
            {
                render_chunk(chunks[x][y]);
            }
        }
    }
}
