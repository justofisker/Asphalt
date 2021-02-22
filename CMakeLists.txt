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
    "${PROJECT_SOURCE_DIR}/src/stb_image.c"
    "${PROJECT_SOURCE_DIR}/src/Block.c"
    "${PROJECT_SOURCE_DIR}/src/Sprite.c"
    "${PROJECT_SOURCE_DIR}/src/Input.c"
    "${PROJECT_SOURCE_DIR}/src/PostProcess.c"
    "${PROJECT_SOURCE_DIR}/src/TextRenderer.c"
    "${PROJECT_SOURCE_DIR}/src/Render.c"
    "${PROJECT_SOURCE_DIR}/thirdparty/glad/src/glad.c"
    "${APP_ICON_RESOURCE_WINDOWS}"
    )

if (UNIX)
    add_executable(Asphalt ${source})
    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
    find_package(SDL2 REQUIRED)
    target_link_libraries(Asphalt dl m ${SDL2_LIBRARY})
elseif (WIN32)
    add_executable(Asphalt WIN32 ${source})
    if (MSVC)
        target_link_libraries(Asphalt "${PROJECT_SOURCE_DIR}/thirdparty/SDL2/x86_64-VS/SDL2.lib" "${PROJECT_SOURCE_DIR}/thirdparty/SDL2/x86_64-VS/SDL2main.lib")
    else (MSVC)
        target_link_libraries(Asphalt "${PROJECT_SOURCE_DIR}/thirdparty/SDL2/x86_64-w64-mingw32/lib/libSDL2.dll.a")
        add_compile_definitions(SDL_MAIN_HANDLED)
    endif (MSVC)
    add_custom_command(TARGET Asphalt POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/x86_64-w64-mingw32/bin/SDL2.dll $<TARGET_FILE_DIR:Asphalt>/SDL2.dll)
endif (UNIX)

add_custom_command(TARGET Asphalt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:Asphalt>/res/)


add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/freetype")
target_link_libraries(Asphalt freetype)
        

target_include_directories(Asphalt PUBLIC
    "${PROJECT_SOURCE_DIR}/thirdparty/cglm/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/glad/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/stb"
    "${PROJECT_SOURCE_DIR}/thirdparty/freetype/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/SDL2/x86_64-w64-mingw32/include"
    )
