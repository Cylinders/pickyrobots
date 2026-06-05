cmake_minimum_required(VERSION 3.20)
project(ccpd C)

add_library(solver SHARED
    src/solver.c
)

target_include_directories(solver
    PUBLIC include
)