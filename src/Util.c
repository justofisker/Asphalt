#include "Util.h"
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

const char* get_file_content(const char *path, long *length)
{
    FILE* file;
    errno_t err = fopen_s(&file, path, "rb");
    if(!file || err)
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

unsigned int compile_shader(const char *vertex_path, const char *fragment_path)
{
    long vertex_source_length;
    const char* vertex_source = get_file_content(vertex_path, &vertex_source_length);
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
    long fragment_source_length;
    const char* fragment_source = get_file_content(fragment_path, &fragment_source_length);
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
