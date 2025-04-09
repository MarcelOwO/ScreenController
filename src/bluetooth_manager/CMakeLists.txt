add_subdirectory(${CMAKE_SOURCE_DIR}/external/simpleble/simpleble ${CMAKE_BINARY_DIR}/external/simpleble/simpleble)

add_library(Bluetooth STATIC bluetooth_manager.cpp bluetooth_manager.h)

target_include_directories(Bluetooth PUBLIC
        ${CMAKE_SOURCE_DIR}/external/simpleble/simpleble/include)

target_link_libraries(Bluetooth PRIVATE simpleble::simpleble )
