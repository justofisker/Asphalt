#ifndef GLOBALS_H
#define GLOBALS_H

#include <cglm/cglm.h>
#include "Texture.h"

extern vec3 global_camera_position;
extern vec3 global_camera_rotation;

extern mat4 global_view;
extern mat4 global_projection;

extern int global_basic_model_loc;
extern int global_basic_view_loc;
extern int global_basic_projection_loc;
extern int global_basic_texture_loc;
extern unsigned int global_basic_shader;
extern Texture *global_texture;

#endif GLOBALS_H