#include "Globals.h"

vec3 global_camera_rotation;
vec3 global_camera_offset = {0.0f, 1.5f, 0.0f};
vec3 global_player_position;
CameraInfo global_camera_info = {70.f, 0.05f, 2000.f};
mat4 global_view;
mat4 global_projection;
int global_block_model_loc;
int global_block_view_loc;
int global_block_projection_loc;
int global_block_texture_loc;
int global_block_view_near_loc;
int global_block_view_far_loc;
unsigned int global_block_shader;
int global_color_model_loc;
int global_color_view_loc;
int global_color_projection_loc;
int global_color_color_loc;
unsigned int global_color_shader;
Texture *global_texture;
int global_height = 720;
int global_width = 1280;
clock_t global_last_frame;
int global_target_framerate = 165;