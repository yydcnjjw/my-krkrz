cmake_minimum_required(VERSION 3.15)
project(my-krkrz)

set(CMAKE_CXX_STANDARD 17)

set(DEPLOY_DIR ${PROJECT_BINARY_DIR}/deploy)
set(DEPS_DIR ${PROJECT_BINARY_DIR}/thirdparty)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEPLOY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DEPLOY_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake)

add_subdirectory(thirdparty)
add_subdirectory(src)
add_subdirectory(test)

