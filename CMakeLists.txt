cmake_minimum_required(VERSION 3.2)
project(libplyxx)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB LIB_SOURCES "libplyxx/*.cpp")
file(GLOB TEST_SOURCES "test/*.cpp")

set(LIBPLYXX_MAJOR_VERSION 0)
set(LIBPLYXX_MINOR_VERSION 5)
set(LIBPLYXX_PATCH_VERSION 0)
set(LIBPLYXX_VERSION ${LIBPLYXX_MAJOR_VERSION}.${LIBPLYXX_MINOR_VERSION}.${LIBPLYXX_PATCH_VERSION})

include_directories("libplyxx")

add_library(libplyxx STATIC ${LIB_SOURCES})
add_executable(libplyxx_test ${TEST_SOURCES})
target_link_libraries(libplyxx_test libplyxx)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT libplyxx_test)
