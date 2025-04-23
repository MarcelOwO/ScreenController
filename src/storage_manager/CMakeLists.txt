add_library(Storage STATIC storage_manager.cpp storage_manager.h)

target_include_directories(Storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


