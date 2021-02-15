#include "Util.h"
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

const char* Util_GetFileContent(const char *path, int *length)
{
    FILE* file;
    file = fopen(path, "rb");
    if(!file)
    {
        printf("Failed to open %s", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    if(length)
        *length = fsize;
    fseek(file, 0, SEEK_SET);
    char* content = malloc(fsize + 1);
    fread(content, 1, fsize, file);
    fclose(file);
    content[fsize] = 0;
    return content;
}

unsigned int Util_CompileShader(const char *vertex_path, const char *fragment_path)
{
    int vertex_source_length;
    const char* vertex_source = Util_GetFileContent(vertex_path, &vertex_source_length);
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, &vertex_source_length);
    glCompileShader(vertex_shader);
    free((void*)vertex_source);
    GLint vertex_compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertex_shader, 1024 - 1, &log_length, message);
        printf("Vertex Shader (%s) failed to compile.\n%s\n", vertex_path, message);
    }
    int fragment_source_length;
    const char* fragment_source = Util_GetFileContent(fragment_path, &fragment_source_length);
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, &fragment_source_length);
    glCompileShader(fragment_shader);
    free((void*)fragment_source);
    GLint fragment_compiled;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragment_shader, 1024 - 1, &log_length, message);
        printf("Fragment Shader (%s) failed to compile.\n%s\n", fragment_path, message);
    }
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    GLint program_linked;
    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024 - 1, &log_length, message);
        printf("Failed to link program (%s & %s)\n%s\n", vertex_path, fragment_path, message);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

static int SEED = -1;

// Following copied from https://gist.github.com/nowl/828013

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

static int noise2(int x, int y)
{
    int tmp = hash[(y + SEED) % 256];
    return hash[(tmp + x) % 256];
}

static float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

static float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

static float noise2d(float x, float y)
{
    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int);
    int t = noise2(x_int+1, y_int);
    int u = noise2(x_int, y_int+1);
    int v = noise2(x_int+1, y_int+1);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

// end https://gist.github.com/nowl/828013

float Util_Perlin2D(float x, float y, float freq, int depth)
{
    if(SEED == -1)
        SEED = rand();
    float xa = x*freq;
    float ya = y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}