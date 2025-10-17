add_library(Storage STATIC storage_manager.cpp )

target_include_directories(Storage
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


target_link_libraries(Storage PUBLIC Logger common)
