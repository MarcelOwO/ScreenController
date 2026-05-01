add_library(Events src/event_factory.cpp src/event_manager.cpp)

target_include_directories(Events PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Events PUBLIC Logger common)
