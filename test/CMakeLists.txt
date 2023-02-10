
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(lib/gtest)
add_subdirectory(lib/google-benchmark)

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/*.cpp)
add_executable(unit_test ${TEST_FILES})
target_link_libraries(unit_test PRIVATE kv_hash)
target_link_libraries(unit_test PRIVATE GTest::gtest GTest::gtest_main)

file(GLOB BENCH_FILES ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp)
add_executable(kv_benchmark ${BENCH_FILES})
target_link_libraries(kv_benchmark PRIVATE kv_hash)
target_link_libraries(kv_benchmark PRIVATE benchmark::benchmark)

file(GLOB PROFILE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/profile/*.cpp)
add_executable(kv_profile ${PROFILE_FILES})
target_link_libraries(kv_profile PRIVATE kv_hash)
target_link_libraries(kv_profile PRIVATE benchmark::benchmark)
