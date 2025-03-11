cmake_minimum_required(VERSION 3.19...3.28)

project(ISC-Node VERSION 0.1.0 LANGUAGES CXX)
message("Configuring ${PROJECT_NAME}")  

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(tests)

