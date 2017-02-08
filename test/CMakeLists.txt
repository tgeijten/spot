file (GLOB TEST_FILES "*.h" "*.cpp")
file (GLOB CCMA_FILES "c-cmaes/src/*.h" "c-cmaes/src/*.cpp")

source_group("" FILES ${TEST_FILES})
source_group("c-cmaes" FILES ${CCMA_FILES})

include_directories(${SPOT_INCLUDE_DIR})
add_executable(spot_test ${TEST_FILES} ${CCMA_FILES})

target_link_libraries(spot_test spot)

set_target_properties(spot_test	PROPERTIES
	PROJECT_LABEL "spot_test"
	OUTPUT_NAME spot_test
    FOLDER "spot" # Place target inside a spot folder in MSVC
	)
