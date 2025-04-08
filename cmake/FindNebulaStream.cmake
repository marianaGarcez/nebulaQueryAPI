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
    
    # Find NebulaStream client library and headers
    find_path(NebulaStream_INCLUDE_DIR
        NAMES API/Query.hpp
        PATHS 
            ${NEBULASTREAM_ROOT}/nes-client/include
            ${NEBULASTREAM_BUILD}/include/nebulastream
            /usr/local/include/nebulastream
            /usr/include/nebulastream
            ${CMAKE_PREFIX_PATH}/include/nebulastream
    )

    find_library(NebulaStream_CLIENT_LIBRARY
        NAMES nes-client libnes-client
        PATHS 
            ${NEBULASTREAM_BUILD}/nes-client
            ${NEBULASTREAM_BUILD}/lib
            /usr/local/lib
            /usr/lib
            ${CMAKE_PREFIX_PATH}/lib
    )

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
            INTERFACE_INCLUDE_DIRECTORIES "${NebulaStream_INCLUDE_DIR}"
        )
        message(STATUS "NebulaStream include dir: ${NebulaStream_INCLUDE_DIR}")
        message(STATUS "NebulaStream client library found at: ${NebulaStream_CLIENT_LIBRARY}")
    endif()
endif() 