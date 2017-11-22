cmake_minimum_required(VERSION 2.8)
project(things2render)

set (CMAKE_C_COMPILER "gcc")
set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS "-std=c++14 -Wall -Werror -Wextra -pedantic -Wno-unused-parameter")


if (DEBUG)
    message("Debug mod")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
else (RELEASE)
    message("Release mod")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -Ofast -msse2 -ffast-math")
endif (DEBUG)

include_directories(src)
add_subdirectory(src)

find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

add_executable(things2render ${SRC})

target_link_libraries(things2render ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(things2render ${SDL2_LIBRARIES})
