#include "Globals.h"

vec3 g_camera_rotation;
vec3 g_camera_offset = {0.0f, 1.5f, 0.0f};
vec3 g_player_position;
CameraInfo g_camera_info = {70.f, 0.05f, 2000.f};
mat4 g_view;
mat4 g_projection;
int g_block_model_loc;
int g_block_view_loc;
int g_block_projection_loc;
int g_block_texture_loc;
int g_block_view_near_loc;
int g_block_view_far_loc;
unsigned int g_block_shader;
int g_color_model_loc;
int g_color_view_loc;
int g_color_projection_loc;
int g_color_color_loc;
int g_color_size_loc;
unsigned int g_color_shader;
unsigned int g_postprocess_shader;
Texture *g_texture;
SDL_Window *g_window;
int g_height = 720;
int g_width = 1280;
int g_target_framerate = 165;

char g_looking_at_block;
ivec3 g_look_block_pos;
Font *g_font_arial;
unsigned int g_fps = 0;
int g_block_selected = 1;
int g_chunks_drawn = 0;
char g_draw_aabb_debug = 0;