cmake_minimum_required(VERSION 3.8.0)
project(Asphalt VERSION 0.1.0)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${PROJECT_SOURCE_DIR}/res/icon/Asphalt.rc")
else()
    set(APP_ICON_RESOURCE_WINDOWS "")
endif()

set (source
    "${PROJECT_SOURCE_DIR}/src/main.c"
    "${PROJECT_SOURCE_DIR}/src/Chunk.c"
    "${PROJECT_SOURCE_DIR}/src/Chunk.h"
    "${PROJECT_SOURCE_DIR}/src/Mesh.c"
    "${PROJECT_SOURCE_DIR}/src/Mesh.h"
    "${PROJECT_SOURCE_DIR}/src/Util.c"
    "${PROJECT_SOURCE_DIR}/src/Util.h"
    "${PROJECT_SOURCE_DIR}/src/Globals.c"
    "${PROJECT_SOURCE_DIR}/src/Globals.h"
    "${PROJECT_SOURCE_DIR}/src/Texture.c"
    "${PROJECT_SOURCE_DIR}/src/Texture.h"
    "${PROJECT_SOURCE_DIR}/src/Block.c"
    "${PROJECT_SOURCE_DIR}/src/Block.h"
    "${PROJECT_SOURCE_DIR}/src/Sprite.c"
    "${PROJECT_SOURCE_DIR}/src/Sprite.h"
    "${PROJECT_SOURCE_DIR}/src/Input.c"
    "${PROJECT_SOURCE_DIR}/src/Input.h"
    "${PROJECT_SOURCE_DIR}/src/PostProcess.c"
    "${PROJECT_SOURCE_DIR}/src/PostProcess.h"
    "${PROJECT_SOURCE_DIR}/src/TextRenderer.c"
    "${PROJECT_SOURCE_DIR}/src/TextRenderer.h"
    "${PROJECT_SOURCE_DIR}/src/Render.c"
    "${PROJECT_SOURCE_DIR}/src/Render.h"
    "${PROJECT_SOURCE_DIR}/thirdparty/glad/src/glad.c"
    "${APP_ICON_RESOURCE_WINDOWS}"
    )

if (EMSCRIPTEN)
    add_executable(Asphalt ${source})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -O3 -s USE_SDL=2 -s USE_FREETYPE=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s EXIT_RUNTIME=1 -s ASYNCIFY --preload-file ${PROJECT_SOURCE_DIR}/res@/res")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else (EMSCRIPTEN)
    set(SDL_RENDER OFF CACHE BOOL "")
    set(SDL_TIMER TRUE CACHE BOOL "")
    set(SDL_HAPTICS OFF CACHE BOOL "")
    set(SDL_FILESYSTEM OFF CACHE BOOL "")
    set(SDL_SENSOR OFF CACHE BOOL "")
    set(SDL_DLOPEN ON CACHE BOOL "")
    set(SDL_AUDIO OFF CACHE BOOL "")

    set(VIDEO_DUMMY OFF CACHE BOOL "")
    set(DISKAUDIO OFF CACHE BOOL "")
    set(DUMMYAUDIO OFF CACHE BOOL "")
    set(HIDAPI OFF CACHE BOOL "")
    set(VIDEO_OPENGL TRUE CACHE BOOL "")
    set(VIDEO_OPENGLES TRUE CACHE BOOL "")
    set(DIRECTX OFF CACHE BOOL "")
    set(RENDER_D3D OFF CACHE BOOL "")
    set(RENDER_METAL OFF CACHE BOOL "")
    set(VIDEO_VIVANTE OFF CACHE BOOL "")
    set(VIDEO_METAL OFF CACHE BOOL "")
    set(VIDEO_VULKAN OFF CACHE BOOL "")
    set(VIDEO_WAYLAND OFF CACHE BOOL "")

    set(SDL_SHARED_ENABLED_BY_DEFAULT OFF CACHE BOOL "")
    add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/SDL")
    set(SDL2_LIBRARIES "SDL2-static" "SDL2main")
    if (UNIX)
        add_executable(Asphalt ${source})
        target_link_libraries(Asphalt OpenGL dl m)
    elseif (WIN32)
        add_executable(Asphalt WIN32 ${source})
        set_target_properties(Asphalt PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Asphalt)
    endif (UNIX)
    
    add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/freetype")
    target_link_libraries(Asphalt freetype ${SDL2_LIBRARIES} ${CMAKE_DL_LIBS})
endif (EMSCRIPTEN)

target_include_directories(Asphalt PUBLIC
    "${PROJECT_SOURCE_DIR}/thirdparty/cglm/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/glad/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/stb"
    "${PROJECT_SOURCE_DIR}/thirdparty/freetype/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/SDL/include"
    )
