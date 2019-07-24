cmake_minimum_required(VERSION 3.12)

project(spot)

# To create a folder hierarchy within Visual Studio.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_path(SPOT_INCLUDE_DIR
    NAMES spot/optimizer.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}
    )

# Place build products (libraries, executables) in root
# binary (build) directory. Otherwise, they get scattered around
# the build directory and so the dll's aren't next to the executables.
get_filename_component(CONFIG_NAME "${CMAKE_BINARY_DIR}" NAME)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CONFIG_NAME}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CONFIG_NAME}")

option(SPOT_TEST_ENABLED "Build and add spot_test" ON)

# Process source code.
add_subdirectory(spot)

if (SPOT_TEST_ENABLED)
    add_subdirectory(spot_test)
    enable_testing()
    add_test(spot_test "${CMAKE_SOURCE_DIR}/bin/${CONFIG_NAME}/spot_test")
endif()

