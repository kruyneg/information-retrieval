target_sources(engine PRIVATE
    types.h
    posting_list.h
    posting_list.cpp
    inverted_index.h
    inverted_index.cpp
    compressed_posting_list.h
    compressed_posting_list.cpp)

# Testing
target_sources(search-unittests PRIVATE
    posting_list_test.cpp
    compressed_posting_list_test.cpp)
