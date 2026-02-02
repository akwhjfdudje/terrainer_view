cmake_minimum_required(VERSION 3.21)
project(TerrainerView LANGUAGES CXX)

# Global configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Compiler warnings
include(cmake/warnings.cmake)

# Third-party libraries

# Path to glad directory
set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/external/glad/")
# Path to glad cmake files
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)

# Specify glad settings
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

target_include_directories(glad_gl_core_33 PUBLIC
    ${GLAD_SOURCES_DIR}/include
)

# GLFW
add_subdirectory(external/glfw)

# GLM (header-only)
add_subdirectory(external/glm)

# ImGui
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw)

file(GLOB_RECURSE TERRAIN_SRC
    src/*.cpp
)

add_executable(terrainer_view ${TERRAIN_SRC})

target_include_directories(terrainer_view PRIVATE src)

target_link_libraries(terrainer_view PRIVATE
    glad_gl_core_33
    glfw
    glm
    imgui
)

enable_warnings(terrainer_view)

if (WIN32)
    target_link_libraries(terrainer_view PRIVATE opengl32)
elseif (APPLE)
    find_library(COCOA Cocoa REQUIRED)
    find_library(OpenGL OpenGL REQUIRED)
    target_link_libraries(terrainer_view PRIVATE ${COCOA} ${OpenGL})
else()
    target_link_libraries(terrainer_view PRIVATE GL X11 pthread dl)
endif()

