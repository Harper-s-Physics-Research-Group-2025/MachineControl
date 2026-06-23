# 1. Set the minimum version of CMake required to parse this file
cmake_minimum_required(VERSION 3.15)

# 2. Name your project and specify the programming language
project(WolframLabEquipmentController LANGUAGES CXX VERSION 0.0)

# 3. Request a standard language specification
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3b. Enable parallel PDB writes for MSVC (critical for multi-file builds)
if(MSVC)
    add_compile_options("/FS")
endif()


# 5. Find external dependencies using custom Find modules
# find_package(Wolfram REQUIRED)
# find_package(LabJack REQUIRED)
# find_package(Teknic REQUIRED)

find_path(WOLFRAM_INCLUDE_DIR 
    NAMES WolframLibrary.h 
    HINTS "C:/Program Files/Wolfram Research/Wolfram/14.3/SystemFiles/IncludeFiles/C"
)


find_path(LABJACK_INCLUDE_DIR
    NAMES LabJackUD.h
    HINTS "C:/Program Files (x86)/LabJack/Drivers"
)
find_library(LABJACK_LIB 
    NAMES LabJackUD.lib 
    HINTS "C:/Program Files (x86)/LabJack/Drivers/64bit"
)


find_path(TEKNIC_INCLUDE_DIR 
    NAMES pubSysCls.h 
    HINTS "C:/Program Files (x86)/Teknic/ClearView/sdk/inc"
)
find_library(TEKNIC_LIB 
    NAMES sFoundation20.lib
    HINTS "C:/Program Files (x86)/Teknic/ClearView/sdk/lib/win/Release/x64"
)
find_file(TEKNIC_RUNTIME_DLL
    NAMES sFoundation20.dll
    HINTS "C:/Program Files (x86)/Teknic/ClearView/sdk/lib/win/Release/x64"
)

# 6. Define output library "EquipmentController" and source files
add_library(wolfram_machine_controller SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/src/wolfram_api.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lab.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Oven5R6900.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RTE7.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/recorder.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/sm_homer.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/sm_manual_controller.cpp"
)


# 7. Include directories for project headers
target_include_directories(wolfram_machine_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${WOLFRAM_INCLUDE_DIR}
        ${LABJACK_INCLUDE_DIR}
        ${TEKNIC_INCLUDE_DIR}
)


# 8. Link all external dependencies to the DLL target
target_link_libraries(wolfram_machine_controller
    PRIVATE 
    ${LABJACK_LIB}
    ${TEKNIC_LIB}
)


# # 9. Set output directory for the DLL
# set_target_properties(LabController PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )


## Name .dll directory based on operating system type
if(WIN32)
    set(WOLFRAM_OS_DIR "Windows-x86-64")
elseif(APPLE)
    # Check for Apple Silicon vs Intel
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(WOLFRAM_OS_DIR "MacOSX-ARM64")
    else()
        set(WOLFRAM_OS_DIR "MacOSX-x86-64")
    endif()
else()
    set(WOLFRAM_OS_DIR "Linux-x86-64")
endif()

# Define the absolute target directory inside your tracked paclet structure
set(PACLET_DEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/WolframMachineControl/LibraryResources/${WOLFRAM_OS_DIR}")


## Copy library into the Wolfram paclet folder
add_custom_command(TARGET wolfram_machine_controller POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PACLET_DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:wolfram_machine_controller>" "${PACLET_DEST_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${TEKNIC_RUNTIME_DLL}" "${PACLET_DEST_DIR}/"
    COMMENT "Copying compiled binary and sFoundation20.dll to Wolfram Paclet directory..."
)