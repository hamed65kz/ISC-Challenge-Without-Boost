cmake_minimum_required(VERSION 3.19...3.28)

project(ISC-Node VERSION 0.1.0 LANGUAGES CXX)
message("Configuring ${PROJECT_NAME}")  

SET(SOURCES
    src/node.cpp
    src/tcp_socket.cpp
    )

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)


include(../cmake_modules/spdlog.cmake)


