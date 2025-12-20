add_library(storage
    doc_storage.h
    document.h
    mongo_doc_storage.h
    mongo_doc_storage.cpp
    index_storage.h
    file_index_storage.h
    file_index_storage.cpp)

target_link_libraries(storage PRIVATE mongocxx bsoncxx)
