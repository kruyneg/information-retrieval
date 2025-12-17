add_subdirectory(linguistics)

add_subdirectory(storage)

add_executable(search-engine main.cpp)

target_link_libraries(search-engine PRIVATE linguistics storage)
