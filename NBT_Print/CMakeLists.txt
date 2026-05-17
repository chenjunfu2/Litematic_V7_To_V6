cmake_minimum_required(VERSION 3.13)
project(NBT_Print LANGUAGES CXX)

add_executable(NBT_Print
    main.cpp
)

target_link_libraries(NBT_Print
    PUBLIC
        ${COMMON_LIBS}
)
