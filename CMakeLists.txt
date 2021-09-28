cmake_minimum_required(VERSION 3.0.0)
project(Asphalt VERSION 0.1.0)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${PROJECT_SOURCE_DIR}/res/icon/Asphalt.rc")
else()
    set(APP_ICON_RESOURCE_WINDOWS "")
endif()

set (source
    "${PROJECT_SOURCE_DIR}/src/main.c"
    "${PROJECT_SOURCE_DIR}/src/Chunk.c"
    "${PROJECT_SOURCE_DIR}/src/Mesh.c"
    "${PROJECT_SOURCE_DIR}/src/Util.c"
    "${PROJECT_SOURCE_DIR}/src/Globals.c"
    "${PROJECT_SOURCE_DIR}/src/Texture.c"
    "${PROJECT_SOURCE_DIR}/src/Block.c"
    "${PROJECT_SOURCE_DIR}/src/Sprite.c"
    "${PROJECT_SOURCE_DIR}/src/Input.c"
    "${PROJECT_SOURCE_DIR}/src/PostProcess.c"
    "${PROJECT_SOURCE_DIR}/src/TextRenderer.c"
    "${PROJECT_SOURCE_DIR}/src/Render.c"
    "${PROJECT_SOURCE_DIR}/thirdparty/glad/src/glad.c"
    "${APP_ICON_RESOURCE_WINDOWS}"
    )

if (EMSCRIPTEN)
    add_executable(Asphalt ${source})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -O3 -s USE_SDL=2 -s USE_FREETYPE=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s EXIT_RUNTIME=1 -s ASYNCIFY --preload-file ${PROJECT_SOURCE_DIR}/res@/res")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else ()
    set(SDL_RENDER_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
    set(SDL_TIMERS_ENABLED_BY_DEFAULT TRUE CACHE BOOL "")
    set(SDL_HAPTICS_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
    set(SDL_FILESYSTEM_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
    set(SDL_SENSOR_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
    set(SDL_DLOPEN_ENABLED_BY_DEFAULT OFF CACHE BOOL "")

    set(VIDEO_DUMMY OFF CACHE BOOL "")
    set(DISKAUDIO OFF CACHE BOOL "")
    set(DUMMYAUDIO OFF CACHE BOOL "")
    set(HIDAPI OFF CACHE BOOL "")
    set(SSE OFF CACHE BOOL "")
    set(SSE2 OFF CACHE BOOL "")
    set(SSE3 OFF CACHE BOOL "")
    set(VIDEO_OPENGL OFF CACHE BOOL "")
    set(VIDEO_OPENGLES TRUE CACHE BOOL "")
    set(DIRECTX OFF CACHE BOOL "")
    set(RENDER_D3D OFF CACHE BOOL "")
    set(RENDER_METAL OFF CACHE BOOL "")
    set(VIDEO_VIVANTE OFF CACHE BOOL "")
    set(VIDEO_METAL OFF CACHE BOOL "")
    set(VIDEO_VULKAN OFF CACHE BOOL "")

    set(SDL_SHARED_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
    add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/SDL")
    set(SDL2_LIBRARIES "SDL2-static" "SDL2main")
    if (UNIX)
        add_executable(Asphalt ${source})
        target_link_libraries(Asphalt dl m ${SDL2_LIBRARIES})
    elseif (WIN32)
        add_executable(Asphalt WIN32 ${source})
        target_link_libraries(Asphalt ${SDL2_LIBRARIES})
    endif (UNIX)
endif (EMSCRIPTEN)

if (NOT DEFINED EMSCRIPTEN)
    add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/freetype")
    target_link_libraries(Asphalt freetype)
endif ()        

target_include_directories(Asphalt PUBLIC
    "${PROJECT_SOURCE_DIR}/thirdparty/cglm/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/glad/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/stb"
    "${PROJECT_SOURCE_DIR}/thirdparty/freetype/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/SDL2/x86_64-w64-mingw32/include"
    )
