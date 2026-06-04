# 1. Set the minimum version of CMake required to parse this file
cmake_minimum_required(VERSION 3.15)

# 2. Name your project and specify the programming language
project(LabEquipmentControllerTests LANGUAGES CXX)

# 3. Request a standard language specification
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Include directories for headers
include_directories(
    ../../Mathematica/include/
    "C:/Program Files (x86)/LabJack/Drivers"
)

# 5. Find Google Test framework (or install it first)
# Option A: Use system-installed GTest
find_package(GTest REQUIRED)

# 6. Define test executable
add_executable(LabControllerTests
    test_main.cpp
)

# 7. Link against test framework and implementation libraries
target_link_libraries(LabControllerTests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    "C:/Program Files (x86)/LabJack/Drivers/LabJackUD.lib"
)

# 8. Register tests with CTest
enable_testing()
add_test(NAME LabControllerTests COMMAND LabControllerTests)

# 9. Set output directory for test executable
set_target_properties(LabControllerTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)