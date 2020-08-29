cmake_minimum_required(VERSION 3.0.0)
project(MinecraftClone VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(MinecraftClone
    "src/main.c"
    "src/stb_image.c"
    "thirdparty/glad/src/glad.c"
    )

add_subdirectory("thirdparty/freeGLUT/freeglut/freeglut")

target_link_libraries(MinecraftClone freeglut_static)

if (UNIX)
    target_link_libraries(MinecraftClone dl)
endif (UNIX)

target_include_directories(MinecraftClone PUBLIC
    "thirdparty/cglm/include"
    "thirdparty/freeGLUT/freeglut/freeglut/include"
    "thirdparty/glad/include"
    "thirdparty/stb"
    )
