# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server

# Include any dependencies generated for this target.
include CMakeFiles/MyExecutableName.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MyExecutableName.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MyExecutableName.dir/flags.make

CMakeFiles/MyExecutableName.dir/Rotator.cpp.o: CMakeFiles/MyExecutableName.dir/flags.make
CMakeFiles/MyExecutableName.dir/Rotator.cpp.o: Rotator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MyExecutableName.dir/Rotator.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MyExecutableName.dir/Rotator.cpp.o -c /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/Rotator.cpp

CMakeFiles/MyExecutableName.dir/Rotator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyExecutableName.dir/Rotator.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/Rotator.cpp > CMakeFiles/MyExecutableName.dir/Rotator.cpp.i

CMakeFiles/MyExecutableName.dir/Rotator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyExecutableName.dir/Rotator.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/Rotator.cpp -o CMakeFiles/MyExecutableName.dir/Rotator.cpp.s

CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.requires:

.PHONY : CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.requires

CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.provides: CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.requires
	$(MAKE) -f CMakeFiles/MyExecutableName.dir/build.make CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.provides.build
.PHONY : CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.provides

CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.provides.build: CMakeFiles/MyExecutableName.dir/Rotator.cpp.o


CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o: CMakeFiles/MyExecutableName.dir/flags.make
CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o: StaticScene.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o -c /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/StaticScene.cpp

CMakeFiles/MyExecutableName.dir/StaticScene.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyExecutableName.dir/StaticScene.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/StaticScene.cpp > CMakeFiles/MyExecutableName.dir/StaticScene.cpp.i

CMakeFiles/MyExecutableName.dir/StaticScene.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyExecutableName.dir/StaticScene.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/StaticScene.cpp -o CMakeFiles/MyExecutableName.dir/StaticScene.cpp.s

CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.requires:

.PHONY : CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.requires

CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.provides: CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.requires
	$(MAKE) -f CMakeFiles/MyExecutableName.dir/build.make CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.provides.build
.PHONY : CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.provides

CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.provides.build: CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o


# Object files for target MyExecutableName
MyExecutableName_OBJECTS = \
"CMakeFiles/MyExecutableName.dir/Rotator.cpp.o" \
"CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o"

# External object files for target MyExecutableName
MyExecutableName_EXTERNAL_OBJECTS =

bin/MyExecutableName: CMakeFiles/MyExecutableName.dir/Rotator.cpp.o
bin/MyExecutableName: CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o
bin/MyExecutableName: CMakeFiles/MyExecutableName.dir/build.make
bin/MyExecutableName: /home/sasl/encad/pecheux/urh15/lib/libUrho3D.a
bin/MyExecutableName: CMakeFiles/MyExecutableName.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable bin/MyExecutableName"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MyExecutableName.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MyExecutableName.dir/build: bin/MyExecutableName

.PHONY : CMakeFiles/MyExecutableName.dir/build

CMakeFiles/MyExecutableName.dir/requires: CMakeFiles/MyExecutableName.dir/Rotator.cpp.o.requires
CMakeFiles/MyExecutableName.dir/requires: CMakeFiles/MyExecutableName.dir/StaticScene.cpp.o.requires

.PHONY : CMakeFiles/MyExecutableName.dir/requires

CMakeFiles/MyExecutableName.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MyExecutableName.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MyExecutableName.dir/clean

CMakeFiles/MyExecutableName.dir/depend:
	cd /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server /home/sasl/eleves/main/3520621/Documents/Projet/planetarium/solar_server/CMakeFiles/MyExecutableName.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MyExecutableName.dir/depend

