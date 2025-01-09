CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
project(Server-Framework)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code)
set(target "test")
set(LIB_SRC 
    code/log.cc
    )
set(Test tests/main.cc)
add_executable(${target} ${LIB_SRC} ${Test})
