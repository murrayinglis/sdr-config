cmake_minimum_required(VERSION 3.10)

# Set the project name
project(sdr-config)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(sdr-config ${SOURCES} include/easyBMP/EasyBMP.cpp)



# include path
include_directories(include/)
include_directories(/usr/include/uhd)
include_directories(/home/murray/boost_1_82_0)
include_directories(/usr/include/fftw3)

target_link_libraries(sdr-config /home/murray/pugixml-1.14/libpugixml.a /usr/local/lib/libuhd.so /usr/lib/x86_64-linux-gnu/libfftw3.a)
#target_link_libraries(sdr-config /usr/lib/x86_64-linux-gnu/libuhd.so)

# Directory containing Boost libraries
set(BOOST_LIB_DIR "/home/murray/boost_1_82_0/stage/lib")

# Find all .so and .a files in the directory
file(GLOB_RECURSE BOOST_LIBS "${BOOST_LIB_DIR}/*.so" "${BOOST_LIB_DIR}/*.a")

# Link all found libraries to the target
target_link_libraries(sdr-config ${BOOST_LIBS})