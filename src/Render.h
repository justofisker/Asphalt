#ifndef RENDER_H
#define RENDER_H

typedef float vec3[3];
typedef float vec4[4]; 

void Render_SetupLookBlock();
void Render_RenderLookBlock(int x, int y, int z);
void Render_RenderDebugBox(vec3 base, vec3 size, vec4 color, char depth_test);

void Render_Start();
void Render_End();

void Render_RenderWorld();

void Render_Setup();

#endif //RENDER_H
