file (GLOB TEST_FILES "*.h" "*.cpp")
file (GLOB CCMA_FILES "c-cmaes/cmaes*.*" "c-cmaes/boundary_transformation.*" )

source_group("" FILES ${TEST_FILES})
source_group("c-cmaes" FILES ${CCMA_FILES})

add_executable(spot_test ${TEST_FILES} ${CCMA_FILES})

set_target_properties(spot_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(spot_test PRIVATE ${XO_INCLUDE_DIR} ${SPOT_INCLUDE_DIR})

target_link_libraries(spot_test spot)
