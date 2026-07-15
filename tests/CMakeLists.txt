add_executable(protocol_tests
  protocol_tests.cpp
  ${PROJECT_SOURCE_DIR}/src/bluetooth/src/linux/socket/socket_helper.cpp)

target_include_directories(protocol_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/src/bluetooth/src/linux/socket)

target_link_libraries(protocol_tests PRIVATE common)

add_test(NAME protocol_tests COMMAND protocol_tests)

find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)

add_executable(authentication_tests authentication_tests.cpp)
target_include_directories(authentication_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/src/bluetooth/src/linux/auth)
target_link_libraries(authentication_tests PRIVATE OpenSSL::Crypto)

add_test(NAME authentication_tests COMMAND authentication_tests)
