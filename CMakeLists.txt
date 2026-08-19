cmake_minimum_required(VERSION 3.10)
project(BackPipe CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    message(STATUS "Building for ARM64")
    set(SSE2_FLAG "")
else()
    message(STATUS "Building for x86")
    set(SSE2_FLAG "-msse2")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSE2_FLAG}")
set(CMAKE_EXE_LINKER_FLAGS "-static")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(backpipe 
    src/main.cpp 
    src/orchestrator/orchestrator.cpp
    src/logs/logs.cpp)

target_include_directories(backpipe PRIVATE include)
target_compile_options(backpipe PRIVATE -O2)
