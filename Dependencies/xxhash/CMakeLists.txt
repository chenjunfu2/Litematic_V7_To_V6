cmake_minimum_required(VERSION 3.13)
project(xxhash LANGUAGES CXX)

set(SOURCES
    xxhash.c
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64|i386|x86)")
    list(APPEND SOURCES xxh_x86dispatch.c)
endif()

add_library(xxhash STATIC ${SOURCES})
