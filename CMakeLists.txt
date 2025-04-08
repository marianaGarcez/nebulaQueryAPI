cmake_minimum_required(VERSION 3.21)
project(NebulaStreamC++Example)

set(CMAKE_CXX_STANDARD 20)

# Set the path to NebulaStream build directory and source
set(NEBULASTREAM_ROOT "/media/psf/Home/Parallels/nebulastream")
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
