add_library(engine
    engine.h
    engine.cpp)

add_subdirectory(indexing)
add_subdirectory(query)

target_link_libraries(engine PRIVATE storage linguistics)

# Testing
target_link_libraries(search-unittests PRIVATE
    engine)
