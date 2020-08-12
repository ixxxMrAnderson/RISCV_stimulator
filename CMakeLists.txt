cmake_minimum_required(VERSION 3.15)
project(RICV)

set(CMAKE_CXX_STANDARD 14)

add_executable(RICV bin.h READIN.h PIPELINE.h main.cpp)
