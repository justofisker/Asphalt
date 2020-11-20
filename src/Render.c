#include "Render.h"
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "Globals.h"

static vec3 sky_color = {0.4f, 0.5f, 1.0f};

void render_begin()
{
    glClearColor(sky_color[0], sky_color[1], sky_color[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm_perspective(glm_rad(global_camera_info.fFOVy), (float)global_width / global_height, global_camera_info.fNear, global_camera_info.fFar, global_projection);
    glm_mat4_identity(global_view);
    mat4 rotation;
    glm_euler_xyz(global_camera_rotation, rotation);
    glm_mul_rot(global_view, rotation, global_view);
    glm_translate(global_view, (vec3){-global_player_position[0], -global_player_position[1], -global_player_position[2]});
    glm_translate(global_view, (vec3){-global_camera_offset[0], -global_camera_offset[1], -global_camera_offset[2]});
}

void render_end()
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

    glutSwapBuffers();
    glutPostRedisplay();
}