cmake_minimum_required(VERSION 3.19)

project(DSTrack VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include AudioFile)
add_subdirectory(src)

