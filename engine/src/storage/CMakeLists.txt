add_library(storage
    doc_storage.h
    document.h
    mongo_doc_storage.h
    mongo_doc_storage.cpp)

target_link_libraries(storage PRIVATE mongocxx bsoncxx)
