add_library(Logger STATIC
  src/default_logger.cpp
  src/logger_factory.cpp
)

target_include_directories(Logger
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(Logger PUBLIC common PRIVATE spdlog)
