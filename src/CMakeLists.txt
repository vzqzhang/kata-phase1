project(kata)

cmake_minimum_required(VERSION 3.5)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( kata kata ${CMAKE_SOURCE_DIR}/kata.cpp )

target_include_directories(
        kata PUBLIC
        ${EOSIO_CONTRACTS_ROOT}/contracts/eosio.token/include
        ${EOSIO_ROOT}/libraries/chain/include )
    
set_target_properties(kata
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
