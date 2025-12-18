add_subdirectory(linguistics)
add_subdirectory(storage)
add_subdirectory(engine)

add_executable(search-engine main.cpp)

target_link_libraries(search-engine PRIVATE engine)
