# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
project ("ROM-Recompiler")
cmake_minimum_required (VERSION 3.8)

# Include sub-projects.
add_subdirectory ("src")
add_subdirectory ("test")
