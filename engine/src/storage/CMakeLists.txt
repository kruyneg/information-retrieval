add_library(storage
    storage.h
    document.h
    mongo_storage.h
    mongo_storage.cpp)

target_link_libraries(storage PRIVATE mongocxx bsoncxx)
