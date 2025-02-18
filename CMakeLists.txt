CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
project(Server-Framework)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code)

set(CMAKE_BUILD_TYPE Debug)

set(target "test")

set(LIB_SRC 

    code/log.cc
    code/util.cc
    code/thread.cc
    code/schedule.cc
    code/timer.cc
    code/iomanager.cc
    code/awaiter.cc
    code/address.cc
    code/buffer.cc
    code/tcp_server.cc
    code/socket.cc
    )
set(Test tests/main.cc)


add_executable(${target} ${LIB_SRC} ${Test})
target_link_libraries(${target} yaml-cpp pthread  )
