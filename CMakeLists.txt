cmake_minimum_required(VERSION 3.14)
project(lab05 LANGUAGES CXX)

add_subdirectory(banking)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
