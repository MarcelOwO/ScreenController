add_library(File STATIC file_processor.cpp file_processor.h)

target_include_directories(File PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


