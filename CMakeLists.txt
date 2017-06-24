cmake_minimum_required(VERSION 2.8.8)

project(unigine_test)

set(CMAKE_CXX_COMPILER g++-4.9)

add_executable(t main.cpp)
target_compile_options(t PRIVATE -std=c++11)