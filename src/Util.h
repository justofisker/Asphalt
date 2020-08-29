#ifndef UTIL_H
#define UTIL_H

const char* get_file_content(const char *path, long *length);
unsigned int compile_shader(const char *vertex_path, const char *fragment_path);

#endif // UTIL_H
