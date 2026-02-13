cmake_minimum_required(VERSION 3.15)
project(Ledger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use vendored doctest header from vendors directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendors)

# Main executable - tests
add_executable(ledger_tests tests.cpp)

# Include Ledger header
target_include_directories(ledger_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Enable CTest integration
enable_testing()
add_test(NAME ledger_tests COMMAND ledger_tests)
