cmake_minimum_required (VERSION 3.2)
find_package (Threads)
add_executable (ver ver.cpp)
set_property (TARGET ver PROPERTY CXX_STANDARD 11)
set_property (TARGET ver PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (ver ${CMAKE_THREAD_LIBS_INIT})

