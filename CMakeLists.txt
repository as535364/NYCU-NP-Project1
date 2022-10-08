cmake_minimum_required(VERSION 3.23)
project(NP_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(npshell src/npshell.cpp)

add_executable(noop src/executable/noop.cpp)
add_executable(number src/executable/number.cpp)
add_executable(removetag src/executable/removetag.cpp)
add_executable(removetag0 src/executable/removetag0.cpp)


set_target_properties( noop PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin )
set_target_properties( number PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin )
set_target_properties( removetag PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin )
set_target_properties( removetag0 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin )
