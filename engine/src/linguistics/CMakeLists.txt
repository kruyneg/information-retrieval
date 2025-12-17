add_library(linguistics
    preprocessor.h
    preprocessor.cpp
    utils.h
    utils.cpp)

add_subdirectory(tokenization)
add_subdirectory(lemmatization)
