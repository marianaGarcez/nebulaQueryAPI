# Find the spdlog library
#
# This module defines:
#  SPDLOG_FOUND        - True if spdlog library is found
#  SPDLOG_INCLUDE_DIRS - Directory where spdlog headers are located
#  SPDLOG_LIBRARIES    - Libraries to link against spdlog

# Try standard find_package first
find_package(spdlog CONFIG QUIET)

if(spdlog_FOUND)
    message(STATUS "Found spdlog via config mode")
    return()
endif()

# Try pkg-config
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_SPDLOG QUIET spdlog)
endif()

# Try to find the include directory
find_path(SPDLOG_INCLUDE_DIR
    NAMES spdlog/spdlog.h
    PATHS
        ${PC_SPDLOG_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/spdlog/include
)

# Try to find the library
find_library(SPDLOG_LIBRARY
    NAMES spdlog
    PATHS
        ${PC_SPDLOG_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Spdlog
    FOUND_VAR SPDLOG_FOUND
    REQUIRED_VARS SPDLOG_INCLUDE_DIR
)

if(SPDLOG_FOUND)
    set(SPDLOG_INCLUDE_DIRS ${SPDLOG_INCLUDE_DIR})
    
    if(SPDLOG_LIBRARY)
        set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})
        message(STATUS "Found spdlog library: ${SPDLOG_LIBRARY}")
    else()
        # spdlog can be header-only
        message(STATUS "Found spdlog headers, but no library. This is okay for header-only usage.")
    endif()
    
    message(STATUS "Found spdlog include dir: ${SPDLOG_INCLUDE_DIR}")
    
    if(NOT TARGET spdlog::spdlog)
        add_library(spdlog::spdlog INTERFACE IMPORTED)
        if(SPDLOG_LIBRARY)
            set_target_properties(spdlog::spdlog PROPERTIES
                INTERFACE_LINK_LIBRARIES "${SPDLOG_LIBRARY}"
            )
        endif()
        set_target_properties(spdlog::spdlog PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}"
        )
    endif()
endif() 