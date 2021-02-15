#ifndef UTIL_H
#define UTIL_H

const char* Util_GetFileContent(const char *path, int *length);
unsigned int Util_CompileShader(const char *vertex_path, const char *fragment_path);
float Util_Perlin2D(float x, float y, float freq, int depth);
int mod(int a, int b);

#endif // UTIL_H
