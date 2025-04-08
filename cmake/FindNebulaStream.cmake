# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Try to find NebulaStream package configuration first
find_package(NebulaStream CONFIG QUIET)

if(NOT NebulaStream_FOUND)
    # If config mode fails, try module mode
    
    # First try to detect NebulaStream installation in user's home directory
    set(USER_HOME_DIR $ENV{HOME})
    message(STATUS "Looking for NebulaStream in user's home directory: ${USER_HOME_DIR}")
    
    # Try to automatically detect NebulaStream installation
    if(NOT DEFINED NEBULASTREAM_ROOT)
        if(EXISTS "${USER_HOME_DIR}/nebulastream")
            set(NEBULASTREAM_ROOT "${USER_HOME_DIR}/nebulastream")
            message(STATUS "Found NebulaStream installation in user's home directory: ${NEBULASTREAM_ROOT}")
        elseif(EXISTS "/media/psf/Home/Parallels/nebulastream")
            set(NEBULASTREAM_ROOT "/media/psf/Home/Parallels/nebulastream")
            message(STATUS "Found NebulaStream installation in Parallels directory: ${NEBULASTREAM_ROOT}")
        elseif(EXISTS "/opt/nebulastream")
            set(NEBULASTREAM_ROOT "/opt/nebulastream")
            message(STATUS "Found NebulaStream installation in /opt: ${NEBULASTREAM_ROOT}")
        endif()
    endif()

    if(NOT DEFINED NEBULASTREAM_ROOT)
        message(WARNING "Could not automatically detect NebulaStream installation. Please set NEBULASTREAM_ROOT manually.")
    else()
        # Try to auto-detect the build directory
        if(NOT DEFINED NEBULASTREAM_BUILD)
            if(EXISTS "${NEBULASTREAM_ROOT}/build")
                set(NEBULASTREAM_BUILD "${NEBULASTREAM_ROOT}/build")
                message(STATUS "Found NebulaStream build directory: ${NEBULASTREAM_BUILD}")
            endif()
        endif()
    endif()
    
    # Find NebulaStream client library and headers
    find_path(NebulaStream_INCLUDE_DIR
        NAMES API/Query.hpp
        PATHS 
            ${NEBULASTREAM_ROOT}/nes-client/include
            ${NEBULASTREAM_BUILD}/include/nebulastream
            ${USER_HOME_DIR}/nebulastream/nes-client/include
            ${USER_HOME_DIR}/nebulastream/build/include/nebulastream
            /usr/local/include/nebulastream
            /usr/include/nebulastream
            ${CMAKE_PREFIX_PATH}/include/nebulastream
    )
    
    # Find the NebulaStream common headers as well
    find_path(NebulaStream_COMMON_INCLUDE_DIR
        NAMES Common/DataTypes/BasicTypes.hpp
        PATHS 
            ${NEBULASTREAM_ROOT}/nes-common/include
            ${NEBULASTREAM_BUILD}/include/nebulastream
            ${USER_HOME_DIR}/nebulastream/nes-common/include
            ${USER_HOME_DIR}/nebulastream/build/include/nebulastream
            /usr/local/include/nebulastream
            /usr/include/nebulastream
            ${CMAKE_PREFIX_PATH}/include/nebulastream
    )

    find_library(NebulaStream_CLIENT_LIBRARY
        NAMES nes-client libnes-client
        PATHS 
            ${NEBULASTREAM_BUILD}/nes-client
            ${NEBULASTREAM_BUILD}/lib
            ${USER_HOME_DIR}/nebulastream/build/nes-client
            ${USER_HOME_DIR}/nebulastream/build/lib
            /usr/local/lib
            /usr/lib
            ${CMAKE_PREFIX_PATH}/lib
    )
    
    # Find common library as well
    find_library(NebulaStream_COMMON_LIBRARY
        NAMES nes-common libnes-common
        PATHS 
            ${NEBULASTREAM_BUILD}/nes-common
            ${NEBULASTREAM_BUILD}/lib
            ${USER_HOME_DIR}/nebulastream/build/nes-common
            ${USER_HOME_DIR}/nebulastream/build/lib
            /usr/local/lib
            /usr/lib
            ${CMAKE_PREFIX_PATH}/lib
    )

    if(NebulaStream_INCLUDE_DIR)
        message(STATUS "Found NebulaStream include directory: ${NebulaStream_INCLUDE_DIR}")
    else()
        message(WARNING "Could not find NebulaStream include directory. Check your installation or set NEBULASTREAM_ROOT correctly.")
    endif()
    
    if(NebulaStream_COMMON_INCLUDE_DIR)
        message(STATUS "Found NebulaStream common include directory: ${NebulaStream_COMMON_INCLUDE_DIR}")
    else()
        message(WARNING "Could not find NebulaStream common include directory. Check your installation.")
    endif()

    if(NebulaStream_CLIENT_LIBRARY)
        message(STATUS "Found NebulaStream client library: ${NebulaStream_CLIENT_LIBRARY}")
    else()
        message(WARNING "Could not find NebulaStream client library. Check your installation or set NEBULASTREAM_ROOT correctly.")
    endif()
    
    if(NebulaStream_COMMON_LIBRARY)
        message(STATUS "Found NebulaStream common library: ${NebulaStream_COMMON_LIBRARY}")
    else()
        message(WARNING "Could not find NebulaStream common library. This might be needed for some advanced functionality.")
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(NebulaStream
        FOUND_VAR NebulaStream_FOUND
        REQUIRED_VARS
            NebulaStream_CLIENT_LIBRARY
            NebulaStream_INCLUDE_DIR
    )

    if(NebulaStream_FOUND AND NOT TARGET NebulaStream::nes-client)
        add_library(NebulaStream::nes-client SHARED IMPORTED)
        set_target_properties(NebulaStream::nes-client PROPERTIES
            IMPORTED_LOCATION "${NebulaStream_CLIENT_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${NebulaStream_INCLUDE_DIR};${NebulaStream_COMMON_INCLUDE_DIR}"
        )
        message(STATUS "NebulaStream client library imported as NebulaStream::nes-client")
    endif()
endif() 