cmake_minimum_required(VERSION 2.8)
project(pathtracer)

set (CMAKE_C_COMPILER "clang")
set (CMAKE_CXX_COMPILER "clang++")
set (CMAKE_CXX_FLAGS "-std=c++14 -Wall -Werror -Wextra -pedantic -Wno-unused-parameter")


if (DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
endif (DEBUG)

include_directories(src)
add_subdirectory(src)

find_package(Threads REQUIRED)
find_package(OpenCL REQUIRED)

include_directories(${OpenCL_INCLUDE_DIRS})

add_executable(pathtracer ${SRC})

target_link_libraries(pathtracer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pathtracer ${OpenCL_LIBRARY})
