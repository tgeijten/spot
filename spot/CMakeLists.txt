file (GLOB SPOT_SOURCES "*.h" "*.cpp")

add_library(spot SHARED ${SPOT_SOURCES})

set_target_properties(spot PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories( spot PUBLIC ${SPOT_INCLUDE_DIR})

target_link_libraries( spot xo )

if (MSVC)
	target_precompile_headers(spot PRIVATE <string> <vector> <memory> <fstream>)
	file ( GLOB_RECURSE PRECOMPILED_HEADER_FILES ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cmake_pch.*)
	source_group("CMakePCH" FILES ${PRECOMPILED_HEADER_FILES})
	source_group("spot" FILES ${SPOT_SOURCES})
	target_compile_definitions( spot PRIVATE SPOT_EXPORTS )
	target_compile_options( spot PRIVATE "/MP" ) # multithreaded compilation on MSVC
endif()

set_target_properties(spot PROPERTIES 
	PROJECT_LABEL "spot"
	OUTPUT_NAME spot
    FOLDER "spot" # Place target inside a spot folder in MSVC
	)
