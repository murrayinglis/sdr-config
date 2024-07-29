cmake_minimum_required(VERSION 3.10)

# Set the project name
project(sdr-config)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(sdr-config main.cpp)

# include path
include_directories(include/)

target_link_libraries(sdr-config /home/murray/pugixml-1.14/libpugixml.a)

# Optionally, you can add the source files of PugiXML if you have them
# add_library(pugixml STATIC path/to/pugixml.cpp)
# target_link_libraries(MyPugiXMLProject pugixml)
