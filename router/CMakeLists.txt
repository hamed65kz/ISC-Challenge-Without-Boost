cmake_minimum_required(VERSION 3.28.0)
project(ISC-Router VERSION 0.1.0 LANGUAGES C CXX)
message("Configuring ${PROJECT_NAME}")  

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)


SET(SOURCES
    src/member_session.cpp
    src/router.cpp
    )

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})


include(../cmake_modules/boost.cmake)
include(../cmake_modules/spdlog.cmake)