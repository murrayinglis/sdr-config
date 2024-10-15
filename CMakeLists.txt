cmake_minimum_required(VERSION 3.10)
project(sdr-config)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(sdr-config ${SOURCES})

# include
include_directories(include/)
include_directories(/usr/include/uhd)
include_directories(/home/murry/boost_1_82_0)

# Find Boost
find_package(Boost REQUIRED COMPONENTS thread system program_options)

# Link Boost libraries
target_link_libraries(sdr-config Boost::thread Boost::program_options 
    Boost::system /home/murry/pugixml-1.14/libpugixml.a /usr/lib/x86_64-linux-gnu/libuhd.so.4.1.0)
