find_package(sdbus-c++ REQUIRED)


add_library(Bluetooth STATIC bluetooth_manager.cpp bluetooth_manager.h)

target_include_directories(Bluetooth
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}  # This line is crucial
)
target_link_libraries(Bluetooth PUBLIC SDBusCpp::sdbus-c++)

