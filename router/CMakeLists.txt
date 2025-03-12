cmake_minimum_required(VERSION 3.28.0)
project(ISC-Router VERSION 0.1.0 LANGUAGES C CXX)
message("Configuring ${PROJECT_NAME}")  

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

add_executable(${PROJECT_NAME} main.cpp)


find_package(Boost REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE  ${Boost_LIBRARIES})
else()
    message(WARNING "Boost library not found.")
endif()



if(NOT DEFINED SPDLOG_ADDED)
    option(SPDLOG_BUILD_TESTS "" OFF)
    option(SPDLOG_BUILD_EXAMPLES "" OFF)
    add_subdirectory(../thirdparty/spdlog "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
    set(SPDLOG_ADDED TRUE PARENT_SCOPE)
endif()
# Link against spdlog::spdlog (header-only interface)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)