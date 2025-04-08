cmake_minimum_required(VERSION 3.21)
project(NebulaStreamC++Example)

set(CMAKE_CXX_STANDARD 20)

# Handle NebulaStream paths
# You can override these by:
# 1. Setting environment variables: export NEBULASTREAM_ROOT=/path/to/nebulastream
# 2. Passing to cmake: cmake -DNEBULASTREAM_ROOT=/path/to/nebulastream ..
# 3. The FindNebulaStream.cmake will try to auto-detect installations in common locations

# Set the path to NebulaStream build directory and source
# Allow overriding from command line or environment
if(NOT DEFINED NEBULASTREAM_ROOT)
    if(DEFINED ENV{NEBULASTREAM_ROOT})
        set(NEBULASTREAM_ROOT $ENV{NEBULASTREAM_ROOT})
        message(STATUS "Using NEBULASTREAM_ROOT from environment: ${NEBULASTREAM_ROOT}")
    else()
        # Default path handled in FindNebulaStream.cmake
        message(STATUS "NEBULASTREAM_ROOT not set - will try to auto-detect")
    endif()
endif()

if(DEFINED NEBULASTREAM_ROOT)
    set(NEBULASTREAM_BUILD "${NEBULASTREAM_ROOT}/build")
    message(STATUS "Setting NEBULASTREAM_BUILD to: ${NEBULASTREAM_BUILD}")
endif()

# Add path to our custom Find module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find NebulaStream package
find_package(NebulaStream REQUIRED)

# Try to find spdlog, but make it optional
set(USE_SPDLOG TRUE)
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    # On Ubuntu 22.04+, try libspdlog-dev package
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SPDLOG QUIET spdlog)
    endif()
    
    # If not found via pkg-config either, disable spdlog
    if(NOT SPDLOG_FOUND)
        message(STATUS "spdlog not found. If you need logging, install with:")
        message(STATUS "  Ubuntu/Debian: sudo apt install libspdlog-dev")
        message(STATUS "  Manually: Clone from https://github.com/gabime/spdlog")
        message(STATUS "Disabling spdlog dependency for now...")
        set(USE_SPDLOG FALSE)
    endif()
endif()

# Add executable
add_executable(NebulaStreamC++Example query.cpp)

# Find additional NebulaStream libraries
find_library(NebulaStream_OPERATORS_LIBRARY
    NAMES nes-operators libnes-operators
    PATHS 
        ${NEBULASTREAM_BUILD}/nes-operators
        ${NEBULASTREAM_BUILD}/lib
        /usr/local/lib
        /usr/lib
)

# Find additional NebulaStream libraries
find_library(NebulaStream_COMMON_LIBRARY
    NAMES nes-common libnes-common
    PATHS 
        ${NEBULASTREAM_BUILD}/nes-common
        ${NEBULASTREAM_BUILD}/lib
        /usr/local/lib
        /usr/lib
)

# Link against NebulaStream libraries
target_link_libraries(NebulaStreamC++Example 
    PRIVATE 
    NebulaStream::nes-client
    ${NebulaStream_OPERATORS_LIBRARY}
    ${NebulaStream_COMMON_LIBRARY}
)

if(USE_SPDLOG)
    target_link_libraries(NebulaStreamC++Example PRIVATE spdlog::spdlog)
endif()

# Add include directories explicitly
if(DEFINED NebulaStream_INCLUDE_DIR)
    target_include_directories(NebulaStreamC++Example
        PRIVATE
        ${NebulaStream_INCLUDE_DIR}
        ${NEBULASTREAM_ROOT}/nes-common/include
        ${NEBULASTREAM_BUILD}/include/nebulastream
    )
    message(STATUS "Using include directories:")
    message(STATUS "  - ${NebulaStream_INCLUDE_DIR}")
    message(STATUS "  - ${NEBULASTREAM_ROOT}/nes-common/include")
    message(STATUS "  - ${NEBULASTREAM_BUILD}/include/nebulastream")
else()
    target_include_directories(NebulaStreamC++Example
        PRIVATE
        ${NEBULASTREAM_ROOT}/nes-client/include
        ${NEBULASTREAM_ROOT}/nes-common/include
        ${NEBULASTREAM_BUILD}/include/nebulastream
    )
endif()

# Add compiler definition if spdlog is available
if(USE_SPDLOG)
    target_compile_definitions(NebulaStreamC++Example PRIVATE HAVE_SPDLOG)
endif()
