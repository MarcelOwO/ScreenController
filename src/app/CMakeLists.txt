add_library(App STATIC app.cpp)

target_link_libraries(App
        PUBLIC
        Window Renderer  glad Storage File Logger  common)
