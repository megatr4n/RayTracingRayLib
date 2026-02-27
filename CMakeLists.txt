cmake_minimum_required(VERSION 3.16)
project(RealTimeRaytracer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(OIDN_DIR ${CMAKE_SOURCE_DIR}/external/oidn)
include_directories(${OIDN_DIR}/include)
link_directories(${OIDN_DIR}/lib)

set(IMGUI_DIR   ${CMAKE_SOURCE_DIR}/external/imgui)
set(RLIMGUI_DIR ${CMAKE_SOURCE_DIR}/external/rlImGui)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
add_subdirectory(external/raylib)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

add_library(rlimgui STATIC
    ${RLIMGUI_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC ${RLIMGUI_DIR})
target_link_libraries(rlimgui PUBLIC raylib imgui)

add_executable(raytracer
    src/main.cpp
    src/material.cpp
    src/hittable.cpp
    src/camera.cpp
    src/renderer.cpp
    src/ui.cpp
)

target_include_directories(raytracer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(raytracer PRIVATE 
    raylib 
    imgui 
    rlimgui 
    Threads::Threads 
    OpenImageDenoise 
)

if(APPLE)
    find_library(COCOA Cocoa)
    find_library(OGL OpenGL)
    find_library(IOKIT IOKit)
    target_link_libraries(raytracer PRIVATE ${COCOA} ${OGL} ${IOKIT})
    
    add_custom_command(TARGET raytracer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${OIDN_DIR}/lib $<TARGET_FILE_DIR:raytracer>
    )
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()