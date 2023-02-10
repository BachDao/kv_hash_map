cmake_minimum_required(VERSION 3.24)
project(kv_hash)

set(CMAKE_CXX_STANDARD 20)
file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(kv_hash ${SRC_FILES})
target_include_directories(kv_hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(test)