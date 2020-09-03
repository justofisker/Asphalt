#include "PostProcess.h"
#include "Mesh.h"
#include "GLobals.h"

#include <cglm/cglm.h>
#include <glad/glad.h>

typedef struct _Vertex
{
    vec2 position;
    vec2 uv;
} Vertex;

typedef unsigned char Index;

Vertex verticies[4] = {
    {-1.0f, -1.0f, 0.0f, 0.0f},
    { 1.0f, -1.0f, 1.0f, 0.0f},
    { 1.0f,  1.0f, 1.0f, 1.0f},
    {-1.0f,  1.0f, 0.0f, 1.0f},
};

Index indicies[6] = {
    0, 1, 2,
    2, 3, 0,
};

Mesh *mesh;


unsigned int framebuffer, textureColorbuffer, rbo;

void render_start_postprocess()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST);

}

void render_end_postprocess()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
}

void setup_postprocess()
{
    // Whole Screen Rect
    mesh = malloc(sizeof(Mesh));

    glGenVertexArrays(1, &mesh->array_object);
    glBindVertexArray(mesh->array_object);

    glGenBuffers(1, &mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glGenBuffers(1, &mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

    mesh->index_count = 6;
    mesh->index_type = GL_UNSIGNED_BYTE;

    glBindVertexArray(0);

    // Framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, global_width, global_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, global_width, global_height); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer is not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void do_postprocess(unsigned int shader, unsigned int slot)
{
    glBindVertexArray(mesh->array_object);
    int screentex_loc = glGetUniformLocation(shader, "screen_tex");
    glUniform1i(screentex_loc, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glDrawElements(GL_TRIANGLES, mesh->index_count, mesh->index_type, NULL);
    glBindVertexArray(0);
}