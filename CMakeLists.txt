cmake_minimum_required(VERSION 2.8)
project(things2render)

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
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.2 REQUIRED)

include_directories(${OpenCL_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

add_executable(things2render ${SRC})

target_link_libraries(things2render ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(things2render ${OpenCL_LIBRARY})
target_link_libraries(things2render ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} glfw)
