cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_COMPILER clang-20)
set(CMAKE_CXX_COMPILER clang++-20)

project(domeykite)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# libc++ required for C++20 std module with Clang
add_compile_options(-stdlib=libc++)
add_link_options(-stdlib=libc++)

find_package(Vulkan REQUIRED)

# C++20 std module
set(LIBCXX_MODULE_DIR "/usr/lib/llvm-20/share/libc++/v1")
add_library(std_module)
target_compile_options(std_module PRIVATE -Wno-reserved-module-identifier)
target_sources(std_module PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ${LIBCXX_MODULE_DIR}
    FILES ${LIBCXX_MODULE_DIR}/std.cppm
)

# Vulkan C++20 module
add_library(vulkan_module)
target_sources(vulkan_module PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ${Vulkan_INCLUDE_DIR}
    FILES
      ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
      ${Vulkan_INCLUDE_DIR}/vulkan/vulkan_video.cppm
)

target_compile_definitions(vulkan_module PUBLIC
  VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
  VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING
)

target_include_directories(vulkan_module PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(vulkan_module PUBLIC std_module Vulkan::Vulkan)

# HelloTriangleApp module
add_library(hello_triangle_app)
target_sources(hello_triangle_app PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES hello_triangle_app.cppm
)
target_link_libraries(hello_triangle_app PUBLIC vulkan_module)

# Executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE hello_triangle_app)
