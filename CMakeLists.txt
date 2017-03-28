cmake_minimum_required(VERSION 2.8)
project(libplyxx)

file(GLOB LIB_SOURCES "libplyxx/*.cpp")
file(GLOB TEST_SOURCES "test/*.cpp")

include_directories("libplyxx")

add_library(libplyxx STATIC ${LIB_SOURCES})
add_executable(libplyxx_test ${TEST_SOURCES})
target_link_libraries(libplyxx_test libplyxx)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT libplyxx_test)