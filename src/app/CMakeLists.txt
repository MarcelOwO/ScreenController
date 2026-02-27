add_library(App STATIC app.cpp)

target_link_libraries(
  App
  PUBLIC Logger
         common
         Window
         Renderer
         Storage
         File
         Bluetooth)
