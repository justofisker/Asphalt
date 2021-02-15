#include "Sprite.h"
#include "Util.h"
#include "Texture.h"
#include <stdlib.h>
#include <glad/glad.h>
#include <cglm/cglm.h>

static int initialized = 0;
static unsigned int vertex_array_object, vertex_buffer, index_buffer, shader_program;
static unsigned int model_loc, view_loc, projection_loc, texture_loc;

typedef struct Vertex {
    float x, y;
    float tex_x, tex_y;
} Vertex;

static Vertex verticies[4] = {
    {-1.f, -1.f  ,  0.f, 0.f},
    { 1.f, -1.f  ,  1.f, 0.f},
    { 1.f,  1.f  ,  1.f, 1.f},
    {-1.f,  1.f  ,  0.f, 1.f},
};

typedef unsigned char Index;

static Index indicies[6] = {
    0, 1, 2,
    2, 3, 0,
};

Sprite* Sprite_CreateSprite(Texture *texture)
{
    Sprite *sprite = malloc(sizeof(Sprite));

    if(!initialized)
    {
        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), &verticies, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_x));

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), &indicies, GL_STATIC_DRAW);

        shader_program = Util_CompileShader("res/shader/sprite_vertex.glsl", "res/shader/sprite_fragment.glsl");

        model_loc = glGetUniformLocation(shader_program, "u_Model");
        view_loc = glGetUniformLocation(shader_program, "u_View");
        projection_loc = glGetUniformLocation(shader_program, "u_Projection");
        texture_loc = glGetUniformLocation(shader_program, "u_Texture");

        initialized = 1;
    }

    sprite->texture = texture;
    sprite->position[0] = 0.f;
    sprite->position[1] = 0.f;
    sprite->scale[0] = 1.f;
    sprite->scale[1] = 1.f;
    sprite->rotation = 0.f;

    return sprite;
}

void Sprite_DrawSprite(Sprite* sprite)
{
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glBindVertexArray(vertex_array_object);
    glUseProgram(shader_program);

    Texture_Bind(sprite->texture, 0);
    glUniform1i(texture_loc, 0);

    mat4 proj = GLM_MAT4_IDENTITY_INIT;
    glm_ortho((float)viewport[0], (float)viewport[2], (float)viewport[1], (float)viewport[3], 0.f, 1.f, proj);
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){sprite->position[0] - sprite->texture->width / 2.f * sprite->scale[0], sprite->position[1] - sprite->texture->height / 2.f * sprite->scale[1], 0.f});
    glm_scale(model, (vec3){sprite->scale[0], sprite->scale[1], 1.f});
    glm_rotate_z(model, sprite->rotation, model);
    glm_scale(model, (vec3){(float)sprite->texture->width, (float)sprite->texture->height, 0.f});

    glUniformMatrix4fv(model_loc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, GLM_MAT4_IDENTITY[0]);
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, proj[0]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);
    
    glBindVertexArray(0);
}
