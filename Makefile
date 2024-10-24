# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/murry/sdr-config

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/murry/sdr-config

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/cmake-gui -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/murry/sdr-config/CMakeFiles /home/murry/sdr-config//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/murry/sdr-config/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named sdr-config

# Build rule for target.
sdr-config: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 sdr-config
.PHONY : sdr-config

# fast build rule for target.
sdr-config/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/build
.PHONY : sdr-config/fast

src/hardware/RX_funcs.o: src/hardware/RX_funcs.cpp.o
.PHONY : src/hardware/RX_funcs.o

# target to build an object file
src/hardware/RX_funcs.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/RX_funcs.cpp.o
.PHONY : src/hardware/RX_funcs.cpp.o

src/hardware/RX_funcs.i: src/hardware/RX_funcs.cpp.i
.PHONY : src/hardware/RX_funcs.i

# target to preprocess a source file
src/hardware/RX_funcs.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/RX_funcs.cpp.i
.PHONY : src/hardware/RX_funcs.cpp.i

src/hardware/RX_funcs.s: src/hardware/RX_funcs.cpp.s
.PHONY : src/hardware/RX_funcs.s

# target to generate assembly for a file
src/hardware/RX_funcs.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/RX_funcs.cpp.s
.PHONY : src/hardware/RX_funcs.cpp.s

src/hardware/TXRX_config.o: src/hardware/TXRX_config.cpp.o
.PHONY : src/hardware/TXRX_config.o

# target to build an object file
src/hardware/TXRX_config.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TXRX_config.cpp.o
.PHONY : src/hardware/TXRX_config.cpp.o

src/hardware/TXRX_config.i: src/hardware/TXRX_config.cpp.i
.PHONY : src/hardware/TXRX_config.i

# target to preprocess a source file
src/hardware/TXRX_config.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TXRX_config.cpp.i
.PHONY : src/hardware/TXRX_config.cpp.i

src/hardware/TXRX_config.s: src/hardware/TXRX_config.cpp.s
.PHONY : src/hardware/TXRX_config.s

# target to generate assembly for a file
src/hardware/TXRX_config.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TXRX_config.cpp.s
.PHONY : src/hardware/TXRX_config.cpp.s

src/hardware/TXRX_funcs.o: src/hardware/TXRX_funcs.cpp.o
.PHONY : src/hardware/TXRX_funcs.o

# target to build an object file
src/hardware/TXRX_funcs.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TXRX_funcs.cpp.o
.PHONY : src/hardware/TXRX_funcs.cpp.o

src/hardware/TXRX_funcs.i: src/hardware/TXRX_funcs.cpp.i
.PHONY : src/hardware/TXRX_funcs.i

# target to preprocess a source file
src/hardware/TXRX_funcs.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TXRX_funcs.cpp.i
.PHONY : src/hardware/TXRX_funcs.cpp.i

src/hardware/TXRX_funcs.s: src/hardware/TXRX_funcs.cpp.s
.PHONY : src/hardware/TXRX_funcs.s

# target to generate assembly for a file
src/hardware/TXRX_funcs.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TXRX_funcs.cpp.s
.PHONY : src/hardware/TXRX_funcs.cpp.s

src/hardware/TX_Funcs.o: src/hardware/TX_Funcs.cpp.o
.PHONY : src/hardware/TX_Funcs.o

# target to build an object file
src/hardware/TX_Funcs.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TX_Funcs.cpp.o
.PHONY : src/hardware/TX_Funcs.cpp.o

src/hardware/TX_Funcs.i: src/hardware/TX_Funcs.cpp.i
.PHONY : src/hardware/TX_Funcs.i

# target to preprocess a source file
src/hardware/TX_Funcs.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TX_Funcs.cpp.i
.PHONY : src/hardware/TX_Funcs.cpp.i

src/hardware/TX_Funcs.s: src/hardware/TX_Funcs.cpp.s
.PHONY : src/hardware/TX_Funcs.s

# target to generate assembly for a file
src/hardware/TX_Funcs.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/hardware/TX_Funcs.cpp.s
.PHONY : src/hardware/TX_Funcs.cpp.s

