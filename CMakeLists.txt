cmake_minimum_required(VERSION 3.16)

project(humpty VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE HUMPTY_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

set(HUMPTY_MAIN_SOURCE src/main.cpp)
set(HUMPTY_LIB_SOURCES ${HUMPTY_SOURCES})
list(FILTER HUMPTY_LIB_SOURCES EXCLUDE REGEX "(.*/)?src/main\\.cpp$")

add_library(humpty_lib ${HUMPTY_LIB_SOURCES})
target_include_directories(humpty_lib PUBLIC src)

add_executable(humpty ${HUMPTY_MAIN_SOURCE})
target_link_libraries(humpty PRIVATE humpty_lib)
target_include_directories(humpty PRIVATE src)

enable_testing()
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(humpty_tests ${TEST_SOURCES})
target_link_libraries(humpty_tests PRIVATE humpty_lib)
target_include_directories(humpty_tests PRIVATE src tests)
add_test(NAME humpty_tests COMMAND humpty_tests)
