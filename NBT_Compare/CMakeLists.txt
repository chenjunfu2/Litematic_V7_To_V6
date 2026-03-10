cmake_minimum_required(VERSION 3.13)
project(NBT_Compare LANGUAGES CXX)

add_executable(NBT_Compare
    main.cpp
    Terminal.cpp
)

target_link_libraries(NBT_Compare
    PUBLIC
        ${COMMON_LIBS}
)
