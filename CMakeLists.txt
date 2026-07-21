cmake_minimum_required(VERSION 3.10)
project(BackPipe CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")

add_executable(backpipe 
    main.cpp 
    orchestrator/orchestrator.cpp)

target_include_directories(backpipe PRIVATE include)
target_compile_options(backpipe PRIVATE -O3)
