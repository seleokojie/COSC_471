cmake_minimum_required (VERSION 2.8.12)
project (Transformation)


include_directories($ENV{EIGEN3_INCLUDE_DIR})

add_executable (Transformation main.cpp)