src/main.o: src/main.cpp.o
.PHONY : src/main.o

# target to build an object file
src/main.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/main.cpp.o
.PHONY : src/main.cpp.o

src/main.i: src/main.cpp.i
.PHONY : src/main.i

# target to preprocess a source file
src/main.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/main.cpp.i
.PHONY : src/main.cpp.i

src/main.s: src/main.cpp.s
.PHONY : src/main.s

# target to generate assembly for a file
src/main.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/main.cpp.s
.PHONY : src/main.cpp.s

src/tests/RX_tests.o: src/tests/RX_tests.cpp.o
.PHONY : src/tests/RX_tests.o

# target to build an object file
src/tests/RX_tests.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/RX_tests.cpp.o
.PHONY : src/tests/RX_tests.cpp.o

src/tests/RX_tests.i: src/tests/RX_tests.cpp.i
.PHONY : src/tests/RX_tests.i

# target to preprocess a source file
src/tests/RX_tests.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/RX_tests.cpp.i
.PHONY : src/tests/RX_tests.cpp.i

src/tests/RX_tests.s: src/tests/RX_tests.cpp.s
.PHONY : src/tests/RX_tests.s

# target to generate assembly for a file
src/tests/RX_tests.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/RX_tests.cpp.s
.PHONY : src/tests/RX_tests.cpp.s

src/tests/TX_tests.o: src/tests/TX_tests.cpp.o
.PHONY : src/tests/TX_tests.o

# target to build an object file
src/tests/TX_tests.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/TX_tests.cpp.o
.PHONY : src/tests/TX_tests.cpp.o

src/tests/TX_tests.i: src/tests/TX_tests.cpp.i
.PHONY : src/tests/TX_tests.i

# target to preprocess a source file
src/tests/TX_tests.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/TX_tests.cpp.i
.PHONY : src/tests/TX_tests.cpp.i

src/tests/TX_tests.s: src/tests/TX_tests.cpp.s
.PHONY : src/tests/TX_tests.s

# target to generate assembly for a file
src/tests/TX_tests.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/TX_tests.cpp.s
.PHONY : src/tests/TX_tests.cpp.s

src/tests/loopback.o: src/tests/loopback.cpp.o
.PHONY : src/tests/loopback.o

# target to build an object file
src/tests/loopback.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/loopback.cpp.o
.PHONY : src/tests/loopback.cpp.o

src/tests/loopback.i: src/tests/loopback.cpp.i
.PHONY : src/tests/loopback.i

# target to preprocess a source file
src/tests/loopback.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/loopback.cpp.i
.PHONY : src/tests/loopback.cpp.i

src/tests/loopback.s: src/tests/loopback.cpp.s
.PHONY : src/tests/loopback.s

# target to generate assembly for a file
src/tests/loopback.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/loopback.cpp.s
.PHONY : src/tests/loopback.cpp.s

src/tests/misc.o: src/tests/misc.cpp.o
.PHONY : src/tests/misc.o

# target to build an object file
src/tests/misc.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/misc.cpp.o
.PHONY : src/tests/misc.cpp.o

src/tests/misc.i: src/tests/misc.cpp.i
.PHONY : src/tests/misc.i

# target to preprocess a source file
src/tests/misc.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/misc.cpp.i
.PHONY : src/tests/misc.cpp.i

src/tests/misc.s: src/tests/misc.cpp.s
.PHONY : src/tests/misc.s

# target to generate assembly for a file
src/tests/misc.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/misc.cpp.s
.PHONY : src/tests/misc.cpp.s

src/tests/pulsed.o: src/tests/pulsed.cpp.o
.PHONY : src/tests/pulsed.o

# target to build an object file
src/tests/pulsed.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/pulsed.cpp.o
.PHONY : src/tests/pulsed.cpp.o

src/tests/pulsed.i: src/tests/pulsed.cpp.i
.PHONY : src/tests/pulsed.i

# target to preprocess a source file
src/tests/pulsed.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/pulsed.cpp.i
.PHONY : src/tests/pulsed.cpp.i

src/tests/pulsed.s: src/tests/pulsed.cpp.s
.PHONY : src/tests/pulsed.s

