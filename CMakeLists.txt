cmake_minimum_required (VERSION 3.0.2)
project (alienos C)

SET(CMAKE_C_FLAGS "-std=c99")

# Add the include folder to the search path for header files
# so that we will find alienos.h
include_directories("include")
 
# Add the executable.
add_executable(emu
    src/emu.c
    src/init.c
    src/emulate.c
    src/exec.c
    src/terminal.c
)
