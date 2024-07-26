cmake_minimum_required(VERSION 3.0.0)
project(FairShares VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7.zip
)
FetchContent_MakeAvailable(googletest)

include_directories(include)

file(GLOB AllocSource src/allocator/*.cpp src/utils.cpp)
add_library(alloc ${AllocSource})

add_executable(alloctest test/allocator/allocator_test.cpp)
target_link_libraries(alloctest PRIVATE alloc)
target_link_libraries(alloctest PRIVATE gtest)

add_executable(simtest test/simulator/simulate_test.cpp test/simulator/simulation.cpp)
target_link_libraries(simtest PRIVATE alloc)

include(GoogleTest)
enable_testing()
gtest_discover_tests(alloctest)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
