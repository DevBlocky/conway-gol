cmake_minimum_required(VERSION 3.17)
project(conway_gol C)

set(CMAKE_C_STANDARD 99)

add_executable(conway_gol main.c gol.c gol.h)