# target to generate assembly for a file
src/tests/pulsed.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/pulsed.cpp.s
.PHONY : src/tests/pulsed.cpp.s

src/tests/tests.o: src/tests/tests.cpp.o
.PHONY : src/tests/tests.o

# target to build an object file
src/tests/tests.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/tests.cpp.o
.PHONY : src/tests/tests.cpp.o

src/tests/tests.i: src/tests/tests.cpp.i
.PHONY : src/tests/tests.i

# target to preprocess a source file
src/tests/tests.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/tests.cpp.i
.PHONY : src/tests/tests.cpp.i

src/tests/tests.s: src/tests/tests.cpp.s
.PHONY : src/tests/tests.s

# target to generate assembly for a file
src/tests/tests.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/tests/tests.cpp.s
.PHONY : src/tests/tests.cpp.s

src/utilities/config.o: src/utilities/config.cpp.o
.PHONY : src/utilities/config.o

# target to build an object file
src/utilities/config.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/utilities/config.cpp.o
.PHONY : src/utilities/config.cpp.o

src/utilities/config.i: src/utilities/config.cpp.i
.PHONY : src/utilities/config.i

# target to preprocess a source file
src/utilities/config.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/utilities/config.cpp.i
.PHONY : src/utilities/config.cpp.i

src/utilities/config.s: src/utilities/config.cpp.s
.PHONY : src/utilities/config.s

# target to generate assembly for a file
src/utilities/config.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/utilities/config.cpp.s
.PHONY : src/utilities/config.cpp.s

src/utilities/utils.o: src/utilities/utils.cpp.o
.PHONY : src/utilities/utils.o

# target to build an object file
src/utilities/utils.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/utilities/utils.cpp.o
.PHONY : src/utilities/utils.cpp.o

src/utilities/utils.i: src/utilities/utils.cpp.i
.PHONY : src/utilities/utils.i

# target to preprocess a source file
src/utilities/utils.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/utilities/utils.cpp.i
.PHONY : src/utilities/utils.cpp.i

src/utilities/utils.s: src/utilities/utils.cpp.s
.PHONY : src/utilities/utils.s

# target to generate assembly for a file
src/utilities/utils.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sdr-config.dir/build.make CMakeFiles/sdr-config.dir/src/utilities/utils.cpp.s
.PHONY : src/utilities/utils.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... sdr-config"
	@echo "... src/hardware/RX_funcs.o"
	@echo "... src/hardware/RX_funcs.i"
	@echo "... src/hardware/RX_funcs.s"
	@echo "... src/hardware/TXRX_config.o"
	@echo "... src/hardware/TXRX_config.i"
	@echo "... src/hardware/TXRX_config.s"
	@echo "... src/hardware/TXRX_funcs.o"
	@echo "... src/hardware/TXRX_funcs.i"
	@echo "... src/hardware/TXRX_funcs.s"
	@echo "... src/hardware/TX_Funcs.o"
	@echo "... src/hardware/TX_Funcs.i"
	@echo "... src/hardware/TX_Funcs.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
	@echo "... src/tests/RX_tests.o"
	@echo "... src/tests/RX_tests.i"
	@echo "... src/tests/RX_tests.s"
	@echo "... src/tests/TX_tests.o"
	@echo "... src/tests/TX_tests.i"
	@echo "... src/tests/TX_tests.s"
	@echo "... src/tests/loopback.o"
	@echo "... src/tests/loopback.i"
	@echo "... src/tests/loopback.s"
	@echo "... src/tests/misc.o"
	@echo "... src/tests/misc.i"
	@echo "... src/tests/misc.s"
	@echo "... src/tests/pulsed.o"
	@echo "... src/tests/pulsed.i"
	@echo "... src/tests/pulsed.s"
	@echo "... src/tests/tests.o"
	@echo "... src/tests/tests.i"
	@echo "... src/tests/tests.s"
	@echo "... src/utilities/config.o"
	@echo "... src/utilities/config.i"
	@echo "... src/utilities/config.s"
	@echo "... src/utilities/utils.o"
	@echo "... src/utilities/utils.i"
	@echo "... src/utilities/utils.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

