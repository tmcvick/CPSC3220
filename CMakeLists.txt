cmake_minimum_required(VERSION 3.6)
project(project2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES rwlock.h main.c rwlock.c)
set(CMAKE_CXX_FLAGS -Wall -pthread)
add_executable(project2 ${SOURCE_FILES})