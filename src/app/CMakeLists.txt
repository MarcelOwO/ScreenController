add_library(App STATIC app.cpp)

target_include_directories(App
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include  # This line is crucial
)

target_link_libraries(App
        PUBLIC
        Window Renderer Bluetooth glad Storage File Logger  common)


