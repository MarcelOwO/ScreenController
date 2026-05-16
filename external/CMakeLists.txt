find_package(glfw3 3.3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(sdbus-c++ REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(ng-log CONFIG REQUIRED)

find_package(PkgConfig Required)
pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)

add_library(stb STATIC stb.cpp)

target_include_directories(stb
        PUBLIC
        ./stb/stb_image.h  # This line is crucial
)

