cmake_minimum_required(VERSION 3.25)
project(BEJparser)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_FLAGS "-Wall -O2 -std=c23")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF) # GNU extensions off

add_executable(BEJparser main.c)