target_sources(engine PRIVATE
    ast.h
    ast.cpp
    bool_query.h
    bool_query.cpp
    ranked_query.h
    ranked_query.cpp)

# Testing
target_sources(search-unittests PRIVATE
    bool_query_test.cpp
    ranked_query_test.cpp)
