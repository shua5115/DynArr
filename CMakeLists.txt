cmake_minimum_required(VERSION 3.6)

project(DynArr)

add_executable(test test.c)

add_executable(hello_dynarr hello_dynarr.c)

set_property(TARGET hello_dynarr test PROPERTY C_STANDARD 99)