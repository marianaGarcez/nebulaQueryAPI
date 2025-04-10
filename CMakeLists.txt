cmake_minimum_required(VERSION 3.12)
project(NebulaStreamExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Find NebulaStream Installation
if(DEFINED ENV{NEBULASTREAM_ROOT})
    set(NEBULASTREAM_ROOT "$ENV{NEBULASTREAM_ROOT}" CACHE PATH "Root directory of NebulaStream installation, found via environment variable.")
    message(STATUS "Found NebulaStream root via environment variable NEBULASTREAM_ROOT: ${NEBULASTREAM_ROOT}")
else()
    set(NEBULASTREAM_ROOT "" CACHE PATH "Root directory of NebulaStream installation (e.g., /path/to/nebulastream)")
    # Check if the variable was set (either by env var above or by user via -D)
    if(NOT NEBULASTREAM_ROOT)
        message(FATAL_ERROR "NebulaStream root directory not found. Please set the NEBULASTREAM_ROOT environment variable or pass -DNEBULASTREAM_ROOT=/path/to/nebula to CMake.")
    else()
        message(STATUS "Using NebulaStream root from CMake cache: ${NEBULASTREAM_ROOT}")
    endif()
endif()

# Check if the specified directory exists
if(NOT EXISTS "${NEBULASTREAM_ROOT}")
    message(FATAL_ERROR "Specified NEBULASTREAM_ROOT does not exist: ${NEBULASTREAM_ROOT}")
endif()

# Set NebulaStream build path (assuming build is inside root)
set(NEBULASTREAM_BUILD "${NEBULASTREAM_ROOT}/build")
if(NOT EXISTS "${NEBULASTREAM_BUILD}")
     message(FATAL_ERROR "NebulaStream build directory does not exist: ${NEBULASTREAM_BUILD}. Ensure NebulaStream is built in a 'build' subdirectory of NEBULASTREAM_ROOT.")
endif()

message(STATUS "Using NEBULASTREAM_BUILD: ${NEBULASTREAM_BUILD}")

# Include directories
include_directories(
    ${NEBULASTREAM_ROOT}
    ${NEBULASTREAM_ROOT}/nes-client/include
    ${NEBULASTREAM_ROOT}/nes-common/include
    ${NEBULASTREAM_ROOT}/nes-operators/include
    ${NEBULASTREAM_ROOT}/nes-configurations/include
    ${NEBULASTREAM_ROOT}/nes-runtime/include
    ${NEBULASTREAM_ROOT}/nes-data-types/include
    ${NEBULASTREAM_ROOT}/nes-util/include
    ${NEBULASTREAM_ROOT}/nes-core/include
    ${NEBULASTREAM_ROOT}/nes-expressions/include
    ${NEBULASTREAM_ROOT}/nes-window-types/include
    ${NEBULASTREAM_BUILD}/include/nebulastream # Generated headers
)

# Link directories
link_directories(
    ${NEBULASTREAM_BUILD}/nes-client
    ${NEBULASTREAM_BUILD}/nes-common
    ${NEBULASTREAM_BUILD}/nes-operators
    ${NEBULASTREAM_BUILD}/nes-configurations
    ${NEBULASTREAM_BUILD}/nes-runtime
    ${NEBULASTREAM_BUILD}/nes-util
    ${NEBULASTREAM_BUILD}/nes-core
    ${NEBULASTREAM_BUILD}/nes-expressions
    ${NEBULASTREAM_BUILD}/nes-data-types
    ${NEBULASTREAM_BUILD}/nes-window-types
)

# Client executable
add_executable(NebulaStreamC++Example query.cpp)

# Link libraries for client
target_link_libraries(NebulaStreamC++Example PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)

# Compiler definitions
target_compile_definitions(NebulaStreamC++Example PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Print status (already handled by messages above)
# message(STATUS "NEBULASTREAM_ROOT: ${NEBULASTREAM_ROOT}") # Redundant
# message(STATUS "NEBULASTREAM_BUILD: ${NEBULASTREAM_BUILD}") # Redundant