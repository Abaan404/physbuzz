cmake_minimum_required(VERSION 3.28)

project(physbuzz
    DESCRIPTION "A basic physics engine"
    VERSION 0.0.2
    LANGUAGES C CXX
)

set(CMAKE_BUILD_PARALLEL_LEVEL 3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRCS "src/*.cpp")
add_executable(${PROJECT_NAME} ${SRCS})

# glm
set(GLM_PATH "submodules/glm")
add_subdirectory(${GLM_PATH})

# glad
set(GLAD_PATH "submodules/glad")
add_subdirectory("${GLAD_PATH}/cmake")
glad_add_library(glad STATIC API gl:core=4.6)

# SDL2
set(SDL_PATH "submodules/SDL")
add_subdirectory(${SDL_PATH})

# ImGui
set(IMGUI_PATH "submodules/imgui")
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
    ${SDL_PATH}/include) # for sdl2 backend

# linking (TODO add option() to use shared libs instead)
target_link_libraries(physbuzz PRIVATE
    SDL2-static SDL2main
    imgui
    glad
    glm
)
