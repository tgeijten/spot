# Gather spot sources.
file (GLOB SPOT_SOURCES "*.h" "*.cpp")

include( PrecompiledHeaders )
use_precompiled_headers( SPOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h" "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp" 256 )

add_library(spot SHARED ${SPOT_SOURCES})

if (MSVC)
	source_group("" FILES ${PRECOMPILED_HEADER_FILES})
	source_group("spot" FILES ${SPOT_SOURCES})
endif()

include_directories(${XO_INCLUDE_DIR})
include_directories(${SPOT_INCLUDE_DIR})

target_compile_definitions( spot PRIVATE SPOT_EXPORTS )

target_link_libraries( spot xo )

set_target_properties(spot PROPERTIES 
	PROJECT_LABEL "spot"
	OUTPUT_NAME spot
    FOLDER "spot" # Place target inside a spot folder in MSVC
	)
