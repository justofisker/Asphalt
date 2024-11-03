#ifndef UTIL_H
#define UTIL_H

const char* Util_GetFileContent(const char *path, int *length);
unsigned int Util_CompileShader(const char *vertex_path, const char *fragment_path);
float Util_Perlin2D(float x, float y, float freq, int depth);
int mod(int a, int b);

typedef float vec3[3];
typedef int ivec3[3];

#define RAYCAST_HITFLAG_FACE_NORTH 0x1
#define RAYCAST_HITFLAG_FACE_EAST 0x2
#define RAYCAST_HITFLAG_FACE_SOUTH 0x4
#define RAYCAST_HITFLAG_FACE_WEST 0x8
#define RAYCAST_HITFLAG_FACE_TOP 0x10
#define RAYCAST_HITFLAG_FACE_BOTTOM 0x20
void Util_RaycastToBlock(vec3 from, vec3 direction, float max_distance, char *hit_flags, ivec3 *result);

#endif // UTIL_H
