target_sources(linguistics PUBLIC
    lemmatizer.h
    mock_lemmatizer.h
    mock_lemmatizer.cpp
    stemming.h
    stemming.cpp
    dict_lemmatizer.h
    dict_lemmatizer.cpp)

# Testing
target_sources(search-unittests PRIVATE
    stemming_test.cpp)
