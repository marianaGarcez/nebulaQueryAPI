cmake_minimum_required(VERSION 3.12)
project(NebulaStreamExample)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Set NebulaStream paths - adjust these to match your installation
set(NEBULASTREAM_ROOT "/media/psf/Home/Parallels/nebulastream-legacy")
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
    ${NEBULASTREAM_ROOT}/nes-benchmark/include
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
    ${NEBULASTREAM_BUILD}/nes-benchmark
)

# Client executable for query.cpp
add_executable(QueryTest query.cpp)

# Client executable for query1.cpp
add_executable(Query1 query1.cpp)

# Client executable for query2.cpp
add_executable(Query2 query2.cpp)   

# Client executable for query3.cpp
add_executable(Query3 query3.cpp)

# Client executable for query4.cpp
add_executable(Query4 query4.cpp)


# Client executable for query5.cpp
add_executable(Query5 query5.cpp)

# Client executable for query6.cpp
add_executable(Query6 query6.cpp)

# Client executable for queryCFA.cpp
add_executable(QueryCFA queryCFA.cpp)

# Client executable for queryCFF.cpp
add_executable(QueryCFF queryCFF.cpp)

# Client executable for queryCFAandCFF.cpp
add_executable(QueryCFAandCFF queryCFAandCFF.cpp)

# Client executable for querytrysamewindow.cpp
add_executable(Querytrysamewindow querytrysamewindow.cpp)   

# Client executable for querytrysamewindowOneday.cpp
add_executable(querytrysamewindowOneday querytrysamewindowOneday.cpp)


# Link libraries for the first client
target_link_libraries(QueryTest PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    nes-benchmark
    nes-runtime
    ${CMAKE_THREAD_LIBS_INIT}
)

# Link libraries for the second client
target_link_libraries(Query1 PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)


# Link libraries for the third client
target_link_libraries(Query2 PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)


target_link_libraries(Query3 PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)


target_link_libraries(Query4 PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations  
    nes-window-types
    nes-expressions
    nes-data-types
    nes-execution
    nes-catalogs
    nes-runtime
    nes-benchmark
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(Query5 PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(Query6 PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(QueryCFA PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(QueryCFF PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(QueryCFAandCFF PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)   

target_link_libraries(Querytrysamewindow PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)   

target_compile_definitions(querytrysamewindowOneday PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)
target_link_libraries(querytrysamewindowOneday PRIVATE
    nes-client
    nes-operators
    nes-common
    nes-configurations
    nes-window-types
    ${CMAKE_THREAD_LIBS_INIT}
)


# Compiler definitions for the first client
target_compile_definitions(QueryTest PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Compiler definitions for the second client
target_compile_definitions(Query1 PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)


# Compiler definitions for the third client
target_compile_definitions(Query2 PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)


# Compiler definitions for the fourth client
target_compile_definitions(Query3 PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)


# Compiler definitions for the fifth client
target_compile_definitions(Query4 PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)


# Compiler definitions for the sixth client
target_compile_definitions(Query5 PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Compiler definitions for the seventh client
target_compile_definitions(Query6 PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Compiler definitions for the seventh client
target_compile_definitions(QueryCFA PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Compiler definitions for the eighth client
target_compile_definitions(QueryCFF PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Compiler definitions for the ninth client
target_compile_definitions(QueryCFAandCFF PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Compiler definitions for the tenth client
target_compile_definitions(Querytrysamewindow PRIVATE NES_COMPILE_TIME_LOG_LEVEL=0)

# Print status
message(STATUS "NEBULASTREAM_ROOT: ${NEBULASTREAM_ROOT}")
message(STATUS "NEBULASTREAM_BUILD: ${NEBULASTREAM_BUILD}")
message(STATUS "NEBULASTREAM_INCLUDE_DIR: ${NEBULASTREAM_ROOT}/nes-client/include")
message(STATUS "NEBULASTREAM_BUILD_DIR: ${NEBULASTREAM_BUILD}")
message(STATUS "NEBULASTREAM_LIB_DIR: ${NEBULASTREAM_BUILD}/lib")
message(STATUS "NEBULASTREAM_INCLUDE_DIR: ${NEBULASTREAM_BUILD}/include")
message(STATUS "NEBULASTREAM_LIB_DIR: ${NEBULASTREAM_BUILD}/lib")
message(STATUS "NEBULASTREAM_INCLUDE_DIR: ${NEBULASTREAM_BUILD}/include")
