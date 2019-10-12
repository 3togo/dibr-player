# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (DIBR_SIMPLE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")
# Recurse into the "Hello" and "Demo" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.
#add_subdirectory (Hello)
#add_subdirectory (Demo)

# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable (dibr_simple dibr_simple.cpp)

