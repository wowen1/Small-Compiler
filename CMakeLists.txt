cmake_minimum_required(VERSION 2.8.4)
project(cse340p4)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(cse340p4
        compiler.cc
        compiler.h
        demo.cc
        inputbuf.cc
        inputbuf.h
        lexer.cc
        lexer.h
        myfile.cpp)
