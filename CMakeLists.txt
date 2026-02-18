cmake_minimum_required(VERSION 3.16)
project(RealTimeRaytracer CXX)

set(CMAKE_CXX_STANDARD 17)

set(RAYLIB_DIR  ${CMAKE_SOURCE_DIR}/external/raylib)
set(IMGUI_DIR   ${CMAKE_SOURCE_DIR}/external/imgui)
set(RLIMGUI_DIR ${CMAKE_SOURCE_DIR}/external/rlImGui)

add_library(raylib STATIC IMPORTED)
if(WIN32)
    set_target_properties(raylib PROPERTIES IMPORTED_LOCATION ${RAYLIB_DIR}/lib/raylib.lib)
else()
    set_target_properties(raylib PROPERTIES IMPORTED_LOCATION ${RAYLIB_DIR}/lib/libraylib.a)
endif()
target_include_directories(raylib INTERFACE ${RAYLIB_DIR}/include)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

add_library(rlimgui STATIC ${RLIMGUI_DIR}/rlImGui.cpp)
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
target_include_directories(raytracer PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(raytracer PRIVATE raylib imgui rlimgui)

if(WIN32)
    target_link_libraries(raytracer PRIVATE winmm opengl32 gdi32)
elseif(APPLE)
    find_library(COCOA Cocoa) find_library(OGL OpenGL) find_library(IOKIT IOKit)
    target_link_libraries(raytracer PRIVATE ${COCOA} ${OGL} ${IOKIT})
else()
    target_link_libraries(raytracer PRIVATE GL dl m pthread)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()