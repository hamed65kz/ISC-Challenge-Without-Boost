cmake_minimum_required(VERSION 3.19...3.28)

project(ISC-Challenge-Bundle VERSION 0.1.0 LANGUAGES CXX)
message("Configuring ${PROJECT_NAME}")  


include(cmake_modules/configure_vcpkg/ConfigureVCPKG.cmake NO_POLICY_SCOPE)
configure_vcpkg()

enable_testing()

add_subdirectory("router")
add_subdirectory("node")
add_subdirectory("tests")

