cmake_minimum_required(VERSION 3.12)
project(NebulaStreamExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Set NebulaStream paths - adjust these to match your installation
set(NEBULASTREAM_ROOT "/media/psf/Home/Parallels/nebulastream")
set(NEBULASTREAM_BUILD "${NEBULASTREAM_ROOT}/build")

# Include directories
include_directories(
    ${NEBULASTREAM_ROOT}
    ${NEBULASTREAM_ROOT}/nes-client/include
    ${NEBULASTREAM_ROOT}/nes-catalogs/include
    ${NEBULASTREAM_ROOT}/nes-common/include
    ${NEBULASTREAM_ROOT}/nes-operators/include
    ${NEBULASTREAM_ROOT}/nes-configurations/include
    ${NEBULASTREAM_ROOT}/nes-runtime/include
    ${NEBULASTREAM_ROOT}/nes-data-types/include
    ${NEBULASTREAM_ROOT}/nes-util/include
    ${NEBULASTREAM_ROOT}/nes-core/include
    ${NEBULASTREAM_ROOT}/nes-expressions/include
    ${NEBULASTREAM_ROOT}/nes-execution/include
    ${NEBULASTREAM_ROOT}/nes-window-types/include
    ${NEBULASTREAM_BUILD}/include/nebulastream
)

# Link directories
link_directories(
    ${NEBULASTREAM_BUILD}/nes-client
    ${NEBULASTREAM_BUILD}/nes-catalogs
    ${NEBULASTREAM_BUILD}/nes-common
    ${NEBULASTREAM_BUILD}/nes-operators
    ${NEBULASTREAM_BUILD}/nes-configurations
    ${NEBULASTREAM_BUILD}/nes-runtime
    ${NEBULASTREAM_BUILD}/nes-util
    ${NEBULASTREAM_BUILD}/nes-core
    ${NEBULASTREAM_BUILD}/nes-expressions
    ${NEBULASTREAM_BUILD}/nes-execution
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

# Print status
message(STATUS "NEBULASTREAM_ROOT: ${NEBULASTREAM_ROOT}")
message(STATUS "NEBULASTREAM_BUILD: ${NEBULASTREAM_BUILD}")