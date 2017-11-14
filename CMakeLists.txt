cmake_minimum_required(VERSION 3.8)
project(mymalloc_c)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES mymalloc.c mallocdrv.c)
add_executable(mymalloc_c ${SOURCE_FILES} mallocdrv.c)