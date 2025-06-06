# Set fmt to be header-only
add_compile_definitions(FMT_HEADER_ONLY)

if(NOT DEFINED SPDLOG_ADDED)
    option(SPDLOG_BUILD_TESTS "" OFF)
    option(SPDLOG_BUILD_EXAMPLES "" OFF)
    add_subdirectory(../thirdparty/spdlog "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
    set(SPDLOG_ADDED TRUE PARENT_SCOPE)
endif()
# Link against spdlog::spdlog (header-only interface)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)