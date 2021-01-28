cmake_minimum_required(VERSION 3.10)

project(spot)

# To create a folder hierarchy within Visual Studio.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_path(SPOT_INCLUDE_DIR
    NAMES spot/optimizer.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}
    )

option(SPOT_TEST_ENABLED "Build and add spot_test" OFF)

# Process source code.
add_subdirectory(spot)

if (SPOT_TEST_ENABLED)
    add_subdirectory(spot_test)
    enable_testing()
    add_test(spot_test "${CMAKE_SOURCE_DIR}/bin/${CONFIG_NAME}/spot_test")
endif()

