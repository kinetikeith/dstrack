cmake_minimum_required(VERSION 3.12)

project(DSTrack VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED False)

option(BUILD_WAV "Build test CLI program" OFF)

include_directories(include AudioFile)
add_subdirectory(src)

