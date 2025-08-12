add_library(Updater STATIC updater.cpp updater.h)
target_include_directories(Updater PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Updater PRIVATE ng-log)