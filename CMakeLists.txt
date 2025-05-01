cmake_minimum_required(VERSION 3.28)

project(physbuzz
    DESCRIPTION "A basic physics engine"
    VERSION 0.0.2
    LANGUAGES C CXX
)

set(CMAKE_BUILD_PARALLEL_LEVEL 12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

# glad
set(GLAD_PATH "submodules/glad")
add_subdirectory("${GLAD_PATH}/cmake")
glad_add_library(glad SHARED API gl:core=4.6)

# efsw
set(EFSW_PATH "submodules/efsw")
add_subdirectory(${EFSW_PATH})

# stb
set(STB_PATH "submodules/stb")
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${STB_PATH})

# ImGui
set(IMGUI_PATH "submodules/imgui")
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)

add_library(imgui SHARED ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
    ${GLFW_PATH}/include) # for glfw backend

# physbuzz
set(ENGINE_PATH "libs")
file(GLOB_RECURSE ENGINE_SRCS "${ENGINE_PATH}/*.cpp")
add_library(physbuzz SHARED ${ENGINE_SRCS})
add_compile_definitions(physbuzz
    GLFW_INCLUDE_NONE)

target_include_directories(physbuzz PUBLIC 
    ${ENGINE_PATH})

target_link_libraries(physbuzz PUBLIC
    assimp
    spdlog::spdlog
    stb
    efsw
    glfw
    glad
    glm)

# driver code
file(GLOB_RECURSE SRCS "src/*.cpp")
add_executable(game ${SRCS})

target_link_libraries(game PRIVATE
    physbuzz
    imgui)

# probably not cross-platform?
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:game>/resources
)

# tests
set(TEST_PATH "tests")
file(GLOB_RECURSE TESTS_SRCS "${TEST_PATH}/*.cpp")
add_executable(tests ${TESTS_SRCS})
add_compile_definitions(tests
    CATCH_CONFIG_PREFIX_MESSAGES)

target_link_libraries(tests PRIVATE
    physbuzz
    Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
