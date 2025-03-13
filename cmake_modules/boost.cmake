find_package(Boost REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE  ${Boost_LIBRARIES})
else()
    message(WARNING "Boost library not found.")
endif()

