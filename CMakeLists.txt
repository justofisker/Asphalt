cmake_minimum_required(VERSION 3.0.0)
project(Asphalt VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(Asphalt
    "src/main.c"
    "src/Chunk.c"
    "src/Mesh.c"
    "src/Util.c"
    "src/Globals.c"
    "src/Texture.c"
    "src/stb_image.c"
    "src/Block.c"
    "src/Sprite.c"
    "src/Input.c"
    "thirdparty/glad/src/glad.c"
    )

add_subdirectory("thirdparty/freeGLUT/freeglut/freeglut")

target_link_libraries(Asphalt freeglut_static)

if (UNIX)
    target_link_libraries(Asphalt dl)
endif (UNIX)

target_include_directories(Asphalt PUBLIC
    "thirdparty/cglm/include"
    "thirdparty/freeGLUT/freeglut/freeglut/include"
    "thirdparty/glad/include"
    "thirdparty/stb"
    )
