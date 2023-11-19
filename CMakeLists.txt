cmake_minimum_required(VERSION 3.27)

project(2dphysics
    DESCRIPTION "A basic 2d physics engine"
    VERSION 0.0.1
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# source files
set(SRCS
    src/collision/collision.cpp
    src/dynamics/dynamics.cpp
    src/opengl/shaders.cpp
    src/2dphysics.cpp
    src/events.cpp
    src/painter.cpp
    src/scene.cpp
    src/ui.cpp
)

# executable
add_executable(${PROJECT_NAME} ${SRCS})

### LIBRARIES ###

# Glad
set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/submodules/glad")
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)

glad_add_library(glad_lib STATIC API gl:core=3.2)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# ImGui
set(IMGUI_PATH "submodules/imgui/")

file(GLOB IMGUI_SOURCES ${IMGUI_PATH}*.cpp)
set(IMGUI_BACKENDS ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)
add_library("imgui" STATIC ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

target_include_directories("imgui" PUBLIC  ${IMGUI_PATH} ${IMGUI_PATH}/backends)

### LINKING ###
target_link_libraries(2dphysics
    PUBLIC
    ${SDL2_LIBRARIES}
    imgui
    glad_lib
)
