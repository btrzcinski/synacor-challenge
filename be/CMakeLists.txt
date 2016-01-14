add_library (be be.cpp)
set_property (TARGET be PROPERTY CXX_STANDARD 11)
set_property (TARGET be PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories (be PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

