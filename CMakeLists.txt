cmake_minimum_required(VERSION 3.16)

project(humpty VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE HUMPTY_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(humpty ${HUMPTY_SOURCES})

target_include_directories(humpty PRIVATE src)
