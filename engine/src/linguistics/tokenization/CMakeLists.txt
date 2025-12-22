target_sources(linguistics PUBLIC
    tokenizer.h
    tokenizer_impl.h
    tokenizer_impl.cpp)

add_subdirectory(rules)

# Testing
target_sources(search-unittests PRIVATE
    tokenizer_test.cpp)
