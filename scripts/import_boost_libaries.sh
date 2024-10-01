#!/bin/bash

# Directory where Boost libraries are located
BOOST_LIB_DIR="/home/murray/boost_1_82_0/stage/lib"
BOOST_LIB_1_74_0_DIR="/usr/lib/x86_64-linux-gnu"

# Destination directory in your package structure
PACKAGE_LIB_DIR="/home/murray/sdr-config/sdr-config-package/usr/local/lib"

# Create the destination directory if it doesn't exist
mkdir -p $PACKAGE_LIB_DIR

# Find and copy all .so and .a files from Boost library directory
find $BOOST_LIB_DIR -name "*.so" -exec cp {} $PACKAGE_LIB_DIR \;
find $BOOST_LIB_DIR -name "*.a" -exec cp {} $PACKAGE_LIB_DIR \;

find $BOOST_LIB_1_74_0_DIR -name "*boost*.so*" -exec cp {} $PACKAGE_LIB_DIR \;
find $BOOST_LIB_1_74_0_DIR -name "*boost*.a*" -exec cp {} $PACKAGE_LIB_DIR \;

