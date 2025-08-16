cmake_minimum_required(VERSION 3.28)

project(
  physbuzz
  DESCRIPTION "A basic physics engine"
  VERSION 0.0.3
  LANGUAGES C CXX)

set(CMAKE_BUILD_PARALLEL_LEVEL 12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(Vulkan REQUIRED)

# efsw
set(EFSW_PATH "${CMAKE_SOURCE_DIR}/submodules/efsw")
add_subdirectory(${EFSW_PATH})

# stb
set(STB_PATH "${CMAKE_SOURCE_DIR}/submodules/stb")
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_PATH})

# ImGui
set(IMGUI_PATH "${CMAKE_SOURCE_DIR}/submodules/imgui")
file(GLOB IMGUI_SRCS ${IMGUI_PATH}/*.cpp
     ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
     ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp)

add_library(imgui SHARED ${IMGUI_SRCS})
target_include_directories(imgui PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends
                                        ${GLFW_PATH}/include) # for glfw backend

# physbuzz
set(ENGINE_PATH "libs")
file(GLOB_RECURSE ENGINE_SRCS "${ENGINE_PATH}/*.cpp")
add_library(physbuzz SHARED ${ENGINE_SRCS})
add_compile_definitions(
  physbuzz
  GLFW_INCLUDE_VULKAN # include vulkan in glfw
  GLM_ENABLE_EXPERIMENTAL # enable glm's quat headers
  VULKAN_HPP_NO_STRUCT_CONSTRUCTORS # use struct constructors
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 # vulkan extensions
  VULKAN_HPP_NO_EXCEPTIONS # no throwing hands
)

target_include_directories(physbuzz PUBLIC ${ENGINE_PATH})
target_compile_options(physbuzz PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(
  physbuzz
  PUBLIC assimp
         spdlog::spdlog
         stb
         efsw
         glfw
         Vulkan::Vulkan
         glm)

# driver code
file(GLOB_RECURSE SRCS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(game ${SRCS})

target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(game PRIVATE physbuzz imgui)

# resources
set(RESOURCES_PATH "${CMAKE_SOURCE_DIR}/resources")
add_custom_command(
  TARGET game
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/resources
          $<TARGET_FILE_DIR:game>/resources)

# shaders
set(RESOURCES_PATH "${CMAKE_SOURCE_DIR}/resources")
file(GLOB_RECURSE SHADERS_SRCS ${RESOURCES_PATH}/*.slang)

set(SPIRV_BUILD_DIR "${CMAKE_BINARY_DIR}/spirv")
set(SHADER_OUTPUTS)

foreach(SHADER ${SHADERS_SRCS})
  file(RELATIVE_PATH REL_PATH ${RESOURCES_PATH} ${SHADER})
  set(OUTPUT "${SPIRV_BUILD_DIR}/${REL_PATH}.spv")
  get_filename_component(OUTPUT_DIR ${OUTPUT} DIRECTORY)

  add_custom_command(
    OUTPUT ${OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    COMMAND slangc -target spirv -o ${OUTPUT} ${SHADER}
    DEPENDS ${SHADER}
    COMMENT "Compiling shader ${REL_PATH} to SPIR-V"
    VERBATIM
  )
  list(APPEND SHADER_OUTPUTS ${OUTPUT})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})
add_dependencies(game shaders)

add_custom_command(
  TARGET game
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${SPIRV_BUILD_DIR}
          $<TARGET_FILE_DIR:game>/spirv
  COMMENT "Copying SPIR-V shaders to game output directory"
)
# tests
set(TEST_PATH "tests")
file(GLOB_RECURSE TESTS_SRCS "${TEST_PATH}/*.cpp")
add_executable(tests ${TESTS_SRCS})
add_compile_definitions(tests CATCH_CONFIG_PREFIX_MESSAGES)

target_link_libraries(tests PRIVATE physbuzz Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
