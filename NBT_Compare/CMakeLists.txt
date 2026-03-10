cmake_minimum_required(VERSION 3.13)
project(NBT_Compare LANGUAGES CXX)

add_executable(nbt_test
    main.cpp
    Terminal.cpp
)

target_link_libraries(nbt_test
    PUBLIC
        ${COMMON_LIBS}
)
