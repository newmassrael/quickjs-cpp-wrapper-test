cmake_minimum_required(VERSION 3.12)

project(QuickJSWrapper LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add QuickJS as subdirectory
add_subdirectory(quickjs)

# Find required packages
find_package(Threads REQUIRED)

# Create the wrapper library
add_library(quickjs_wrapper STATIC
    quickjs_wrapper.cpp
    quickjs_wrapper.h
)

# Link with QuickJS
target_link_libraries(quickjs_wrapper 
    PUBLIC qjs
    PRIVATE Threads::Threads
)

# Include directories
target_include_directories(quickjs_wrapper
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/quickjs
)

# Compiler settings for the wrapper
target_compile_features(quickjs_wrapper PUBLIC cxx_std_17)

# Enable warnings but allow QuickJS to have its own warning settings
if(MSVC)
    target_compile_options(quickjs_wrapper PRIVATE /W4)
else()
    target_compile_options(quickjs_wrapper PRIVATE -Wall -Wextra)
endif()

# Create an example executable
add_executable(quickjs_example example.cpp)
target_link_libraries(quickjs_example quickjs_wrapper)

# Installation
install(TARGETS quickjs_wrapper
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES quickjs_wrapper.h 
    DESTINATION include
)

# Optional: create a pkg-config file (disabled for now)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/quickjs_wrapper.pc.in
#                ${CMAKE_CURRENT_BINARY_DIR}/quickjs_wrapper.pc
#                @ONLY)

# Enable testing if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_test(NAME quickjs_example_test COMMAND quickjs_example)
    
    # Add Google Test from submodule
    add_subdirectory(googletest)
    
    # Create test executable
    add_executable(quickjs_wrapper_tests
        tests/test_basic_functionality.cpp
        tests/test_stack_overflow.cpp
        tests/test_recursive_calls.cpp
        tests/test_memory_exhaustion.cpp
        tests/test_non_recursive_stackoverflow.cpp
    )
    
    target_link_libraries(quickjs_wrapper_tests
        quickjs_wrapper
        gtest_main
        gmock_main
    )
    
    # Include test directories
    target_include_directories(quickjs_wrapper_tests PRIVATE tests)
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(quickjs_wrapper_tests)
endif()