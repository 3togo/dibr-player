# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/jc/git/dibr-player

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jc/git/dibr-player/build

# Include any dependencies generated for this target.
include CMakeFiles/dibr-player-opencl.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/dibr-player-opencl.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/dibr-player-opencl.dir/flags.make

CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o: CMakeFiles/dibr-player-opencl.dir/flags.make
CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o: ../opencl/yuv.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jc/git/dibr-player/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o -c /home/jc/git/dibr-player/opencl/yuv.cpp

CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jc/git/dibr-player/opencl/yuv.cpp > CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.i

CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jc/git/dibr-player/opencl/yuv.cpp -o CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.s

CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.requires:

.PHONY : CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.requires

CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.provides: CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.requires
	$(MAKE) -f CMakeFiles/dibr-player-opencl.dir/build.make CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.provides.build
.PHONY : CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.provides

CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.provides.build: CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o


CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o: CMakeFiles/dibr-player-opencl.dir/flags.make
CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o: ../opencl/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jc/git/dibr-player/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o -c /home/jc/git/dibr-player/opencl/main.cpp

CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jc/git/dibr-player/opencl/main.cpp > CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.i

CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jc/git/dibr-player/opencl/main.cpp -o CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.s

CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.requires:

.PHONY : CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.requires

CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.provides: CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/dibr-player-opencl.dir/build.make CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.provides.build
.PHONY : CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.provides

CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.provides.build: CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o


# Object files for target dibr-player-opencl
dibr__player__opencl_OBJECTS = \
"CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o" \
"CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o"

# External object files for target dibr-player-opencl
dibr__player__opencl_EXTERNAL_OBJECTS =

dibr-player-opencl: CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o
dibr-player-opencl: CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o
dibr-player-opencl: CMakeFiles/dibr-player-opencl.dir/build.make
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libboost_program_options.a
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libOpenCL.so
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_shape.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_stitching.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_superres.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_videostab.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_aruco.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_bgsegm.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_bioinspired.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_ccalib.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_datasets.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_dpm.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_face.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_freetype.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_fuzzy.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_hdf.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_line_descriptor.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_optflow.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_plot.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_reg.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_saliency.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_stereo.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_structured_light.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_surface_matching.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_text.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_ximgproc.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_xobjdetect.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_xphoto.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_video.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_viz.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_phase_unwrapping.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_rgbd.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_calib3d.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_features2d.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_flann.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_objdetect.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_ml.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_highgui.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_photo.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_videoio.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_imgcodecs.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_imgproc.so.3.2.0
dibr-player-opencl: /usr/lib/x86_64-linux-gnu/libopencv_core.so.3.2.0
dibr-player-opencl: CMakeFiles/dibr-player-opencl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jc/git/dibr-player/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable dibr-player-opencl"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dibr-player-opencl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/dibr-player-opencl.dir/build: dibr-player-opencl

.PHONY : CMakeFiles/dibr-player-opencl.dir/build

CMakeFiles/dibr-player-opencl.dir/requires: CMakeFiles/dibr-player-opencl.dir/opencl/yuv.cpp.o.requires
CMakeFiles/dibr-player-opencl.dir/requires: CMakeFiles/dibr-player-opencl.dir/opencl/main.cpp.o.requires

.PHONY : CMakeFiles/dibr-player-opencl.dir/requires

CMakeFiles/dibr-player-opencl.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/dibr-player-opencl.dir/cmake_clean.cmake
.PHONY : CMakeFiles/dibr-player-opencl.dir/clean

CMakeFiles/dibr-player-opencl.dir/depend:
	cd /home/jc/git/dibr-player/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jc/git/dibr-player /home/jc/git/dibr-player /home/jc/git/dibr-player/build /home/jc/git/dibr-player/build /home/jc/git/dibr-player/build/CMakeFiles/dibr-player-opencl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/dibr-player-opencl.dir/depend

