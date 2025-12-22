# Testing
add_executable(search-unittests)

set_target_properties(search-unittests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(search-unittests PRIVATE gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(search-unittests)

# Main
add_executable(search-engine main.cpp)

target_link_libraries(search-engine PRIVATE engine)

set_target_properties(search-engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(linguistics)
add_subdirectory(storage)
add_subdirectory(engine)
add_subdirectory(third_party)
