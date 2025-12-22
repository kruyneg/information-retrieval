add_library(linguistics
    preprocessor.h
    preprocessor.cpp
    utils.h
    utils.cpp)

add_subdirectory(tokenization)
add_subdirectory(lemmatization)

# Testing
target_link_libraries(search-unittests PRIVATE linguistics)
