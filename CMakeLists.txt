cmake_minimum_required (VERSION 3.2)
project (synacor-challenge)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING
        "Possible values are empty, Debug, Release, RelWithDebInfo and MinSizeRel"
        FORCE)
endif ()

add_subdirectory (be)
add_subdirectory (fe)

