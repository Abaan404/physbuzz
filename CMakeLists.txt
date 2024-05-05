cmake_minimum_required(VERSION 3.28)

project(physbuzz
    DESCRIPTION "A basic physics engine"
    VERSION 0.0.2
    LANGUAGES C CXX
)

set(CMAKE_BUILD_PARALLEL_LEVEL 3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# glm
set(GLM_PATH "submodules/glm")
add_subdirectory(${GLM_PATH})

# glad
set(GLAD_PATH "submodules/glad")
add_subdirectory("${GLAD_PATH}/cmake")
glad_add_library(glad STATIC API gl:core=4.6)

# glfw
set(GLFW_PATH "submodules/glfw")
add_subdirectory(${GLFW_PATH})

# engine
set(ENGINE_PATH "libs")
file(GLOB_RECURSE ENGINE_SRCS "${ENGINE_PATH}/*.cpp")

add_library(libphysbuzz STATIC ${ENGINE_SRCS})
target_include_directories(libphysbuzz PUBLIC 
    ${ENGINE_PATH})

target_link_libraries(libphysbuzz PUBLIC
    glfw
    glad
    glm
)

# ImGui
set(IMGUI_PATH "submodules/imgui")
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
    ${GLFW_PATH}/include) # for glfw backend

# driver code
file(GLOB_RECURSE SRCS "src/*.cpp")
add_executable(physbuzz ${SRCS})

target_link_libraries(physbuzz PRIVATE
    libphysbuzz
    imgui
)

# tests
set(CATCH2_PATH "submodules/Catch2")
set(TEST_PATH "tests")
add_subdirectory(${CATCH2_PATH})

file(GLOB_RECURSE TESTS_SRCS "${TEST_PATH}/*.cpp")
add_executable(tests ${TESTS_SRCS})
target_link_libraries(tests PRIVATE
    libphysbuzz
    Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
