# 1. Set the minimum version of CMake required to parse this file
cmake_minimum_required(VERSION 3.15)

# 2. Name your project and specify the programming language
project(LabEquipmentController LANGUAGES CXX)

# 3. Request a standard language specification
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3b. Enable parallel PDB writes for MSVC (critical for multi-file builds)
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /FS")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /FS")
    string(APPEND CMAKE_CXX_FLAGS " /FS")
endif()

# 4. Set module path to find custom Find*.cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# 5. Find external dependencies using custom Find modules
find_package(Wolfram REQUIRED)
find_package(LabJack REQUIRED)
find_package(Teknic REQUIRED)

# 6. Include directories for project headers
include_directories(
    ../include/
    ${WOLFRAM_INCLUDE_DIRS}
    ${LABJACK_INCLUDE_DIRS}
    ${TEKNIC_INCLUDE_DIRS}
)

# 7. Define your target output as a shared library (DLL) for Mathematica
add_library(LabController SHARED
    ../src/API.cpp
    ../src/Oven5R6900.cpp
    ../src/RTE7.cpp
    ../src/recorder.cpp
    ../src/sm_homer.cpp
    ../src/sm_manual_controller.cpp
)

# 8. Link all external dependencies to the DLL target
target_link_libraries(LabController 
    PRIVATE 
    ${WOLFRAM_LIBRARIES}
    ${LABJACK_LIBRARIES}
    ${TEKNIC_LIBRARIES}
)

# 9. Set output directory for the DLL
set_target_properties(LabController PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)