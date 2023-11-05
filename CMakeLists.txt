cmake_minimum_required(VERSION 3.27)

project(2dphysics
    DESCRIPTION "A basic 2d physics engine"
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# source files
set(SRCS
    src/collision/collision.cpp
    src/2dphysics.cpp
    src/events.cpp
    src/painter.cpp
    src/physics.cpp
    src/ui.cpp
)

# executable
add_executable(${PROJECT_NAME} ${SRCS})

### LIBRARIES ###

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# ImGui as static library

set(IMGUI_PATH "submodules/imgui/")

file(GLOB IMGUI_SOURCES ${IMGUI_PATH}*.cpp)
set(IMGUI_BACKENDS ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer2.cpp)
add_library("imgui" STATIC ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

target_include_directories("imgui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends)

### LINKING ###
target_link_libraries(2dphysics ${SDL2_LIBRARIES} imgui)
