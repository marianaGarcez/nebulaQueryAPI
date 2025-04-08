cmake_minimum_required(VERSION 3.21)
project(NebulaStreamC++Example)

set(CMAKE_CXX_STANDARD 20)

# Set the path to NebulaStream build directory and source
# Allow overriding from command line or environment
if(NOT DEFINED NEBULASTREAM_ROOT)
    if(DEFINED ENV{NEBULASTREAM_ROOT})
        set(NEBULASTREAM_ROOT $ENV{NEBULASTREAM_ROOT})
    else()
        # Default path if not specified
        set(NEBULASTREAM_ROOT "/media/psf/Home/Parallels/nebulastream")
        message(STATUS "Using default NEBULASTREAM_ROOT: ${NEBULASTREAM_ROOT}")
        message(STATUS "You can override this by setting -DNEBULASTREAM_ROOT=/path/to/nebulastream")
    endif()
endif()
set(NEBULASTREAM_BUILD "${NEBULASTREAM_ROOT}/build")

# Add path to our custom Find module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find NebulaStream package
find_package(NebulaStream REQUIRED)

# Find spdlog dependency
find_package(spdlog REQUIRED)

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

# Link against NebulaStream libraries
target_link_libraries(NebulaStreamC++Example 
    PRIVATE 
    NebulaStream::nes-client
    ${NebulaStream_OPERATORS_LIBRARY}
    spdlog::spdlog
)

# Add include directories explicitly
target_include_directories(NebulaStreamC++Example
    PRIVATE
    ${NEBULASTREAM_ROOT}/nes-client/include
    ${NEBULASTREAM_BUILD}/include/nebulastream
)
