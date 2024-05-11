cmake_minimum_required(VERSION 3.26)
project(ExtendibleHashing)

set(CMAKE_CXX_STANDARD 17)

add_executable(ExtendibleHashing main.cpp
        ExtendibleHashing.h
        ExtendibleHashing.cpp)
