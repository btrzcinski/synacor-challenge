add_executable (fe main.cpp args.cpp codestr.cpp file.cpp)
set_property (TARGET fe PROPERTY CXX_STANDARD 11)
set_property (TARGET fe PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries (fe LINK_PUBLIC be)

