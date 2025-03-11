project(node_tests)
# Test executable
enable_testing()
add_executable(${PROJECT_NAME} node_tests.cpp)

# Find and link Google Test
find_package(GTest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}  PRIVATE GTest::gtest GTest::gtest_main)

# Add tests
include(GoogleTest)
#gtest_discover_tests(${PROJECT_NAME})

add_test(NAME ${PROJECT_NAME}  COMMAND ${PROJECT_NAME} )
