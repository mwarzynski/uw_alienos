cmake_minimum_required (VERSION 2.6)
project (alienos)

# Add the include folder to the search path for header files
# so that we will find alienos.h
include_directories("include")
 
# Add the executable.
add_executable(emu src/emu.c src/init.c)
