
add_library(App STATIC App.cpp App.h)

target_include_directories(App
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}  # This line is crucial
)

target_link_libraries(App
        PUBLIC
        Window Renderer Bluetooth glad Storage File ng-log)


