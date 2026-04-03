cmake_minimum_required(VERSION 3.13)
project(Litematic_V7_To_V6 LANGUAGES CXX)

add_executable(Litematic_V7_To_V6
    main.cpp
    Litematic_V7_To_V6.cpp
)

set_release_static_link(Litematic_V7_To_V6)

target_link_libraries(Litematic_V7_To_V6
    PUBLIC
        ${COMMON_LIBS}
)
