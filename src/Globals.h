#ifndef GLOBALS_H
#define GLOBALS_H

#undef __SSE3__
#include <cglm/cglm.h>
#include <SDL.h>
#include "Texture.h"

#define WINDOW_TITLE "Asphalt"

typedef struct _CameraInfo
{
    float fFOVy;
    float fNear;
    float fFar;
} CameraInfo;

typedef struct _Font Font;

extern vec3 g_player_position;
extern vec3 g_camera_offset;
extern vec3 g_camera_rotation;
extern CameraInfo g_camera_info;

extern mat4 g_view;
extern mat4 g_projection;

extern int g_block_model_loc;
extern int g_block_view_loc;
extern int g_block_projection_loc;
extern int g_block_texture_loc;
extern int g_block_view_near_loc;
extern int g_block_view_far_loc;
extern unsigned int g_block_shader;
extern int g_color_model_loc;
extern int g_color_view_loc;
extern int g_color_projection_loc;
extern int g_color_color_loc;
extern unsigned int g_color_shader;
extern unsigned int g_postprocess_shader;
extern Texture *g_texture;
extern SDL_Window *g_window;

extern char g_looking_at_block;
extern ivec3 g_look_block_pos;
extern Font *g_font_arial;
extern unsigned int g_fps;
extern int g_block_selected;

extern int g_height;
extern int g_width;

extern int g_target_framerate;
extern int g_chunks_drawn;

#endif // GLOBALS_H