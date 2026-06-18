cmake_minimum_required(VERSION 4.0)

project(rsa LANGUAGES CXX)
include(CTest)

option(RSA_ENABLE_TESTS "Build tests" ON)

add_library(rsa INTERFACE)
target_include_directories(rsa INTERFACE src)

add_executable(keygen)
target_sources(keygen PRIVATE src/keygen.cpp)
target_link_libraries(keygen PRIVATE rsa)

add_executable(encrypt)
target_sources(encrypt PRIVATE src/encrypt.cpp)
target_link_libraries(encrypt PRIVATE rsa)

add_executable(decrypt)
target_sources(decrypt PRIVATE src/decrypt.cpp)
target_link_libraries(decrypt PRIVATE rsa)

if (RSA_ENABLE_TESTS)
    add_executable(primegen_test)
    target_sources(primegen_test PRIVATE test/primegen.cpp)
    target_link_libraries(primegen_test PRIVATE rsa)
    add_test(NAME primegen_test COMMAND ./primegen_test)

    add_executable(coprimegen_test)
    target_sources(coprimegen_test PRIVATE test/coprimegen.cpp)
    target_link_libraries(coprimegen_test PRIVATE rsa)
    add_test(NAME coprimegen_test COMMAND ./coprimegen_test)

    add_executable(gcd_test)
    target_sources(gcd_test PRIVATE test/gcd.cpp)
    target_link_libraries(gcd_test PRIVATE rsa)
    add_test(NAME gcd_test COMMAND ./gcd_test)

    add_executable(keygen_test)
    target_sources(keygen_test PRIVATE test/keygen.cpp)
    target_link_libraries(keygen_test PRIVATE rsa)
    add_test(NAME keygen_test COMMAND ./keygen_test)
endif()
