cmake_minimum_required(VERSION 3.15)
project(whotool CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_executable(whotool
    main.cpp
    who.cpp
    utils.cpp
)

target_link_libraries(whotool PRIVATE advapi32 netapi32 shell32)
