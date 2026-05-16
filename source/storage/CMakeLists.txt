add_library(Storage
  STATIC
  src/storage_manager_impl.cpp
  src/storage_factory.cpp
)

target_include_directories(Storage
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_link_libraries(Storage PUBLIC Logger common)
