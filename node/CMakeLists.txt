cmake_minimum_required(VERSION 3.19...3.28)

project(ISC-Node VERSION 0.1.0 LANGUAGES CXX)
message("Configuring ${PROJECT_NAME}")  



SET(SOURCES
    src/node.cpp
    src/tcp_socket.cpp
    )

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)


if(NOT DEFINED SPDLOG_ADDED)
    option(SPDLOG_BUILD_TESTS "" OFF)
    option(SPDLOG_BUILD_EXAMPLES "" OFF)
    add_subdirectory(../thirdparty/spdlog "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
    set(SPDLOG_ADDED TRUE PARENT_SCOPE)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)


