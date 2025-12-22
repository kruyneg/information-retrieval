cmake_minimum_required(VERSION 3.16)
project(SearchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_compile_options(-Wno-deprecated-declarations)

find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)

include_directories(/usr/include/mongocxx/v_noabi)
include_directories(/usr/include/bsoncxx/v_noabi)

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${PROJECT_INCLUDE_DIR})

enable_testing()

add_subdirectory(src)
