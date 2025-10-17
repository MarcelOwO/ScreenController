add_library(Logger STATIC
  logger.cpp
)

target_include_directories(Logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Logger PRIVATE  ng-log)
