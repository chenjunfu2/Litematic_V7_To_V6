cmake_minimum_required(VERSION 3.13)
project(Litematic_V7_To_V6 LANGUAGES CXX)

add_executable(nbt_test
    main.cpp
    Litematic_V7_To_V6.cpp
)

target_link_libraries(nbt_test
    PUBLIC
        ${COMMON_LIBS}
)
