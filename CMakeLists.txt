cmake_minimum_required (VERSION 3.3)
project ("FAP: Flexible Arbitrary Precision Numeric Library")
set (FAP_VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)

# Generate the library
add_library(fap ${CMAKE_SOURCE_DIR}/src/Fap.cpp)

# Include directories
target_include_directories(fap
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                          )

# Compiler options
target_compile_options(fap
                      PRIVATE -fno-use-cxa-atexit -m64
                      )

# Generate the test add_executable
add_executable(fap_test
	       EXCLUDE_FROM_ALL
               ${CMAKE_SOURCE_DIR}/test/main.cpp
              )
target_include_directories(fap_test PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(fap_test fap)
