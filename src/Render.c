#include "Render.h"
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "Globals.h"
#include "Mesh.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

static vec3 sky_color = {0.4f, 0.5f, 1.0f};
static Mesh* look_block_mesh;

void Render_SetupLookBlock()
{
    typedef struct _Vertex
    {
        vec3 position;
    } Vertex;

    typedef unsigned short Index;

    Vertex verticies[8] = {
        { 1.0f,  1.0f, -0.0f},
        { 1.0f, -0.0f, -0.0f},
        {-0.0f, -0.0f, -0.0f},
        {-0.0f,  1.0f, -0.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f, -0.0f,  1.0f},
        {-0.0f, -0.0f,  1.0f},
        {-0.0f,  1.0f,  1.0f},
    };

    Index indicies[36] = {
        2, 1, 0,
        0, 3, 2,
        1, 5, 4,
        4, 0, 1,
        4, 5, 6,
        6, 7, 4,
        7, 6, 2,
        2, 3, 7,
        6, 5, 1,
        1, 2, 6,
        0, 4, 7,
        7, 3, 0,
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VB, IB;
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
    glBindVertexArray(0);

    Mesh* mesh = malloc(sizeof(Mesh));
    mesh->array_object = VAO;
    mesh->vertex_buffer = VB;
    mesh->index_buffer = IB;
    mesh->index_count = 36;
    mesh->index_type = GL_UNSIGNED_SHORT;

    look_block_mesh = mesh;
}

void Render_RenderLookBlock(int x, int y, int z)
{
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){x  - g_player_position[0], y - g_player_position[1], z - g_player_position[2]});
    glUseProgram(g_color_shader);
    glUniformMatrix4fv(g_color_model_loc, 1, GL_FALSE, model[0]);
    glUniformMatrix4fv(g_color_view_loc, 1, GL_FALSE, g_view[0]);
    glUniformMatrix4fv(g_color_projection_loc, 1, GL_FALSE, g_projection[0]);
    glUniform4f(g_color_color_loc, 0.0f, 0.0f, 0.0f, 1.0f);
    glBindVertexArray(look_block_mesh->array_object);
    glLineWidth(1.5f);
    glDrawElements(GL_LINES, look_block_mesh->index_count, look_block_mesh->index_type, (void*)0);
    glBindVertexArray(0);
}

void Render_Start()
{
    glClearColor(sky_color[0], sky_color[1], sky_color[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm_perspective(glm_rad(g_camera_info.fFOVy), (float)g_width / g_height, g_camera_info.fNear, g_camera_info.fFar, g_projection);
    glm_mat4_identity(g_view);
    mat4 rotation;
    glm_euler_xyz(g_camera_rotation, rotation);
    glm_mul_rot(g_view, rotation, g_view);
    glm_translate(g_view, (vec3){-g_camera_offset[0], -g_camera_offset[1], -g_camera_offset[2]});
}

void Render_End()
{
    GLenum error = glGetError();
    while(error != GL_NO_ERROR)
    {
        char *error_name;
        switch(error) {
            case GL_INVALID_OPERATION:              error_name="INVALID_OPERATION";              break;
            case GL_INVALID_ENUM:                   error_name="INVALID_ENUM";                   break;
            case GL_INVALID_VALUE:                  error_name="INVALID_VALUE";                  break;
            case GL_OUT_OF_MEMORY:                  error_name="OUT_OF_MEMORY";                  break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error_name="INVALID_FRAMEBUFFER_OPERATION";  break;
            default:                                error_name="UNKNOWN ERROR";                  break;
        }
        printf("OpenGL Error: %s!\n", error_name);
        error = glGetError();
    }
    
    SDL_GL_SwapWindow(g_window);
}

void Render_Setup()
{
    g_block_shader = Util_CompileShader("res/shader/block_vertex.glsl", "res/shader/block_fragment.glsl");
    g_postprocess_shader = Util_CompileShader("res/shader/postprocess_vertex.glsl", "res/shader/postprocess_fragment.glsl");
    g_color_shader = Util_CompileShader("res/shader/color_vertex.glsl", "res/shader/color_fragment.glsl");
    g_texture = Texture_CreateTexture("res/texture/blocks.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 0, 0.0f);

    g_block_model_loc = glGetUniformLocation(g_block_shader, "u_Model");
    g_block_view_loc = glGetUniformLocation(g_block_shader, "u_View");
    g_block_projection_loc = glGetUniformLocation(g_block_shader, "u_Projection");
    g_block_texture_loc = glGetUniformLocation(g_block_shader, "u_Texture");
    g_block_view_near_loc = glGetUniformLocation(g_block_shader, "u_ViewNear");
    g_block_view_far_loc = glGetUniformLocation(g_block_shader, "u_ViewFar");
    g_color_model_loc = glGetUniformLocation(g_color_shader, "u_Model");
    g_color_view_loc = glGetUniformLocation(g_color_shader, "u_View");
    g_color_projection_loc = glGetUniformLocation(g_color_shader, "u_Projection");
    g_color_color_loc = glGetUniformLocation(g_color_shader, "u_Color");

    glm_vec3_zero(g_player_position);
    g_player_position[1] = 255.0f;
    glm_vec3_zero(g_camera_rotation);
    glm_mat4_identity(g_view);
    glm_mat4_identity(g_projection);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}
