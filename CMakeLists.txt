project(kata_phase1)

cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

# if no cdt root is given use default path
if(EOSIO_CDT_ROOT STREQUAL "" OR NOT EOSIO_CDT_ROOT)
    message("EOSIO_CDT_ROOT is missing")
endif()

ExternalProject_Add(
        kata_project
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
        BINARY_DIR ${CMAKE_BINARY_DIR}/kata
        CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake  -DEOSIO_ROOT=${EOSIO_ROOT} -DEOSIO_CONTRACTS_ROOT=${EOSIO_CONTRACTS_ROOT}
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

ExternalProject_Add(
    kata_tests_project
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests
    BINARY_DIR ${CMAKE_BINARY_DIR}/tests
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DEOSIO_ROOT=${EOSIO_ROOT} -DEOSIO_CONTRACTS_ROOT=${EOSIO_CONTRACTS_ROOT}
    TEST_COMMAND ./kata_tests --color_output  --log_level=all -- --verbose
    INSTALL_COMMAND ""
    BUILD_ALWAYS 1
    )
    
add_dependencies(kata_tests_project kata_project)
