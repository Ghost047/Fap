cmake_minimum_required (VERSION 3.3)
project ("FLAP: Flexible Arbitrary Precision Numeric Library")
set (FLAP_VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)

# Generate the library
add_library(flap ${CMAKE_SOURCE_DIR}/src/fap.cpp)

# Include directories
target_include_directories(flap
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                          )

# Compiler options
target_compile_options(flap
                      PRIVATE -fno-use-cxa-atexit
                      )

# Copy the object file
#add_custom_command( OUTPUT fap 
#               COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/CMakeFiles/fap.dir/src/fap.cpp.o ${CMAKE_BINARY_DIR}/fap.o
#               DEPENDS details/part.out 
#               )
